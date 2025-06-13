#include <Windows.h>
#include <unknwn.h>
#include <winrt/Windows.Foundation.h>
#include "../support/WinRtUtils.hpp"
#include "../MyExeServerWinRt/MyServerImpl.hpp"


/** DLL entry point */
BOOL APIENTRY DllMain (HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

// Used to determine whether the DLL can be unloaded by OLE.
STDAPI DllCanUnloadNow() {
    if (winrt::get_module_lock()) {
        return S_FALSE;
    }

    winrt::clear_factory_cache();
    return S_OK;
}

// Returns a class factory to create an object of the requested type.
STDAPI DllGetClassObject(::GUID const& clsid, ::GUID const& iid, void** result) {
    *result = nullptr;

    if (clsid == __uuidof(MyServer)) {
        return winrt::make<ClassFactory<MyServerImpl>>()->QueryInterface(iid, result);
    }

    return winrt::hresult_class_not_available().to_abi();
}

// DllRegisterServer - Adds entries to the system registry.
STDAPI DllRegisterServer() {
    std::wstring dll_path = GetModulePath(); // includes type library

    auto tlbGuid = RegisterTypeLibrary(true, dll_path);
    RegisterComClass(true, __uuidof(MyServer), tlbGuid, dll_path);
    return S_OK;
}

// DllUnregisterServer - Removes entries from the system registry.
STDAPI DllUnregisterServer() {
    std::wstring dll_path = GetModulePath(); // includes type library

    auto tlbGuid = RegisterTypeLibrary(false, dll_path);
    RegisterComClass(false, __uuidof(MyServer), tlbGuid, dll_path);
    return S_OK;
}
