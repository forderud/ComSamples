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

/** Interface Pointer IDentifier (IPID). A 128-bit number that uniquely
    identifies an interface on an object within an object exporter. */
#pragma pack(push, 1)
struct COGETSERVERPID_IPID {
    unsigned short field1;
    unsigned short field2;
    unsigned short pid;    ///< process ID (clamped on overflow)
    unsigned short field4;
    unsigned short fields[4];

    bool IsPidValid() const {
        return pid != 0xFFFF; // detect clamped PID
    }
};
#pragma pack(pop)
static_assert(sizeof(COGETSERVERPID_IPID) == 16, "COGETSERVERPID_IPID size mismatch");

#pragma pack(push, 1)
struct COGETSERVERPID_STDOBJREF {
    unsigned long       flags;       // SORF_ flags (see above)
    unsigned long       cPublicRefs; // count of references passed
    unsigned hyper      oxid;        // oxid of server with this oid
    unsigned hyper      oid;         // oid of object with this ipid
    COGETSERVERPID_IPID ipid;        // ipid of Interface
};
#pragma pack(pop)

/** Partial OBJREF structure up to PID member at offset 52 bytes.
    Packed struct to make offsets deterministic.
    REF: https://en.wikipedia.org/wiki/OBJREF */
#pragma pack(push, 1)
struct COGETSERVERPID_OBJREF {
    unsigned long            signature; ///< Should be 'MEOW'
    unsigned long            flags;     ///< 1 (OBJREF_STANDARD)
    GUID                     iid;       ///< IID_IUnknown
    COGETSERVERPID_STDOBJREF std;

    bool IsValid() const {
        return (signature == 0x574f454d) && (flags == 1);
    }
};
#pragma pack(pop)


/** Get the Process ID of a COM server running in a different process. */
inline HRESULT GetServerSTDOBJREF(IUnknown* obj, /*out*/COGETSERVERPID_STDOBJREF& objref) {
    if (obj == NULL)
        return E_INVALIDARG;

    {
        // IProxyManager proxy type required to assume COGETSERVERPID_OBJREF format
        CComPtr<IUnknown> proxy_mgr;
        HRESULT hr = obj->QueryInterface(IID_IProxyManager, (void**)&proxy_mgr);
        if (FAILED(hr))
            return hr;
    }

    // create IStream object
    CComPtr<IStream> stream;
    HRESULT hr = ::CreateStreamOnHGlobal(NULL, TRUE/*delete on release*/, &stream);
    if (FAILED(hr))
        return hr;

    // marshal "obj" into stream
    hr = ::CoMarshalInterface(stream, IID_IUnknown, obj, MSHCTX_INPROC, NULL, MSHLFLAGS_NORMAL);
    if (FAILED(hr))
        return hr;

    // map stream into memory
    HGLOBAL hg = NULL;
    hr = ::GetHGlobalFromStream(stream, &hg);
    if (SUCCEEDED(hr)) {
        hr = RPC_E_INVALID_OBJREF; // assume failure until proven othervise

        auto* header = (COGETSERVERPID_OBJREF*)GlobalLock(hg);
        size_t header_size = GlobalSize(hg);
        if (header && (header_size >= sizeof(COGETSERVERPID_OBJREF))) {
            if (header->IsValid()) {
                objref = header->std;
                hr = S_OK;
            }

            GlobalUnlock(hg);
        }
    }

    // rewind stream and release marshal data
    LARGE_INTEGER zero{};
    stream->Seek(zero, SEEK_SET, NULL);
    CoReleaseMarshalData(stream);

    return hr;
}


static uint16_t GetTcpPortFromOXID(OXID oxid) {
    RPC_BINDING_HANDLE rpcConn = 0;
    {
        // RPC over TCP/IP MUST use port 135 (https://learn.microsoft.com/en-us/openspecs/windows_protocols/ms-rpce/95fbfb56-d67a-47df-900c-e263d6031f22)
        RPC_WSTR rpcConnStr = nullptr;
        RPC_STATUS status = RpcStringBindingComposeW(nullptr, RPC_WSTR(L"ncacn_ip_tcp"), RPC_WSTR(L"127.0.0.1"), RPC_WSTR(L"135"), NULL, &rpcConnStr);
        assert(status == RPC_S_OK);

        status = RpcBindingFromStringBindingW(rpcConnStr, &rpcConn);
        assert(status == RPC_S_OK);

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
    assert(status == RPC_S_OK);

    // Query OXID Resolver for COM server TCP port
    // This will cause the COM server to start listening on the returned TCP port
    unsigned short requestedProtocols[] = { 0x07 }; // TCP protocol ID from DCE 1.1: Remote Procedure Call
    DUALSTRINGARRAY* COMServerStringBindings = nullptr;
    IPID remoteUnknownIPID{};
    DWORD authHint = 0;
    error_status_t err = ResolveOxid(rpcConn, &oxid, _countof(requestedProtocols), requestedProtocols, &COMServerStringBindings, &remoteUnknownIPID, &authHint);
    assert(err == RPC_S_OK);

    RpcBindingFree(&rpcConn);

    std::wstring_view strBindings((wchar_t*)COMServerStringBindings->aStringArray, COMServerStringBindings->wSecurityOffset);
    //std::wstring_view secBindings((wchar_t*)COMServerStringBindings->aStringArray + COMServerStringBindings->wSecurityOffset, COMServerStringBindings->wNumEntries - COMServerStringBindings->wSecurityOffset);

    std::wstring localPort; // server TCP port
    {
        size_t idx1 = strBindings.find(L"[");
        size_t idx2 = strBindings.find(L"]", idx1);

        localPort = strBindings.substr(idx1 + 1, idx2 - idx1 - 1);
    }

    return (uint16_t)std::stoi(localPort);
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
            uint16_t port = GetTcpPortFromOXID(objref.oxid);
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
