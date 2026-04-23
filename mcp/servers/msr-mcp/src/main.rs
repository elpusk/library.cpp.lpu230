use rmcp::{tool, tool_router, ServiceExt, transport::stdio, handler::server::wrapper::Parameters};
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

// Global state for the MCP server
struct ServerState {
    dll: Option<Lpu237Dll>,
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
struct MsrServer;

#[derive(Deserialize, JsonSchema)]
struct ReadCardArgs {
    /// Timeout in seconds for the card swipe (default 30)
    timeout_sec: u64,
}

#[derive(Serialize, JsonSchema)]
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
extern "C" fn msr_callback(param: *mut std::ffi::c_void) {
    let index = param as c_ulong;
    let mut state = STATE.lock().unwrap();
    if let Some(tx) = state.tx.take() {
        let _ = tx.send(index);
    }
}

#[tool_router(server_handler)]
impl MsrServer {
    #[tool(description = "Wait for a magnetic card swipe and return track data from LPU237 device")]
    async fn read_card(&self, Parameters(args): Parameters<ReadCardArgs>) -> Result<String, String> {
        let (tx, rx) = oneshot::channel();
        
        let res = (|| async {
            let (dll, h_dev) = {
                let mut state = STATE.lock().unwrap();
                
                if state.tx.is_some() {
                    return Err("Already reading a card".to_string());
                }

                let dll = state.dll.as_ref().ok_or("DLL not loaded")?.clone();
                dll.dll_on();
                
                let devices = dll.get_list().map_err(|e| format!("Failed to get device list: {}", e))?;
                if devices.is_empty() {
                    return Err("No LPU237 device found".to_string());
                }
                
                let h_dev = dll.open(&devices[0]).map_err(|_| "Failed to open device".to_string())?;
                state.h_dev = h_dev;
                
                dll.enable(h_dev);
                state.tx = Some(tx);
                dll.wait_swipe_with_callback(h_dev, msr_callback, std::ptr::null_mut());
                
                (dll, h_dev)
            };

            let result = timeout(Duration::from_secs(args.timeout_sec), rx).await;
            
            let mut state = STATE.lock().unwrap();
            state.h_dev = INVALID_HANDLE_VALUE;
            state.tx.take();

            match result {
                Ok(Ok(index)) => {
                    if index == LPU237_DLL_RESULT_CANCEL {
                         cleanup(&dll, h_dev);
                         Ok(ReadCardResponse {
                            success: false,
                            track1: None,
                            track2: None,
                            track3: None,
                            error: None,
                            cancelled: true,
                        })
                    } else {
                        let t1 = dll.get_data(index, 1).ok().map(|d| String::from_utf8_lossy(&d).to_string());
                        let t2 = dll.get_data(index, 2).ok().map(|d| String::from_utf8_lossy(&d).to_string());
                        let t3 = dll.get_data(index, 3).ok().map(|d| String::from_utf8_lossy(&d).to_string());
                        cleanup(&dll, h_dev);
                        Ok(ReadCardResponse {
                            success: true,
                            track1: t1,
                            track2: t2,
                            track3: t3,
                            error: None,
                            cancelled: false,
                        })
                    }
                }
                Ok(Err(_)) => {
                    cleanup(&dll, h_dev);
                    Ok(ReadCardResponse {
                        success: false,
                        track1: None,
                        track2: None,
                        track3: None,
                        error: Some("Operation cancelled".to_string()),
                        cancelled: true,
                    })
                }
                Err(_) => {
                    dll.cancel_wait_swipe(h_dev);
                    cleanup(&dll, h_dev);
                    Ok(ReadCardResponse {
                        success: false,
                        track1: None,
                        track2: None,
                        track3: None,
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
    dll.dll_off();
}

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
    // Standard logger for MCP often writes to stderr
    eprintln!("Starting LPU237 MSR MCP Server...");

    let dll_path = local_get_lpu237_dll_path();
    eprintln!("Loading DLL from: {:?}", dll_path);

    let dll = unsafe { 
        Lpu237Dll::new(&dll_path).map_err(|e| {
            let err = format!("Failed to load DLL from {:?}: {}", dll_path, e);
            eprintln!("{}", err);
            anyhow::anyhow!(err)
        })? 
    };
    
    {
        let mut state = STATE.lock().unwrap();
        state.dll = Some(dll);
    }

    eprintln!("DLL loaded successfully. Starting stdio transport...");

    let transport = (tokio::io::stdin(), tokio::io::stdout());
    let service = MsrServer.serve(transport).await.map_err(|e| {
        eprintln!("Failed to start service: {}", e);
        e
    })?;
    
    eprintln!("MSR MCP Server is running and waiting for commands.");
    service.waiting().await?;
    
    Ok(())
}
