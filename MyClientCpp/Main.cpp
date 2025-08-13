#include <atlbase.h>
#include <atlcom.h> // for CComObject
#include <atlsafe.h> // for CComSafeArray
#include <iostream>
#include <vector>
#include <MyInterfaces.tlh>
#include "../support/ComSupport.hpp"
#include "../support/CoGetServerPID.hpp"
#include "ms-dcom_h.h"


/** Convert SAFEARRAY to a std::vector> */
template<class T>
std::vector<T> ToStdVector(const SAFEARRAY * sa) {
    CComSafeArray<T> arr;

    arr.Attach(sa);
    unsigned int size = arr.GetCount();
    std::vector<T> result(size);
    for (unsigned int i = 0; i < size; ++i)
        result[i] = arr[(LONG)i];
    
    arr.Detach();
    return result;
}


inline HRESULT GetTcpPortFromOXID(OXID oxid, uint16_t& port) {
    RPC_BINDING_HANDLE rpcConn = 0;
    {
        // RPC over TCP/IP MUST use port 135 (https://learn.microsoft.com/en-us/openspecs/windows_protocols/ms-rpce/95fbfb56-d67a-47df-900c-e263d6031f22)
        RPC_WSTR rpcConnStr = nullptr;
        RPC_STATUS status = RpcStringBindingComposeW(nullptr, RPC_WSTR(L"ncacn_ip_tcp"), RPC_WSTR(L"127.0.0.1"), RPC_WSTR(L"135"), NULL, &rpcConnStr);
        if (status != RPC_S_OK)
            return E_FAIL;

        status = RpcBindingFromStringBindingW(rpcConnStr, &rpcConn);
        if (status != RPC_S_OK)
            return E_FAIL;

        RpcStringFreeW(&rpcConnStr);
    }

    // Make OXID Resolver authenticate without a password
    RPC_STATUS status = RpcBindingSetAuthInfoExW(rpcConn,
        RPC_WSTR(L"NT Authority\\NetworkService"),
        RPC_C_AUTHN_LEVEL_PKT_INTEGRITY, // see https://learn.microsoft.com/en-us/openspecs/windows_protocols/ms-dcom/9b20084f-f673-4486-b81c-f6cdcc5edf84
        RPC_C_AUTHN_DEFAULT,
        NULL,
        RPC_C_AUTHZ_NONE,
        nullptr
    );
    if (status != RPC_S_OK)
        return E_FAIL;

    // Query OXID Resolver for COM server TCP port
    // This will cause the COM server to start listening on the returned TCP port
    unsigned short requestedProtocols[] = { 0x07 }; // TCP protocol ID from DCE 1.1: Remote Procedure Call
    DUALSTRINGARRAY* COMServerStringBindings = nullptr;
    IPID remoteUnknownIPID{};
    DWORD authHint = 0;
    error_status_t err = ResolveOxid(rpcConn, &oxid, _countof(requestedProtocols), requestedProtocols, &COMServerStringBindings, &remoteUnknownIPID, &authHint);
    if (err != RPC_S_OK)
        return E_FAIL;

    RpcBindingFree(&rpcConn);

    std::wstring_view strBindings((wchar_t*)COMServerStringBindings->aStringArray, COMServerStringBindings->wSecurityOffset);
    //std::wstring_view secBindings((wchar_t*)COMServerStringBindings->aStringArray + COMServerStringBindings->wSecurityOffset, COMServerStringBindings->wNumEntries - COMServerStringBindings->wSecurityOffset);

    std::wstring localPort; // server TCP port
    {
        size_t idx1 = strBindings.find(L"[");
        size_t idx2 = strBindings.find(L"]", idx1);

        localPort = strBindings.substr(idx1 + 1, idx2 - idx1 - 1);
    }

    port = (uint16_t)std::stoi(localPort);
    return S_OK;
}


/** Non-creatable COM class that doesn't need any CLSID. */
class MyClient : 
    public CComObjectRootEx<CComMultiThreadModel>, // also compatible with STA
    public CComCoClass<MyClient>, // no CLSID needed
    public MyInterfaces::IMyClient {
public:
    MyClient() {
        wprintf(L"MyClient constructor\n");
    }
    ~MyClient() {
        wprintf(L"MyClient destructor\n");
    }

    /** XmitMessage impl. */
    HRESULT raw_XmitMessage(MyInterfaces::Message * msg) override {
        if (!msg)
            return E_INVALIDARG;

        using namespace std;
        auto pid = GetClientProcessID();
        wcout << L"ClientPID: " << pid << L"\n";

        wcout << L"Received message:\n";
        wcout << L"  sev=" << msg->sev << L"\n";
        wcout << L"  time=" << msg->time << L"\n";
        wcout << L"  value=" << msg->value << L"\n";
        wcout << L"  desc=" << msg->desc << L"\n";
        wcout << L"  color=(" << msg->color[0] << L", " << msg->color[1] << L", " << msg->color[2] << L")\n";

        wcout << L"  data=[";
        if (msg->data) {
            for (BYTE elm : ToStdVector<BYTE>(msg->data))
                wcout << elm << L",";
        }
        wcout << L"]\n";

        return S_OK;
    }

    BEGIN_COM_MAP(MyClient)
        COM_INTERFACE_ENTRY(MyInterfaces::IMyClient)
    END_COM_MAP()
};


int main() {
    ComInitialize com(COINIT_MULTITHREADED);

    // Disable COM security to allow callbacks from elevated servers.
    HRESULT hr = CoInitializeSecurity(nullptr, -1/*auto*/, nullptr, NULL/*reserved*/,
        RPC_C_AUTHN_LEVEL_DEFAULT, ///< 
        RPC_C_IMP_LEVEL_IDENTIFY,  ///< allow server to identify but not impersonate client
        nullptr, EOAC_NONE/*capabilities*/, NULL/*reserved*/);
    if (FAILED(hr)) {
        _com_error err(hr);
        std::wcout << L"CoInitializeSecurity failure: " << err.ErrorMessage() << std::endl;
        return 1;
    }

    {
        // create or connect to server object in a separate process
        MyInterfaces::IMyServerPtr server;
        DWORD context = CLSCTX_ALL; // change to CLSCTX_LOCAL_SERVER to force out-of-proc
        hr = server.CreateInstance(__uuidof(MyInterfaces::MyServer), nullptr, context);
        if (FAILED(hr)) {
            _com_error err(hr);
            std::wcout << L"CoCreateInstance failure: " << err.ErrorMessage() << std::endl;
            return 1;
        }

        {
            COGETSERVERPID_STDOBJREF objref{};
            GetServerSTDOBJREF(server, /*out*/objref);
            uint16_t port = 0;
            GetTcpPortFromOXID(objref.oxid, port);
            std::wcout << L"Server TCP port: " << port << L"\n";
        }

        try {
            auto cruncher = server->GetNumberCruncher();
            double pi = cruncher->ComputePi();
            std::wcout << L"pi = " << pi << std::endl;

            auto callback = CreateLocalInstance<MyClient>();
            server->Subscribe(callback);

            // wait 5 seconds before exiting to give the server time to send messages
            Sleep(5000);

            // cruncher & callback references will be released here
        }
        catch (const _com_error& e) {
            std::wcout << L"Call failure: " << e.ErrorMessage() << std::endl;
            return 1;
        }

        // server reference will be released here
    }

    return 0;
}

// instantiate ATL
class MyClientModule : public CAtlExeModuleT<MyClientModule> {};
MyClientModule _Module;


void* midl_user_allocate(size_t cBytes) {
    return malloc(cBytes);
}

void midl_user_free(void* p) {
    free(p);
}
