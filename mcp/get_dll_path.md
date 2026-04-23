# dll(so) path 얻기

## tg_lpu237_dll.dll 이나 libtg_lpu237_dll.so path 얻는 방법은 아래 코드를 사용.

``` rust
fn local_get_lpu237_dll_path() -> PathBuf {
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
            .join("tg_lpu237_dll.dll")
    }

    #[cfg(target_os = "linux")]
    {
        PathBuf::from("/usr/share/elpusk/program/00000006/coffee_manager/so/libtg_lpu237_dll.so")
    }
}
```

## tg_lpu237_ibutton.dll 이나 libtg_lpu237_ibutton.so path 얻는 방법은 아래 코드를 사용.

``` rust
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
```
