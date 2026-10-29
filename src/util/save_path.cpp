#include "util/save_path.h"
#include <cstdlib>
#include <filesystem>
#include <system_error>

namespace fs = std::filesystem;

namespace util {

    namespace {

        std::string Env(const char* name) {
#ifdef _MSC_VER
#pragma warning(suppress : 4996)
#endif
            const char* value = std::getenv(name);
            return value ? value : "";
        }

        // Empty when the platform's data folder can't be worked out
        fs::path UserDataDir() {
#if defined(_WIN32)
            const std::string base = Env("APPDATA");
            return base.empty() ? fs::path() : fs::path(base) / "topple";
#elif defined(__APPLE__)
            const std::string home = Env("HOME");
            return home.empty() ? fs::path() : fs::path(home) / "Library" / "Application Support" / "topple";
#else
            const std::string data = Env("XDG_DATA_HOME");
            if (!data.empty()) return fs::path(data) / "topple";
            const std::string home = Env("HOME");
            return home.empty() ? fs::path() : fs::path(home) / ".local" / "share" / "topple";
#endif
        }

    }  // namespace

    std::string SaveFilePath(const std::string& fileName, const std::string& programDir) {
        const fs::path legacy = fs::path(programDir) / fileName;

        const fs::path dir = UserDataDir();
        if (dir.empty()) return legacy.string();

        std::error_code error;
        fs::create_directories(dir, error);
        if (error) return legacy.string();

        const fs::path path = dir / fileName;
        if (!fs::exists(path, error) && fs::exists(legacy, error))
            fs::copy_file(legacy, path, error);   // best effort: a failed copy just starts fresh
        return path.string();
    }

}  // namespace util
