#include <Windows.h>
#include <atlbase.h>
#include "../MyExeServerCpp/Resource.h" // shared between EXE & DLL probjects
#include "MyInterfaces.tlh"
#include "../support/ComSupport.hpp"


class MyserverModule : public ATL::CAtlDllModuleT<MyserverModule> {
public:
    MyserverModule() {
    }

    DECLARE_LIBID(__uuidof(MyInterfaces::__MyInterfaces))
    DECLARE_REGISTRY_APPID_RESOURCEID(IDR_AppID, "{AF080472-F173-4D9D-8BE7-435776617347}")
};

MyserverModule _AtlModule;



// DLL Entry Point
extern "C" BOOL WINAPI DllMain(HINSTANCE /*hInstance*/, DWORD dwReason, LPVOID lpReserved) {
    return _AtlModule.DllMain(dwReason, lpReserved);
}

// Used to determine whether the DLL can be unloaded by OLE.
STDAPI DllCanUnloadNow() {
    return _AtlModule.DllCanUnloadNow();
}

// Returns a class factory to create an object of the requested type.
_Check_return_
STDAPI DllGetClassObject(_In_ REFCLSID rclsid, _In_ REFIID riid, _Outptr_ LPVOID* ppv) {
    return _AtlModule.DllGetClassObject(rclsid, riid, ppv);
}

// DllRegisterServer - Adds entries to the system registry.
STDAPI DllRegisterServer() {
    // registers object, typelib and all interfaces in typelib
    return _AtlModule.DllRegisterServer();
}

// DllUnregisterServer - Removes entries from the system registry.
STDAPI DllUnregisterServer() {
    return _AtlModule.DllUnregisterServer();
}

// DllInstall - Adds/Removes entries to the system registry per user per machine.
STDAPI DllInstall(BOOL bInstall, _In_opt_  LPCWSTR pszCmdLine) {
    static const wchar_t szUserSwitch[] = L"user";

    if (pszCmdLine != NULL) {
        if (_wcsnicmp(pszCmdLine, szUserSwitch, _countof(szUserSwitch)) == 0)
            ATL::AtlSetPerUserRegistration(true);
    }

    HRESULT hr = E_FAIL;
    if (bInstall) {
        hr = DllRegisterServer();
        if (FAILED(hr))
            DllUnregisterServer();
    }
    else {
        hr = DllUnregisterServer();
    }

    return hr;
}
