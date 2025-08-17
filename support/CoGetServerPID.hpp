#pragma once
#include <stdexcept>

#pragma comment(lib, "Rpcrt4.lib") // for RpcServerInqCallAttributesW


/** Get client process ID (PID) during handling of incoming RPC call.
    Only supported for the "ncalrpc" local RPC protocol. */
inline DWORD GetClientProcessID(RPC_BINDING_HANDLE binding = nullptr) {
    RPC_CALL_ATTRIBUTES attribs{};
    attribs.Version = RPC_CALL_ATTRIBUTES_VERSION; // 3
    attribs.Flags = RPC_QUERY_CLIENT_PID;
    RPC_STATUS status = RpcServerInqCallAttributesW(binding, &attribs);
    if (status != RPC_S_OK)
        throw std::runtime_error("RPC_QUERY_CLIENT_PID failed");

    return (DWORD)(size_t)(attribs.ClientPID); // truncating cast should be safe, since GetCurrentProcessId() returns a DWORD
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
static_assert(sizeof(COGETSERVERPID_IPID) == 16, "IPID size mismatch");

#pragma pack(push, 1)
struct COGETSERVERPID_STDOBJREF {
    unsigned long       flags;       // SORF_ flags (see above)
    unsigned long       cPublicRefs; // count of references passed
    unsigned hyper      oxid;        // oxid of server with this oid
    unsigned hyper      oid;         // oid of object with this ipid
    COGETSERVERPID_IPID ipid;        // ipid of Interface
};
#pragma pack(pop)
static_assert(sizeof(COGETSERVERPID_STDOBJREF) == 2 * 4 + 2 * 8 + 16, "STDOBJREF size mismatch");


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
static_assert(sizeof(COGETSERVERPID_OBJREF) == 2 * 4 + 16 + 40, "OBJREF size mismatch");



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
