#include "process_single_instance.hpp"

#include <tlhelp32.h>
#include <algorithm>
#include <cctype>
#include <string>

namespace {

constexpr const char* kMutexName = "Global\\GZ_telemetry_PROTECTOR_SINGLETON";
constexpr const char* kLockFileName = ".gz_telemetry_instance.lock";

std::string ToLower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return value;
}

std::string GetCurrentModulePath() {
    char path[MAX_PATH] = {};
    GetModuleFileNameA(nullptr, path, MAX_PATH);
    return path;
}

std::string DirectoryOf(const std::string& path) {
    const size_t slash = path.find_last_of("\\/");
    return slash == std::string::npos ? std::string() : path.substr(0, slash);
}

std::string BaseNameOf(const std::string& path) {
    const size_t slash = path.find_last_of("\\/");
    return slash == std::string::npos ? path : path.substr(slash + 1);
}

bool IsRandomizedDebugName(const std::string& baseName) {
    const std::string lower = ToLower(baseName);
    if (lower.size() < 12 || lower.size() > 20) return false;
    if (lower.compare(lower.size() - 4, 4, ".exe") != 0) return false;

    const size_t stemLength = lower.size() - 4;
    if (stemLength < 8 || stemLength > 16) return false;

    for (size_t i = 0; i < stemLength; ++i) {
        if (!std::isalpha(static_cast<unsigned char>(baseName[i]))) return false;
    }
    return true;
}

bool IsGeneratedOverlayName(const std::string& baseName) {
    const std::string lower = ToLower(baseName);
    return lower == "gameoverlay_debug.exe" ||
           lower == "securityhealthservice.exe" ||
           IsRandomizedDebugName(baseName);
}

bool QueryProcessPath(HANDLE process, std::string& outPath) {
    char path[MAX_PATH] = {};
    DWORD size = MAX_PATH;
    if (!QueryFullProcessImageNameA(process, 0, path, &size)) return false;
    outPath.assign(path, size);
    return true;
}

std::string LockFilePath() {
    return DirectoryOf(GetCurrentModulePath()) + "\\" + kLockFileName;
}

HANDLE OpenTransientLockFile() {
    const std::string path = LockFilePath();
    DeleteFileA(path.c_str());
    return CreateFileA(
        path.c_str(),
        GENERIC_READ | DELETE,
        FILE_SHARE_READ | FILE_SHARE_DELETE,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE,
        nullptr);
}

void TerminateGeneratedSiblings() {
    const std::string currentPath = GetCurrentModulePath();
    const std::string currentDir = ToLower(DirectoryOf(currentPath));
    const std::string currentBase = ToLower(BaseNameOf(currentPath));

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return;

    PROCESSENTRY32W entry = {};
    entry.dwSize = sizeof(entry);
    if (!Process32FirstW(snapshot, &entry)) {
        CloseHandle(snapshot);
        return;
    }

    do {
        const DWORD pid = entry.th32ProcessID;
        if (pid == 0 || pid == GetCurrentProcessId()) continue;

        HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_TERMINATE | SYNCHRONIZE,
                                     FALSE, pid);
        if (!process) continue;

        std::string path;
        if (QueryProcessPath(process, path)) {
            const std::string dir = ToLower(DirectoryOf(path));
            const std::string base = BaseNameOf(path);
            const std::string lowerBase = ToLower(base);
            const bool sameDir = dir == currentDir;
            const bool sameExe = lowerBase == currentBase;
            if (sameDir && (sameExe || IsGeneratedOverlayName(base))) {
                TerminateProcess(process, 0);
                WaitForSingleObject(process, 3000);
            }
        }
        CloseHandle(process);
    } while (Process32NextW(snapshot, &entry));

    CloseHandle(snapshot);
}

} // namespace

namespace ProcessSingleInstance {

Guard::~Guard() {
    if (lock_file_ != INVALID_HANDLE_VALUE) {
        CloseHandle(lock_file_);
        lock_file_ = INVALID_HANDLE_VALUE;
    }
    if (mutex_) {
        ReleaseMutex(mutex_);
        CloseHandle(mutex_);
    }
}

bool Guard::Acquire() {
    TerminateGeneratedSiblings();

    for (int attempt = 0; attempt < 2; ++attempt) {
        mutex_ = CreateMutexA(nullptr, TRUE, kMutexName);
        const DWORD error = GetLastError();

        if (mutex_ && error != ERROR_ALREADY_EXISTS) {
            lock_file_ = OpenTransientLockFile();
            return true;
        }

        if (mutex_) {
            CloseHandle(mutex_);
            mutex_ = nullptr;
        }

        TerminateGeneratedSiblings();
        Sleep(300);
    }

    return false;
}

} // namespace ProcessSingleInstance
