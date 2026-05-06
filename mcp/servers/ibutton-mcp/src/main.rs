use rmcp::{tool, tool_router, ServiceExt, handler::server::wrapper::Parameters, model::ListToolsResult};
use serde::{Deserialize, Serialize};
use schemars::JsonSchema;
use std::sync::{Arc, Mutex};
use tokio::sync::oneshot;
use tokio::time::{timeout, Duration};
use once_cell::sync::Lazy;
use tg_lpu237_ibutton::{Lpu237IButton, LPU237LOCK_DLL_RESULT_CANCEL};
use lpu237_common::{HANDLE, INVALID_HANDLE_VALUE};
use libc::c_ulong;
use std::path::PathBuf;
use std::collections::HashMap;
use std::future::Future;

// Global state for the MCP server
struct ServerState {
    dll: Option<Lpu237IButton>,
    h_dev: HANDLE,
    buf_index: c_ulong,
    tx: Option<oneshot::Sender<()>>,
    is_running: bool, // Track if background operation is active
    last_result: Option<ReadIButtonResponse>,
}

static STATE: Lazy<Arc<Mutex<ServerState>>> = Lazy::new(|| {
    Arc::new(Mutex::new(ServerState {
        dll: None,
        h_dev: INVALID_HANDLE_VALUE,
        buf_index: 0,
        tx: None,
        is_running: false,
        last_result: None,
    }))
});

#[derive(Clone)]
struct IButtonServer;

#[derive(Deserialize, JsonSchema)]
struct ReadIButtonArgs {
    /// Timeout in seconds for the ibutton touch (default 30)
    timeout_sec: u64,
}

#[derive(Serialize, Clone, JsonSchema)]
struct ReadIButtonResponse {
    success: bool,
    data: Option<String>,
    id: Option<String>,
    error: Option<String>,
    cancelled: bool,
}

#[derive(Deserialize, JsonSchema)]
struct EmptyArgs {}

/// Callback from DLL
extern "C" fn ibutton_callback(_param: *mut std::ffi::c_void) {
    let mut state = STATE.lock().unwrap();
    let index = state.buf_index; // Use the index stored when wait_key was called
    
    if let Some(dll) = state.dll.clone() {
        let result = if index == LPU237LOCK_DLL_RESULT_CANCEL {
            ReadIButtonResponse {
                success: false,
                data: None,
                id: None,
                error: None,
                cancelled: true,
            }
        } else {
            let data = dll.get_data(index).ok().map(|d| hex::encode(d));
            let id = dll.get_id(state.h_dev).ok().map(|d| hex::encode(d));
            ReadIButtonResponse {
                success: true,
                data,
                id,
                error: None,
                cancelled: false,
            }
        };
        state.last_result = Some(result);
    }
    
    if let Some(tx) = state.tx.take() {
        let _ = tx.send(());
    }
}

