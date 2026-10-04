#include <Windows.h>
#include <atlbase.h>
#include "../MyExeServerAtl/Resource.h" // shared between EXE & DLL probjects
#include "MyInterfaces.tlh"
#include "../support/ComSupport.hpp"


// exported symbols (in addition to DllMain)
#pragma comment(linker, "/export:DllCanUnloadNow,PRIVATE")
#pragma comment(linker, "/export:DllGetClassObject,PRIVATE")
#pragma comment(linker, "/export:DllRegisterServer,PRIVATE")
#pragma comment(linker, "/export:DllUnregisterServer,PRIVATE")


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
