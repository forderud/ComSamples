#pragma once
#define _USE_MATH_DEFINES
#include <math.h>
#include <thread>
#include <vector>

#include <atlbase.h>
#include <atlcom.h>  // for CComObject
#include <ATLComTime.h> // for COleDateTime
#include "MyInterfaces.tlh"
#include "Resource.h"
#include "../support/ComSupport.hpp"


using namespace MyInterfaces;

/** Non-creatable COM class that doesn't need any CLSID. */
class ATL_NO_VTABLE NumberCruncher :
    public CComObjectRootEx<CComMultiThreadModel>, // also compatible with STA
    public CComCoClass<NumberCruncher>, // no CLSID needed
    public INumberCruncher,
    public INumberCruncher2
{
public:
    NumberCruncher() {
        printf("NumberCruncher ctor\n");
    }

    /*NOT virtual*/ ~NumberCruncher() {
        printf("NumberCruncher dtor\n");
    }

    HRESULT raw_ComputePi(/*out*/double* val) override {
        if (!val)
            return E_INVALIDARG;

        *val = M_PI + 0.01; // old inaccurate impl.
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE raw_ComputePi2(/*out*/double* val) override {
        if (!val)
            return E_INVALIDARG;

        *val = M_PI;
        return S_OK;
    }

    BEGIN_COM_MAP(NumberCruncher)
        COM_INTERFACE_ENTRY(INumberCruncher)
        COM_INTERFACE_ENTRY(INumberCruncher2)
    END_COM_MAP()
};


/** Creatable COM class that needs a CLSID. */
class ATL_NO_VTABLE MyServerImpl :
    public CComObjectRootEx<CComMultiThreadModel>, // also compatible with STA
    public CComCoClass<MyServerImpl, &__uuidof(MyServer)>, // CLSID
    public IMyServer
{
public:
    MyServerImpl() {
        printf("MyServerImpl ctor\n");

        m_thread = std::thread(&MyServerImpl::ThreadStart, this);
    }

    /*NOT virtual*/ ~MyServerImpl() {
        printf("MyServerImpl dtor\n");

        m_active = false;
        m_thread.join();
    }

    HRESULT raw_GetNumberCruncher(/*out*/INumberCruncher** obj) override {
        if (!obj)
            return E_INVALIDARG;

        *obj = CreateLocalInstance<NumberCruncher>().Detach();
        return S_OK;
    }

    HRESULT raw_Subscribe(/*in*/IMyClient* client) override {
        if (!client)
            return E_INVALIDARG;

        printf("MyServerImpl Subscribe\n");
        m_clients.push_back(client);
        return S_OK;
    }

    HRESULT raw_Unsubscribe(/*in*/IMyClient* client) override {
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

    void ThreadStart() {
        while (m_active) {
            if (m_clients.size() > 0) {
                printf("Broadcasting message to all subscribed clients.\n");
                Message msg;
                msg.sev = Severity::Info;
                msg.time = COleDateTime::GetCurrentTime();
                msg.value = 1.23;
                msg.desc = "Hello there!";
                msg.color[0] = 255;
                msg.color[1] = 0;
                msg.color[2] = 0;

                CComSafeArray<byte> data(4);
                for (int i = 0; i < (int)data.GetCount(); ++i)
                    data[i] = (byte)i;
                msg.data = data.Detach();

                BroadcastMessage(msg);
            } else {
                printf("No clients subscribed.\n");

            }

            Sleep(2000); // wait 2 seconds
        }

    }

    /** Broadcast message to all connected clients. Will disconnect clients on RPC failure. */
    void BroadcastMessage(Message& msg) {
        for (auto it = m_clients.begin(); it != m_clients.end();) {
            try {
                (*it)->XmitMessage(&msg);
            } catch (_com_error& err) {
                HRESULT hr = err.Error();

                constexpr HRESULT WIN32_ERR = (0x8000 | FACILITY_WIN32) << 16; // high-order part of Win32 error code
                if (hr == (WIN32_ERR | RPC_S_SERVER_UNAVAILABLE)) {
                    // expect RPC_S_SERVER_UNAVAILABLE (0x800706BA) when client disconnect
                    printf("Disconnecting client after call failure (RPC_S_SERVER_UNAVAILABLE)\n");

                    it = m_clients.erase(it);
                    continue;
                }
            }

            // advance to next element on success
            it++;
        }
    }

    DECLARE_REGISTRY_RESOURCEID(IDR_MyServerImpl) // RGS file reference

    BEGIN_COM_MAP(MyServerImpl)
        COM_INTERFACE_ENTRY(IMyServer)
    END_COM_MAP()

private:
    bool m_active = true;
    std::thread m_thread;
    std::vector<IMyClientPtr> m_clients;
};

OBJECT_ENTRY_AUTO(__uuidof(MyServer), MyServerImpl)
