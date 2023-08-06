#include <Windows.h>
#include <atlbase.h>
#include "Resource.h"
#include "MyInterfaces.tlh"


class MyserverModule : public ATL::CAtlExeModuleT<MyserverModule> {
public:
    MyserverModule() {
    }

    DECLARE_LIBID(__uuidof(MyInterfaces::__MyInterfaces))
};

MyserverModule _AtlModule;



// EXE Entry Point
int wmain(int /*argc*/, wchar_t* /*argv*/[]) {
    // initialize COM early for programmatic COM security
    _AtlModule.InitializeCom();

    return _AtlModule.WinMain(SW_SHOWDEFAULT);
}
