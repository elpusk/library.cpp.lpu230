use rmcp::{tool, tool_router, ServiceExt, handler::server::wrapper::Parameters};
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
    // Just signal completion. Do not overwrite state.buf_index.
    let mut state = STATE.lock().unwrap();
    if let Some(tx) = state.tx.take() {
        let _ = tx.send(());
    }
}

#[tool_router(server_handler)]
impl IButtonServer {
    #[tool(description = "Wait for an I-Button touch and return its data/id from LPU237 device (Blocking)")]
    async fn read_ibutton(&self, Parameters(args): Parameters<ReadIButtonArgs>) -> Result<String, String> {
        let (tx, rx) = oneshot::channel();
        
        let res = (|| async {
            let (dll, h_dev, buf_index) = {
                let mut state = STATE.lock().unwrap();
                
                if state.is_running {
                    return Err("An operation is already in progress".to_string());
                }

                let dll = state.dll.as_ref().ok_or("DLL not loaded")?.clone();
                
                let devices = dll.get_list().map_err(|e| format!("Failed to get device list: {}", e))?;
                if devices.is_empty() {
                    return Err("No LPU237 I-Button device found".to_string());
                }
                
                let h_dev = dll.open(&devices[0]).map_err(|_| "Failed to open device".to_string())?;
                state.h_dev = h_dev;
                state.last_result = None;
                state.is_running = true;
                
                dll.enable(h_dev);
                state.tx = Some(tx);
                
                // Use the return value as the buffer index
                let idx = dll.wait_key_with_callback(h_dev, ibutton_callback, std::ptr::null_mut());
                state.buf_index = idx;
                
                (dll, h_dev, idx)
            };

            let result = timeout(Duration::from_secs(args.timeout_sec), rx).await;
            
            let mut state = STATE.lock().unwrap();
            state.h_dev = INVALID_HANDLE_VALUE;
            state.tx.take();
            state.is_running = false;

            match result {
                Ok(Ok(())) => {
                    if buf_index == LPU237LOCK_DLL_RESULT_CANCEL {
                         cleanup(&dll, h_dev);
                         Ok(ReadIButtonResponse {
                            success: false,
                            data: None,
                            id: None,
                            error: None,
                            cancelled: true,
                        })
                    } else {
                        let data = dll.get_data(buf_index).ok().map(|d| hex::encode(d));
                        let id = dll.get_id(h_dev).ok().map(|d| hex::encode(d));
                        
                        cleanup(&dll, h_dev);
                        Ok(ReadIButtonResponse {
                            success: true,
                            data,
                            id,
                            error: None,
                            cancelled: false,
                        })
                    }
                }
                Ok(Err(_)) => {
                    cleanup(&dll, h_dev);
                    Ok(ReadIButtonResponse {
                        success: false,
                        data: None,
                        id: None,
                        error: Some("Operation cancelled".to_string()),
                        cancelled: true,
                    })
                }
                Err(_) => {
                    dll.cancel_wait_key(h_dev);
                    cleanup(&dll, h_dev);
                    Ok(ReadIButtonResponse {
                        success: false,
                        data: None,
                        id: None,
                        error: Some("Timeout".to_string()),
                        cancelled: false,
                    })
                }
            }
        })().await;

        match res {
            Ok(resp) => Ok(serde_json::to_string(&resp).unwrap()),
            Err(e) => Err(e),
        }
    }

    #[tool(description = "Start waiting for an I-Button touch in the background. AI Agent can continue while waiting.")]
    async fn start_read_ibutton(&self, Parameters(args): Parameters<ReadIButtonArgs>) -> Result<String, String> {
        let (tx, rx) = oneshot::channel();
        
        let (dll, h_dev, buf_index) = {
            let mut state = STATE.lock().unwrap();
            
            if state.is_running {
                return Err("An operation is already in progress".to_string());
            }

            let dll = state.dll.as_ref().ok_or("DLL not loaded")?.clone();
            
            let devices = dll.get_list().map_err(|e| format!("Failed to get device list: {}", e))?;
            if devices.is_empty() {
                return Err("No LPU237 I-Button device found".to_string());
            }
            
            let h_dev = dll.open(&devices[0]).map_err(|_| "Failed to open device".to_string())?;
            state.h_dev = h_dev;
            state.last_result = None;
            state.is_running = true;
            
            dll.enable(h_dev);
            state.tx = Some(tx);
            
            let idx = dll.wait_key_with_callback(h_dev, ibutton_callback, std::ptr::null_mut());
            state.buf_index = idx;
            
            (dll, h_dev, idx)
        };

        // Spawn background task
        tokio::spawn(async move {
            // 1. Wait for callback signal (triggered by ibutton_callback)
            let result = timeout(Duration::from_secs(args.timeout_sec), rx).await;
            
            // 2. Retrieval phase: Only happens after signal or timeout
            let final_resp = match result {
                Ok(Ok(())) => {
                    // Signal received! Now get the data using the captured buf_index.
                    if buf_index == LPU237LOCK_DLL_RESULT_CANCEL {
                         ReadIButtonResponse {
                            success: false,
                            data: None,
                            id: None,
                            error: None,
                            cancelled: true,
                        }
                    } else {
                        let data = dll.get_data(buf_index).ok().map(|d| hex::encode(d));
                        let id = dll.get_id(h_dev).ok().map(|d| hex::encode(d));
                        ReadIButtonResponse {
                            success: true,
                            data,
                            id,
                            error: None,
                            cancelled: false,
                        }
                    }
                }
                Ok(Err(_)) => {
                    ReadIButtonResponse {
                        success: false,
                        data: None,
                        id: None,
                        error: Some("Operation cancelled".to_string()),
                        cancelled: true,
                    }
                }
                Err(_) => {
                    // Timeout: cancel the operation in DLL
                    dll.cancel_wait_key(h_dev);
                    ReadIButtonResponse {
                        success: false,
                        data: None,
                        id: None,
                        error: Some("Timeout".to_string()),
                        cancelled: false,
                    }
                }
            };

            cleanup(&dll, h_dev);
            
            let mut state = STATE.lock().unwrap();
            state.h_dev = INVALID_HANDLE_VALUE;
            state.tx.take();
            state.last_result = Some(final_resp);
            state.is_running = false;
        });

        Ok("I-Button reading started in background. Please touch the key.".to_string())
    }

    #[tool(description = "Check the result of the background I-Button reading operation")]
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

    #[tool(description = "Cancel any pending I-Button read operation on LPU237 device")]
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
    let service = IButtonServer.serve(transport).await.map_err(|e| {
        eprintln!("Failed to start service: {}", e);
        e
    })?;
    
    eprintln!("I-Button MCP Server is running and waiting for commands.");
    service.waiting().await?;
    
    // De-initialize DLL once
    dll.dll_off();
    
    Ok(())
}
