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
};

MyserverModule _AtlModule;



// EXE Entry Point
int wmain(int /*argc*/, wchar_t* /*argv*/[]) {
    ComInitialize com(COINIT_MULTITHREADED);

    return _AtlModule.WinMain(SW_SHOWDEFAULT);
}
