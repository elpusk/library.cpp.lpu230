use rmcp::{tool, tool_router, ServiceExt, transport::stdio, handler::server::wrapper::Parameters};
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
    tx: Option<oneshot::Sender<c_ulong>>,
}

static STATE: Lazy<Arc<Mutex<ServerState>>> = Lazy::new(|| {
    Arc::new(Mutex::new(ServerState {
        dll: None,
        h_dev: INVALID_HANDLE_VALUE,
        tx: None,
    }))
});

#[derive(Clone)]
struct IButtonServer;

#[derive(Deserialize, JsonSchema)]
struct ReadIButtonArgs {
    /// Timeout in seconds for the ibutton touch
    timeout_sec: u64,
}

#[derive(Serialize, JsonSchema)]
struct ReadIButtonResponse {
    success: bool,
    data: Option<String>,
    id: Option<String>,
    error: Option<String>,
    cancelled: bool,
}

/// Callback from DLL
extern "C" fn ibutton_callback(param: *mut std::ffi::c_void) {
    let index = param as c_ulong;
    let mut state = STATE.lock().unwrap();
    if let Some(tx) = state.tx.take() {
        let _ = tx.send(index);
    }
}

#[tool_router(server_handler)]
impl IButtonServer {
    #[tool(description = "Wait for an I-Button touch and return its data/id")]
    async fn read_ibutton(&self, Parameters(args): Parameters<ReadIButtonArgs>) -> String {
        let (tx, rx) = oneshot::channel();
        
        let res = (|| async {
            let (dll, h_dev) = {
                let mut state = STATE.lock().unwrap();
                
                if state.tx.is_some() {
                    return Err("Already reading an I-Button".to_string());
                }

                let dll = state.dll.as_ref().ok_or("DLL not loaded")?.clone();
                dll.dll_on();
                
                let devices = dll.get_list().map_err(|e| format!("Failed to get device list: {}", e))?;
                if devices.is_empty() {
                    return Err("No LPU237 I-Button device found".to_string());
                }
                
                let h_dev = dll.open(&devices[0]).map_err(|_| "Failed to open device".to_string())?;
                state.h_dev = h_dev;
                
                dll.enable(h_dev);
                state.tx = Some(tx);
                dll.wait_key_with_callback(h_dev, ibutton_callback, std::ptr::null_mut());
                
                (dll, h_dev)
            };

            let result = timeout(Duration::from_secs(args.timeout_sec), rx).await;
            
            let mut state = STATE.lock().unwrap();
            state.h_dev = INVALID_HANDLE_VALUE;
            state.tx.take();

            match result {
                Ok(Ok(index)) => {
                    if index == LPU237LOCK_DLL_RESULT_CANCEL {
                         cleanup(&dll, h_dev);
                         Ok(ReadIButtonResponse {
                            success: false,
                            data: None,
                            id: None,
                            error: None,
                            cancelled: true,
                        })
                    } else {
                        let data = dll.get_data(index).ok().map(|d| hex::encode(d));
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
            Ok(resp) => serde_json::to_string(&resp).unwrap(),
            Err(e) => format!("Error: {}", e),
        }
    }

    #[tool(description = "Cancel a pending I-Button read operation")]
    async fn cancel_ibutton(&self, _params: Parameters<serde_json::Value>) -> String {
        let mut state = STATE.lock().unwrap();
        if state.h_dev == INVALID_HANDLE_VALUE {
            return "false".to_string();
        }
        
        if let Some(dll) = &state.dll {
            dll.cancel_wait_key(state.h_dev);
            state.tx.take();
            "true".to_string()
        } else {
            "Error: DLL not loaded".to_string()
        }
    }
}

fn cleanup(dll: &Lpu237IButton, h_dev: HANDLE) {
    if h_dev != INVALID_HANDLE_VALUE {
        dll.disable(h_dev);
        dll.close(h_dev);
    }
    dll.dll_off();
}

fn local_get_lpu237_ibutton_path() -> PathBuf {
    #[cfg(target_os = "windows")]
    {
        // ProgramFiles 경로 얻기 (x64/x86 구분 포함)
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
    let dll_path = local_get_lpu237_ibutton_path();

    let dll = unsafe { Lpu237IButton::new(dll_path).map_err(|e| anyhow::anyhow!("Failed to load DLL: {}", e))? };
    
    {
        let mut state = STATE.lock().unwrap();
        state.dll = Some(dll);
    }

    let transport = (tokio::io::stdin(), tokio::io::stdout());
    let service = IButtonServer.serve(transport).await?;
    service.waiting().await?;
    
    Ok(())
}