#[tool_router(server_handler)]
impl IButtonServer {
    #[tool(description = "Wait for an I-Button touch and return its data/id from LPU237 device (Blocking)")]  // runtime: read_ibutton
    async fn read_ibutton(&self, Parameters(args): Parameters<ReadIButtonArgs>) -> Result<String, String> {
        let (tx, rx) = oneshot::channel();
        
        let (dll, h_dev) = {
            let mut state = STATE.lock().unwrap();
            if state.is_running { return Err("An operation is already in progress".to_string()); }

            let dll = state.dll.as_ref().ok_or("DLL not loaded")?.clone();
            let devices = dll.get_list().map_err(|e| format!("Failed to get device list: {}", e))?;
            if devices.is_empty() { return Err("No LPU237 I-Button device found".to_string()); }
            
            let h_dev = dll.open(&devices[0]).map_err(|_| "Failed to open device".to_string())?;
            state.h_dev = h_dev;
            state.last_result = None;
            state.is_running = true;
            dll.enable(h_dev);
            state.tx = Some(tx);
            
            // Capture and store the index returned by the DLL
            let idx = dll.wait_key_with_callback(h_dev, ibutton_callback, std::ptr::null_mut());
            state.buf_index = idx;
            
            (dll, h_dev)
        };

        let res = match timeout(Duration::from_secs(args.timeout_sec), rx).await {
            Ok(Ok(())) => {
                let mut state = STATE.lock().unwrap();
                state.last_result.take().ok_or("Failed to retrieve data from buffer".to_string())?
            }
            Ok(Err(_)) => ReadIButtonResponse {
                success: false, data: None, id: None,
                error: Some("Operation cancelled".to_string()), cancelled: true,
            },
            Err(_) => {
                dll.cancel_wait_key(h_dev);
                ReadIButtonResponse {
                    success: false, data: None, id: None,
                    error: Some("Timeout".to_string()), cancelled: false,
                }
            }
        };

        {
            let mut state = STATE.lock().unwrap();
            state.h_dev = INVALID_HANDLE_VALUE;
            state.is_running = false;
            state.tx.take();
        }
        cleanup(&dll, h_dev);

        Ok(serde_json::to_string(&res).unwrap())
    }

    #[tool(description = "Start waiting for an I-Button touch in the worker of lpu237 dynamic library. AI Agent have to check the reading status while waiting.")]  // runtime: start_read_ibutton
    async fn start_read_ibutton(&self, Parameters(args): Parameters<ReadIButtonArgs>) -> Result<String, String> {
        let (tx, rx) = oneshot::channel();
        
        let (dll, h_dev) = {
            let mut state = STATE.lock().unwrap();
            if state.is_running { return Err("An operation is already in progress".to_string()); }

            let dll = state.dll.as_ref().ok_or("DLL not loaded")?.clone();
            let devices = dll.get_list().map_err(|e| format!("Failed to get device list: {}", e))?;
            if devices.is_empty() { return Err("No LPU237 I-Button device found".to_string()); }
            
            let h_dev = dll.open(&devices[0]).map_err(|_| "Failed to open device".to_string())?;
            state.h_dev = h_dev;
            state.last_result = None;
            state.is_running = true;
            dll.enable(h_dev);
            state.tx = Some(tx);
            
            // Capture and store the index returned by the DLL
            let idx = dll.wait_key_with_callback(h_dev, ibutton_callback, std::ptr::null_mut());
            state.buf_index = idx;
            
            (dll, h_dev)
        };

        // Background task only for timeout handling and final cleanup
        tokio::spawn(async move {
            let result = timeout(Duration::from_secs(args.timeout_sec), rx).await;
            
            if let Err(_) = result {
                // Timeout case
                let mut state = STATE.lock().unwrap();
                if state.is_running && state.h_dev == h_dev {
                    dll.cancel_wait_key(h_dev);
                    state.last_result = Some(ReadIButtonResponse {
                        success: false, data: None, id: None,
                        error: Some("Timeout".to_string()), cancelled: false,
                    });
                }
            }

            // Final state cleanup
            {
                let mut state = STATE.lock().unwrap();
                if state.h_dev == h_dev {
                    state.h_dev = INVALID_HANDLE_VALUE;
                    state.is_running = false;
                    state.tx.take();
                }
            }
            cleanup(&dll, h_dev);
        });

        Ok("I-Button reading started in background. Please touch the key.".to_string())
    }

    #[tool(description = "Check the result of the status of I-Button reading operation")]  // runtime: get_ibutton_result
    async fn get_ibutton_result(&self, _params: Parameters<EmptyArgs>) -> Result<String, String> {
        let mut state = STATE.lock().unwrap();
        if let Some(res) = state.last_result.take() {
            Ok(serde_json::to_string(&res).unwrap())
        } else if state.is_running {
            Ok("Still waiting for I-Button touch...".to_string())
        } else {
            Ok("No background operation in progress or results already retrieved.".to_string())
        }
    }

