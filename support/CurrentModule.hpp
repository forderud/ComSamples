#pragma once
#include <windows.h>

/** Return the folder path for the current EXE or DLL. */
inline std::wstring GetModuleFolderPath () {
    HMODULE module = nullptr;
    // Get handle to exe/dll that this static lib is linked against
    auto* module_ptr = (wchar_t const*)GetModuleFolderPath; // pointer to current function
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, module_ptr, &module);

    // retrieve full exe/dll path (incl. filename)
    wchar_t file_path[MAX_PATH] = {};
    GetModuleFileNameW(module, file_path, static_cast<DWORD>(std::size(file_path)));
    // Get path without filename part
    PathRemoveFileSpecW(file_path);
    return file_path;
}
