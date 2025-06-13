#include <stdio.h>
#include "../support/WinRtUtils.hpp"
#include "MyServerImpl.hpp"


int wmain(int argc, wchar_t* argv[]) {
    // initialize COM
    winrt::init_apartment(winrt::apartment_type::multi_threaded);

    if (argc == 2) {
        std::wstring exe_path = GetModulePath(); // includes type library

        if (!_wcsicmp(argv[1] + 1, L"regserver")) {
            // register COM class
            auto tlbGuid = RegisterTypeLibrary(true, exe_path);
            RegisterComClass(true, __uuidof(MyServer), tlbGuid, exe_path);
            return 0;
        } else if (!_wcsicmp(argv[1] + 1, L"unregserver")) {
            // unregister COM class
            auto tlbGuid = RegisterTypeLibrary(false, exe_path);
            RegisterComClass(false, __uuidof(MyServer), tlbGuid, exe_path);
            return 0;
        }
    }

    // register class factory in current process
    DWORD registration = 0;
    winrt::check_hresult(::CoRegisterClassObject(__uuidof(MyServer), winrt::make<ClassFactory<MyServerImpl>>().get(), CLSCTX_LOCAL_SERVER, REGCLS_MULTIPLEUSE, &registration));

    wprintf(L"Waiting for COM class creation requests...\n");

    // sleep until object count drops to zero
    while (MyServerImpl::IsActive())
        Sleep(1000);

    winrt::uninit_apartment();
    return 0;
}