    #[tool(description = "Cancel any pending I-Button read operation on LPU237 device")]  // runtime: cancel_ibutton
    async fn cancel_ibutton(&self, _params: Parameters<EmptyArgs>) -> Result<String, String> {
        let mut state = STATE.lock().unwrap();
        if state.h_dev == INVALID_HANDLE_VALUE {
            return Ok("No pending operation to cancel".to_string());
        }
        
        if let Some(dll) = &state.dll {
            dll.cancel_wait_key(state.h_dev);
            state.tx.take();
            Ok("Successfully cancelled".to_string())
        } else {
            Err("DLL not loaded".to_string())
        }
    }
}

fn cleanup(dll: &Lpu237IButton, h_dev: HANDLE) {
    if h_dev != INVALID_HANDLE_VALUE {
        dll.disable(h_dev);
        dll.close(h_dev);
    }
}

// ---------- description override wrapper ----------

/// IButtonServer 를 감싸서 list_tools 의 description 을 런타임 값으로 교체
#[derive(Clone)]
struct IButtonServerWithDesc;

impl rmcp::handler::server::ServerHandler for IButtonServerWithDesc {
    fn get_info(&self) -> rmcp::model::ServerInfo {
        IButtonServer.get_info()
    }

    fn list_tools(
        &self,
        request: Option<rmcp::model::PaginatedRequestParams>,
        context: rmcp::service::RequestContext<rmcp::RoleServer>,
    ) -> impl Future<Output = Result<ListToolsResult, rmcp::ErrorData>> + Send + '_ {
        async move {
            let mut result = IButtonServer.list_tools(request, context).await?;
            let map = DESCRIPTIONS.lock().unwrap();
            for tool in &mut result.tools {
                let key: &str = &tool.name.clone();
                if let Some(d) = map.get(key) {
                    tool.description = Some(d.clone().into());
                }
            }
            Ok(result)
        }
    }

    fn call_tool(
        &self,
        request: rmcp::model::CallToolRequestParams,
        context: rmcp::service::RequestContext<rmcp::RoleServer>,
    ) -> impl Future<Output = Result<rmcp::model::CallToolResult, rmcp::ErrorData>> + Send + '_ {
        async move { IButtonServer.call_tool(request, context).await }
    }
}

// ---------- description 로드 ----------

const JSON_FILENAME: &str = "lpu23x-ibutton-mcp.json";

/// MCP 설정 JSON 파일 경로 반환
fn get_config_path() -> PathBuf {
    #[cfg(target_os = "windows")]
    {
        use std::ffi::OsString;
        use std::os::windows::ffi::OsStringExt;
        // FOLDERID_ProgramData = {62AB5D82-FDC1-4DC3-A9DD-070D1D495D97}
        const FOLDERID_PROGRAM_DATA: windows::core::GUID = windows::core::GUID {
            data1: 0x62AB5D82,
            data2: 0xFDC1,
            data3: 0x4DC3,
            data4: [0xA9, 0xDD, 0x07, 0x0D, 0x1D, 0x49, 0x5D, 0x97],
        };
        let base = unsafe {
            windows::Win32::UI::Shell::SHGetKnownFolderPath(
                &FOLDERID_PROGRAM_DATA,
                windows::Win32::UI::Shell::KNOWN_FOLDER_FLAG(0),
                None,
            )
            .map(|pwstr| {
                let slice = pwstr.as_wide();
                OsString::from_wide(slice).into_string().unwrap_or_default()
            })
            .unwrap_or_else(|_| {
                std::env::var("ProgramData").unwrap_or_else(|_| "C:\\ProgramData".to_string())
            })
        };
        PathBuf::from(base)
            .join("elpusk")
            .join("00000006")
            .join("coffee_manager")
            .join("mcp")
            .join(JSON_FILENAME)
    }
    #[cfg(not(target_os = "windows"))]
    {
        PathBuf::from("/usr/share/elpusk/programdata/00000006/coffee_manager/mcp")
            .join(JSON_FILENAME)
    }
}

