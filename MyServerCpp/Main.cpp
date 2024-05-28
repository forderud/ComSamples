#include <Windows.h>
#include <atlbase.h>
#include "Resource.h"
#include "MyInterfaces.tlh"
#include "../support/ComSupport.hpp"


class MyserverModule : public ATL::CAtlExeModuleT<MyserverModule> {
public:
    MyserverModule() {
    }

    DECLARE_LIBID(__uuidof(MyInterfaces::__MyInterfaces))
    DECLARE_REGISTRY_APPID_RESOURCEID(IDR_AppID, "{AF080472-F173-4D9D-8BE7-435776617347}")

    HRESULT InitializeSecurity() noexcept {
        // Disable COM security to allow any client to connect.
        // WARNING: Enables non-admin clients to connect to a server running with admin privileges.
        HRESULT hr = CoInitializeSecurity(nullptr, -1/*auto*/, nullptr, NULL/*reserved*/,
            RPC_C_AUTHN_LEVEL_DEFAULT, ///< 
            RPC_C_IMP_LEVEL_IDENTIFY,  ///< allow server to identify but not impersonate client
            nullptr, EOAC_NONE/*capabilities*/, NULL/*reserved*/);
        return hr;
    }
};

MyserverModule _AtlModule;



// EXE Entry Point
int wmain(int /*argc*/, wchar_t* /*argv*/[]) {
    // initialize COM early for programmatic COM security
    _AtlModule.InitializeCom();
    HRESULT hr = _AtlModule.InitializeSecurity();
    if (FAILED(hr))
        abort();

    return _AtlModule.WinMain(SW_SHOWDEFAULT);
}
