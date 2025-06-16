#pragma once
#define _USE_MATH_DEFINES // M_PI
#include <math.h>

#include "MyInterfaces.tlh"
using namespace MyInterfaces;


/** Non-creatable COM class that doesn't need any CLSID. */
class NumberCruncher : public winrt::implements<NumberCruncher, INumberCruncher, winrt::no_weak_ref> {
public:
    NumberCruncher() {
#ifndef NDEBUG
        wprintf(L"NumberCruncher ctor\n");
#endif
    }

    ~NumberCruncher() {
#ifndef NDEBUG
        wprintf(L"NumberCruncher dtor\n");
#endif
    }

    HRESULT raw_ComputePi(double* val) override {
        if (!val)
            return E_INVALIDARG;

        *val = M_PI;
        return S_OK;
    }
};


/** Creatable COM class that needs a CLSID. */
class MyServerImpl : public winrt::implements<MyServerImpl, IMyServer, winrt::no_weak_ref> {
public:
    MyServerImpl() {
#ifndef NDEBUG
        wprintf(L"MyServerImpl ctor\n");
#endif
    }

    ~MyServerImpl() {
#ifndef NDEBUG
        wprintf(L"MyServerImpl dtor\n");
#endif
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