/// JSON 파일에서 description 로드. 실패 시 하드코딩 기본값 반환.
fn load_descriptions() -> HashMap<&'static str, String> {
    let defaults: HashMap<&str, &str> = [
        ("read_ibutton",       "Synchronous i-button read. Blocks until response or timeout. Use get_ibutton_result() on success."),
        ("start_read_ibutton", "Asynchronous i-button read. Poll get_ibutton_result() every 500 ms\u{2013}1 s to check for a response."),
        ("get_ibutton_result", "Get i-button result."),
        ("cancel_ibutton",     "Set i-button data ignore mode."),
    ].into();

    let path = get_config_path();
    if let Ok(content) = std::fs::read_to_string(&path) {
        if let Ok(map) = serde_json::from_str::<HashMap<String, String>>(&content) {
            eprintln!("Loaded descriptions from: {:?}", path);
            return defaults
                .into_iter()
                .map(|(k, v)| (k, map.get(k).cloned().unwrap_or_else(|| v.to_string())))
                .collect();
        }
    }
    eprintln!("Using default descriptions (config not found: {:?})", path);
    defaults.into_iter().map(|(k, v)| (k, v.to_string())).collect()
}

/// 전역 description 저장소 (main 에서 한 번 초기화)
static DESCRIPTIONS: Lazy<Mutex<HashMap<&'static str, String>>> =
    Lazy::new(|| Mutex::new(HashMap::new()));

// ---------- DLL path ----------

fn local_get_lpu237_ibutton_path() -> PathBuf {
    #[cfg(target_os = "windows")]
    {
        let base = if cfg!(target_pointer_width = "64") {
            std::env::var("ProgramFiles")
        } else {
            std::env::var("ProgramFiles(x86)")
        }.expect("ProgramFiles env not found");

        let arch_dir = if cfg!(target_pointer_width = "64") {
            "x64"
        } else {
            "x86"
        };

        PathBuf::from(base)
            .join("elpusk")
            .join("00000006")
            .join("coffee_manager")
            .join("dll")
            .join(arch_dir)
            .join("tg_lpu237_ibutton.dll")
    }

    #[cfg(target_os = "linux")]
    {
        PathBuf::from("/usr/share/elpusk/program/00000006/coffee_manager/so/libtg_lpu237_ibutton.so")
    }
}

#[tokio::main]
async fn main() -> anyhow::Result<()> {
    eprintln!("Starting LPU237 I-Button MCP Server...");

    // description 로드 (JSON 또는 기본값)
    {
        let loaded = load_descriptions();
        *DESCRIPTIONS.lock().unwrap() = loaded;
    }

    let dll_path = local_get_lpu237_ibutton_path();
    eprintln!("Loading DLL from: {:?}", dll_path);

    let dll = unsafe { 
        Lpu237IButton::new(&dll_path).map_err(|e| {
            let err = format!("Failed to load DLL from {:?}: {}", dll_path, e);
            eprintln!("{}", err);
            anyhow::anyhow!(err)
        })? 
    };
    
    // Initialize DLL once
    dll.dll_on();

    {
        let mut state = STATE.lock().unwrap();
        state.dll = Some(dll.clone());
    }

    eprintln!("DLL loaded and initialized successfully. Starting stdio transport...");

    let transport = (tokio::io::stdin(), tokio::io::stdout());
    let service = IButtonServerWithDesc.serve(transport).await.map_err(|e| {
        eprintln!("Failed to start service: {}", e);
        e
    })?;
    
    eprintln!("I-Button MCP Server is running and waiting for commands.");
    service.waiting().await?;

    
    // De-initialize DLL once
    dll.dll_off();
    
    Ok(())
}
