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

        m_thread = std::thread(&MyServerImpl::ThreadStart, this);
    }

    /*NOT virtual*/ ~MyServerImpl() {
        printf("MyServerImpl dtor\n");

        m_active = false;
        m_thread.join();
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
                (*it)->SendMessage(&msg);
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
