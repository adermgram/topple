#pragma once

#include <string>

namespace util {

    /* Where to keep a save file: the per-user data folder (%APPDATA%\topple, ~/.local/share/topple,
    ~/Library/Application Support/topple), which stays writable and survives replacing the game folder.
    A file left next to the program by an older version is copied over the first time.
    If that folder can't be found or created, the file goes next to the program instead. */
    std::string SaveFilePath(const std::string& fileName, const std::string& programDir);

}  // namespace util
