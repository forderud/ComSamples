#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include "../support/WinRtUtils.hpp"

#include "MyInterfaces.tlh"
using namespace MyInterfaces;


/** Non-creatable COM class that doesn't need any CLSID. */
class NumberCruncher : public winrt::implements<NumberCruncher, INumberCruncher, winrt::no_weak_ref> {
public:
    NumberCruncher() {
    }

    ~NumberCruncher() {
    }

    HRESULT raw_ComputePi(double* val) override {
        if (!val)
            return E_INVALIDARG;

        *val = M_PI;
        return S_OK;
    }
};


/** Creatable COM class that needs a CLSID. */
class MyServerImpl : public winrt::implements<MyServerImpl, IMyServer, winrt::no_weak_ref>, public LifetimeTracker {
public:
    MyServerImpl() {
    }

    ~MyServerImpl() {
    }

    HRESULT raw_GetNumberCruncher(INumberCruncher** nc) override {
        if (!nc)
            return E_INVALIDARG;

        winrt::com_ptr<INumberCruncher> tmp = winrt::make<NumberCruncher>();
        *nc = tmp.detach();
        return S_OK;
    }

    HRESULT raw_Subscribe(IMyClient*) override {
        return E_NOTIMPL;
    }

    HRESULT raw_Unsubscribe(IMyClient*) override {
        return E_NOTIMPL;
    }
};


int wmain(int argc, wchar_t* argv[]) {
    // initialize COM
    winrt::init_apartment(winrt::apartment_type::multi_threaded);

    if (argc == 2) {
        std::wstring exe_path = GetModulePath(); // includes type library

        if (!wcscmp(argv[1], L"/regserver")) {
            // register COM class
            auto tlbGuid = RegisterTypeLibrary(true, exe_path);
            RegisterComExeClass(true, __uuidof(MyServer), tlbGuid, exe_path);
            return 0;
        } else if (!wcscmp(argv[1], L"/unregserver")) {
            // unregister COM class
            auto tlbGuid = RegisterTypeLibrary(false, exe_path);
            RegisterComExeClass(false, __uuidof(MyServer), tlbGuid, exe_path);
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

    return 0;
}
