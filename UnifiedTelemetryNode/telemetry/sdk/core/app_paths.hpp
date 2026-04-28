#pragma once

#include <windows.h>

#include <string>

namespace AppPaths {

inline bool IsSlash(const char c) {
    return c == '\\' || c == '/';
}

inline std::string WithBackslashes(std::string path) {
    for (char& c : path) {
        if (c == '/') c = '\\';
    }
    return path;
}

inline std::string EnsureTrailingSlash(std::string path) {
    if (!path.empty() && !IsSlash(path.back())) {
        path.push_back('\\');
    }
    return path;
}

inline bool IsAbsolutePath(const std::string& path) {
    if (path.size() >= 3 &&
        ((path[0] >= 'A' && path[0] <= 'Z') || (path[0] >= 'a' && path[0] <= 'z')) &&
        path[1] == ':' && IsSlash(path[2])) {
        return true;
    }
    return path.rfind("\\\\", 0) == 0 || path.rfind("//", 0) == 0;
}

inline std::string ParentDirectory(const std::string& path) {
    const std::size_t pos = path.find_last_of("\\/");
    if (pos == std::string::npos) return "";
    return path.substr(0, pos);
}

inline void EnsureDirectoryTree(const std::string& directory) {
    if (directory.empty()) return;

    const std::string path = WithBackslashes(directory);
    std::size_t start = 0;
    if (path.size() >= 3 && path[1] == ':' && path[2] == '\\') {
        start = 3;
    }

    std::size_t pos = path.find('\\', start);
    while (pos != std::string::npos) {
        const std::string part = path.substr(0, pos);
        if (!part.empty() && part.back() != ':') {
            CreateDirectoryA(part.c_str(), nullptr);
        }
        pos = path.find('\\', pos + 1);
    }

    CreateDirectoryA(path.c_str(), nullptr);
}

inline std::string ExecutableDirectory() {
    char exePath[MAX_PATH] = {};
    GetModuleFileNameA(nullptr, exePath, MAX_PATH);

    std::string dir = exePath;
    const std::size_t pos = dir.find_last_of("\\/");
    if (pos != std::string::npos) {
        dir.resize(pos + 1);
    }
    return EnsureTrailingSlash(dir);
}

inline std::string LocalAppDataRoot() {
    char buffer[4096] = {};
    const DWORD len = GetEnvironmentVariableA("LOCALAPPDATA", buffer, static_cast<DWORD>(sizeof(buffer)));

    std::string root;
    if (len > 0 && len < sizeof(buffer)) {
        root.assign(buffer, len);
    } else {
        root = ExecutableDirectory();
    }

    root = EnsureTrailingSlash(root) + "UnifiedTelemetryNode";
    EnsureDirectoryTree(root);
    return EnsureTrailingSlash(root);
}

inline std::string RuntimePath(const std::string& relativePath) {
    if (relativePath.empty()) return LocalAppDataRoot();

    if (IsAbsolutePath(relativePath)) {
        EnsureDirectoryTree(ParentDirectory(relativePath));
        return WithBackslashes(relativePath);
    }

    std::string relative = WithBackslashes(relativePath);
    while (!relative.empty() && IsSlash(relative.front())) {
        relative.erase(relative.begin());
    }

    const std::string path = LocalAppDataRoot() + relative;
    EnsureDirectoryTree(ParentDirectory(path));
    return path;
}

inline std::string LoaderSessionPath() {
    return RuntimePath("loader_session.json");
}

inline std::string KeyPath() {
    return RuntimePath("key.txt");
}

inline std::string SettingsConfigPath() {
    return RuntimePath("dataMacro\\Config\\settings.json");
}

} // namespace AppPaths
