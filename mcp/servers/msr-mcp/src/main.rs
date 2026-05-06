use rmcp::{tool, tool_router, ServiceExt, handler::server::wrapper::Parameters, model::ListToolsResult};
use serde::{Deserialize, Serialize};
use schemars::JsonSchema;
use std::sync::{Arc, Mutex};
use tokio::sync::oneshot;
use tokio::time::{timeout, Duration};
use once_cell::sync::Lazy;
use tg_lpu237_dll::{Lpu237Dll, LPU237_DLL_RESULT_CANCEL};
use lpu237_common::{HANDLE, INVALID_HANDLE_VALUE};
use libc::c_ulong;
use std::path::PathBuf;
use std::collections::HashMap;
use std::future::Future;

// Global state for the MCP server
struct ServerState {
    dll: Option<Lpu237Dll>,
    h_dev: HANDLE,
    buf_index: c_ulong,
    tx: Option<oneshot::Sender<()>>,
    is_running: bool, // Track if background operation is active
    last_result: Option<ReadCardResponse>,
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
struct MsrServer;

#[derive(Deserialize, JsonSchema)]
struct ReadCardArgs {
    /// Timeout in seconds for the card swipe (default 30)
    timeout_sec: u64,
}

#[derive(Serialize, Clone, JsonSchema)]
struct ReadCardResponse {
    success: bool,
    track1: Option<String>,
    track2: Option<String>,
    track3: Option<String>,
    error: Option<String>,
    cancelled: bool,
}

#[derive(Deserialize, JsonSchema)]
struct EmptyArgs {}

/// Callback from DLL
extern "C" fn msr_callback(_param: *mut std::ffi::c_void) {
    let mut state = STATE.lock().unwrap();
    let index = state.buf_index; // Use the index stored when wait_swipe was called

    if let Some(dll) = state.dll.clone() {
        let result = if index == LPU237_DLL_RESULT_CANCEL {
            ReadCardResponse {
                success: false,
                track1: None,
                track2: None,
                track3: None,
                error: None,
                cancelled: true,
            }
        } else {
            let t1 = dll.get_data(index, 1).ok().map(|d| String::from_utf8_lossy(&d).to_string());
            let t2 = dll.get_data(index, 2).ok().map(|d| String::from_utf8_lossy(&d).to_string());
            let t3 = dll.get_data(index, 3).ok().map(|d| String::from_utf8_lossy(&d).to_string());
            ReadCardResponse {
                success: true,
                track1: t1,
                track2: t2,
                track3: t3,
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
impl MsrServer {
    #[tool(description = "Wait for a magnetic card swipe and return track data from LPU237 device (Blocking)")]
    async fn read_card(&self, Parameters(args): Parameters<ReadCardArgs>) -> Result<String, String> {
        let (tx, rx) = oneshot::channel();

        let (dll, h_dev) = {
            let mut state = STATE.lock().unwrap();
            if state.is_running { return Err("An operation is already in progress".to_string()); }

            let dll = state.dll.as_ref().ok_or("DLL not loaded")?.clone();
            let devices = dll.get_list().map_err(|e| format!("Failed to get device list: {}", e))?;
            if devices.is_empty() { return Err("No LPU237 device found".to_string()); }

            let h_dev = dll.open(&devices[0]).map_err(|_| format!("Failed to open device: {}", devices[0]))?;
            state.h_dev = h_dev;
            state.last_result = None;
            state.is_running = true;
            dll.enable(h_dev);
            state.tx = Some(tx);
            
            // Capture and store the index returned by the DLL
            let idx = dll.wait_swipe_with_callback(h_dev, msr_callback, std::ptr::null_mut());
            state.buf_index = idx;
            
            (dll, h_dev)
        };

        let res = match timeout(Duration::from_secs(args.timeout_sec), rx).await {
            Ok(Ok(())) => {
                let mut state = STATE.lock().unwrap();
                state.last_result.take().ok_or("Failed to retrieve data from buffer".to_string())?
            }
            Ok(Err(_)) => ReadCardResponse {
                success: false, track1: None, track2: None, track3: None,
                error: Some("Operation cancelled".to_string()), cancelled: true,
            },
            Err(_) => {
                dll.cancel_wait_swipe(h_dev);
                ReadCardResponse {
                    success: false, track1: None, track2: None, track3: None,
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

    #[tool(description = "Start waiting for a magnetic card swipe in the the worker of lpu237 dynamic library. AI Agent have to check the reading status while waiting.")]
    async fn start_read_card(&self, Parameters(args): Parameters<ReadCardArgs>) -> Result<String, String> {
        let (tx, rx) = oneshot::channel();

        let (dll, h_dev) = {
            let mut state = STATE.lock().unwrap();
            if state.is_running { return Err("An operation is already in progress".to_string()); }

            let dll = state.dll.as_ref().ok_or("DLL not loaded")?.clone();
            let devices = dll.get_list().map_err(|e| format!("Failed to get device list: {}", e))?;
            if devices.is_empty() { return Err("No LPU237 device found".to_string()); }

            let h_dev = dll.open(&devices[0]).map_err(|_| "Failed to open device".to_string())?;
            state.h_dev = h_dev;
            state.last_result = None;
            state.is_running = true;
            dll.enable(h_dev);
            state.tx = Some(tx);
            
            // Capture and store the index returned by the DLL
            let idx = dll.wait_swipe_with_callback(h_dev, msr_callback, std::ptr::null_mut());
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
                    dll.cancel_wait_swipe(h_dev);
                    state.last_result = Some(ReadCardResponse {
                        success: false, track1: None, track2: None, track3: None,
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

        Ok("MSR card reading started in background. Please swipe the card.".to_string())
    }

    #[tool(description = "Check the result of the status of card reading operation")]
    async fn get_card_result(&self, _params: Parameters<EmptyArgs>) -> Result<String, String> {
        let mut state = STATE.lock().unwrap();
        if let Some(res) = state.last_result.take() {
            Ok(serde_json::to_string(&res).unwrap())
        } else if state.is_running {
            Ok("Still waiting for card swipe...".to_string())
        } else {
            Ok("No background operation in progress or results already retrieved.".to_string())
        }
    }

    #[tool(description = "Cancel any pending card read operation on LPU237 device")]
    async fn cancel_card(&self, _params: Parameters<EmptyArgs>) -> Result<String, String> {
        let mut state = STATE.lock().unwrap();
        if state.h_dev == INVALID_HANDLE_VALUE {
            return Ok("No pending operation to cancel".to_string());
        }
        
        if let Some(dll) = &state.dll {
            dll.cancel_wait_swipe(state.h_dev);
            state.tx.take();
            Ok("Successfully cancelled".to_string())
        } else {
            Err("DLL not loaded".to_string())
        }
    }
}

fn cleanup(dll: &Lpu237Dll, h_dev: HANDLE) {
    if h_dev != INVALID_HANDLE_VALUE {
        dll.disable(h_dev);
        dll.close(h_dev);
    }
}

// ---------- description override wrapper ----------

/// MsrServer 를 감싸서 list_tools 의 description 을 런타임 값으로 교체
#[derive(Clone)]
struct MsrServerWithDesc;

impl rmcp::handler::server::ServerHandler for MsrServerWithDesc {
    fn get_info(&self) -> rmcp::model::ServerInfo {
        MsrServer.get_info()
    }

    fn list_tools(
        &self,
        request: Option<rmcp::model::PaginatedRequestParams>,
        context: rmcp::service::RequestContext<rmcp::RoleServer>,
    ) -> impl Future<Output = Result<ListToolsResult, rmcp::ErrorData>> + Send + '_ {
        async move {
            let mut result = MsrServer.list_tools(request, context).await?;
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
        async move { MsrServer.call_tool(request, context).await }
    }
}

// ---------- description 로드 ----------

const JSON_FILENAME: &str = "lpu23x-msr-mcp.json";

/// MCP 설정 JSON 파일 경로 반환
fn get_config_path() -> PathBuf {
    #[cfg(target_os = "windows")]
    {
        use std::ffi::OsString;
        use std::os::windows::ffi::OsStringExt;
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
        ("read_card",       "Synchronous a magnetic card read. Blocks until response or timeout. Use get_card_result() on success."),
        ("start_read_card", "Asynchronous a magnetic card read. Poll get_card_result() every 500 ms\u{2013}1 s to check for a response."),
        ("get_card_result", "Get magnetic card result."),
        ("cancel_card",     "Set a magnetic card data ignore mode."),
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

fn local_get_lpu237_dll_path() -> PathBuf {
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
            .join("tg_lpu237_dll.dll")
    }

    #[cfg(target_os = "linux")]
    {
        PathBuf::from("/usr/share/elpusk/program/00000006/coffee_manager/so/libtg_lpu237_dll.so")
    }
}

#[tokio::main]
async fn main() -> anyhow::Result<()> {
    eprintln!("Starting LPU237 MSR MCP Server...");

    // description 로드 (JSON 또는 기본값)
    {
        let loaded = load_descriptions();
        *DESCRIPTIONS.lock().unwrap() = loaded;
    }

    let dll_path = local_get_lpu237_dll_path();
    eprintln!("Loading DLL from: {:?}", dll_path);

    let dll = unsafe { 
        Lpu237Dll::new(&dll_path).map_err(|e| {
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
    let service = MsrServerWithDesc.serve(transport).await.map_err(|e| {
        eprintln!("Failed to start service: {}", e);
        e
    })?;
    
    eprintln!("MSR MCP Server is running and waiting for commands.");
    service.waiting().await?;
    
    // De-initialize DLL once
    dll.dll_off();
    
    Ok(())
}
