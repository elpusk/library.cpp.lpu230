#include <main.h>

#include <iostream>
#include <filesystem>

#include <mp_type.h>
#include <mp_clog.h>
#include <mp_cfile.h>
#include <mp_coffee_path.h>

#ifdef _WIN32
static bool _clean_all_on_windows();
#endif

/**
 * remove all installed file and paths.
 * 데몬(서비스) 중지 후, 모든 로그를 삭제 하기 때문에 여기서는 trace 나 로그를 사용 할수 없다.
 * 리눅스의 경우에는 uninstaller 에서 -P option 에 의해 실행 되므로, 여기서는 지워하지 않는다.
 */
int main_remove_all(const _mp::type_set_wstring& set_parameters)
{
	int n_result(_mp::exit_error_clean_up);

	do {
#ifdef _WIN32
        //windows
        if (!_clean_all_on_windows()) {
            continue;
        }
        n_result = EXIT_SUCCESS;
#else
        //linux
        n_result = _mp::exit_error_not_supported;
#endif //_WIN32
	}while(false);

	return n_result;
}

#ifdef _WIN32
bool _clean_all_on_windows()
{
    try {
        // 경로 정의
        const std::filesystem::path pd = _mp::cfile::get_path_ProgramData();
        const std::filesystem::path base = pd / "Elpusk";
        const std::filesystem::path idFolder = base / "00000006";
        const std::filesystem::path manager = idFolder / "coffee_manager";

        // 1. coffee_manager 폴더와 모든 하위 항목 삭제
        if (std::filesystem::exists(manager)) {
            std::uintmax_t cnt = std::filesystem::remove_all(manager);
            //std::cout << "[INFO] Removed coffee_manager (" << cnt << " items)\n";
        }

        // 2. idFolder(00000006)가 비었으면 삭제
        if (std::filesystem::exists(idFolder) && std::filesystem::is_empty(idFolder)) {
			bool removed = std::filesystem::remove_all(idFolder);// remove() 하면 idFolder 폴더 자체는 삭제 안됨
            if (removed) {
                //std::cout << "[INFO] Removed folder: " << idFolder << "\n";
            }
        }

        // 3. base(Elpusk) 폴더가 비었으면 삭제
        if (std::filesystem::exists(base) && std::filesystem::is_empty(base)) {
            bool removed = std::filesystem::remove_all(base);
            if (removed) {
                //std::cout << "[INFO] Removed folder: " << base << "\n";
            }
        }

        return true;
    }
    catch (const std::filesystem::filesystem_error& e) {
        //std::cerr << "[ERROR] " << e.what() << "\n";
        return false;
    }
}

#endif //_WIN32