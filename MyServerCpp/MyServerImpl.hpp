#pragma once
#define _USE_MATH_DEFINES
#include <math.h>
#include <vector>

#include <atlbase.h>
#include <atlcom.h>  // for CComObject
#include "MyInterfaces.tlh"
#include "Resource.h"


using namespace MyInterfaces;

class ATL_NO_VTABLE NumberCruncher :
    public CComObjectRootEx<CComMultiThreadModel>, // also compatible with single-threaded apartment
    public CComCoClass<NumberCruncher>, // no associated CLSID 
    public INumberCruncher
{
public:
    NumberCruncher() {
        printf("NumberCruncher ctor\n");
    }

    /*NOT virtual*/ ~NumberCruncher() {
        printf("NumberCruncher dtor\n");
    }

    /** Factory function. */
    static CComPtr<NumberCruncher> Create() {
        // create an object (with ref. count zero)
        CComObject<NumberCruncher>* tmp = nullptr;
        CComObject<NumberCruncher>::CreateInstance(&tmp);
        // move into smart-ptr (will incr. ref. count to one)
        return tmp;
    }

    HRESULT STDMETHODCALLTYPE raw_ComputePi(/*out*/double* val) override {
        if (!val)
            return E_INVALIDARG;

        *val = M_PI;
        return S_OK;
    }

    BEGIN_COM_MAP(NumberCruncher)
        COM_INTERFACE_ENTRY(INumberCruncher)
    END_COM_MAP()
};



class ATL_NO_VTABLE MyServerImpl :
    public CComObjectRootEx<CComMultiThreadModel>, // also compatible with single-threaded apartment
    public CComCoClass<MyServerImpl, &__uuidof(MyServer)>,
    public IMyServer
{
public:
    MyServerImpl() {
        printf("MyServerImpl ctor\n");
    }

    /*NOT virtual*/ ~MyServerImpl() {
        printf("MyServerImpl dtor\n");
    }

    HRESULT STDMETHODCALLTYPE raw_GetNumberCruncher(/*out*/INumberCruncher** obj) override {
        if (!obj)
            return E_INVALIDARG;

        *obj = NumberCruncher::Create().Detach();
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE raw_Subscribe(/*in*/IMyClient* client) override {
        if (!client)
            return E_INVALIDARG;

        printf("MyServerImpl Subscribe\n");
        m_clients.push_back(client);
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE raw_Unsubscribe(/*in*/IMyClient* client) override {
        if (!client)
            return E_INVALIDARG;

        for (auto it = m_clients.begin(); it != m_clients.end(); ++it) {
            if (it->GetInterfacePtr() == client) {
                printf("MyServerImpl Unsubscribe\n");
                m_clients.erase(it);
                return S_OK;
            }
        }
        return E_FAIL;
    }

    DECLARE_REGISTRY_RESOURCEID(IDR_MyServerImpl) // RGS file reference

    BEGIN_COM_MAP(MyServerImpl)
        COM_INTERFACE_ENTRY(IMyServer)
    END_COM_MAP()

private:
    std::vector<IMyClientPtr> m_clients;
};

OBJECT_ENTRY_AUTO(__uuidof(MyServer), MyServerImpl)
