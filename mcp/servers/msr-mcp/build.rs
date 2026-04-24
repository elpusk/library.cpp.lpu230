use winres;

fn make_version(major: u16, minor: u16, patch: u16, build: u16) -> u64 {
    ((major as u64) << 48)
        | ((minor as u64) << 32)
        | ((patch as u64) << 16)
        | (build as u64)
}

fn main(){
    // Windows 타겟일 때만 실행
    if std::env::var("CARGO_CFG_TARGET_OS").unwrap() == "windows" {
        let v = std::env::var("CARGO_PKG_VERSION").unwrap();
        // "1.2.3-alpha.1" → "1.2.3"
        let core = v.split('-').next().unwrap();

        let mut parts = core.split('.').map(|s| s.parse::<u16>().unwrap_or(0));

        let major = parts.next().unwrap_or(0);
        let minor = parts.next().unwrap_or(0);
        let patch = parts.next().unwrap_or(0);

        // build 번호는 없으므로 0 (또는 git count 넣어도 됨)
        let build = 0;        
        let mut res = winres::WindowsResource::new();

        res.set("FileDescription", "lpu23x-msr-mcp");
        res.set("ProductName", "lpu23x-msr-mcp");
        res.set("CompanyName", "Elpusk.Co.,Ltd");
        res.set("LegalCopyright", "Copyright (C) 2026 Elpusk.Co.,Ltd");
        res.set("FileVersion", &v);
        res.set("ProductVersion", &v);        

        let version_u64 = make_version(major, minor, patch, build);
        // 중요: 버전 (콤마 구분)
        res.set_version_info(winres::VersionInfo::FILEVERSION, version_u64);
        res.set_version_info(winres::VersionInfo::PRODUCTVERSION, version_u64);

        res.compile().unwrap();
    }    

}