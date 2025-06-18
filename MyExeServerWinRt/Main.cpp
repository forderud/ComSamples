#include <stdio.h>
#include <functional>

#define WINRT_CUSTOM_MODULE_LOCK
#include <wil/resource.h>
#include <wil/cppwinrt_notifiable_module_lock.h>
#include <wil/cppwinrt_register_com_server.h>

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

    wil::unique_event _comExit;
    _comExit.create();

    wil::notifiable_module_lock::instance().set_notifier([&]() {
        _comExit.SetEvent();
        });
    auto resetOnExit = wil::scope_exit([&] {
        wil::notifiable_module_lock::instance().set_notifier(nullptr);
        });

    // register class factory in current process
    // TODO: Fix buggy wil::register_com_server implementation (see https://github.com/microsoft/wil/pull/533)
    auto revoker = wil::register_com_server<MyServerImpl>(); // registers with CLSCTX_LOCAL_SERVER, REGCLS_MULTIPLEUSE

    wprintf(L"Waiting for COM class creation requests...\n");

    // wait until object count drops to zero
    _comExit.wait();

    winrt::uninit_apartment(); // will decrement get_module_lock()
    return 0;
}
