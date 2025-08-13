#pragma once
#include <stdexcept>
#include <atlcom.h>  // for CComObject

#pragma comment(lib, "Rpcrt4.lib") // for RpcServerInqCallAttributesW

/* Disable BSTR caching to ease memory management debugging.
   REF: https://docs.microsoft.com/en-us/previous-versions/windows/desktop/automat/setoanocache */
extern "C" void SetOaNoCache ();


/** RAII class for COM initialization. */
class ComInitialize {
public:
    ComInitialize (COINIT apartment /*= COINIT_MULTITHREADED*/) : m_initialized(false) {
        // REF: https://docs.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-coinitializeex
        HRESULT hr = CoInitializeEx(NULL, apartment);
        if (SUCCEEDED(hr))
            m_initialized = true;

        // Activate fast stub rundown after app crashes. Reduces the cleanup delay from ~11min to <10sec
        CComPtr<IGlobalOptions> globalOptions;
        if (FAILED(globalOptions.CoCreateInstance(CLSID_GlobalOptions, NULL, CLSCTX_INPROC_SERVER)))
            abort();
        if (FAILED(globalOptions->Set(COMGLB_RO_SETTINGS, COMGLB_FAST_RUNDOWN)))
            abort();

#ifndef NDEBUG
        // Disable BSTR caching to make use-after-free bugs crash
        SetOaNoCache();
#endif
    }

    ~ComInitialize () {
        if (m_initialized)
            CoUninitialize();
    }

private:
    bool m_initialized; ///< must uninitialize in dtor
};

/** Convenience function to create a locally implemented COM instance without the overhead of CoCreateInstance.
    The COM class does not need to be registred for construction to succeed. However, lack of registration can
    cause problems if transporting the class out-of-process. */
template <class T>
CComPtr<T> CreateLocalInstance () {
    // create an object (with ref. count zero)
    CComObject<T> * tmp = nullptr;
    if (FAILED(CComObject<T>::CreateInstance(&tmp)))
        abort(); // deliberately crash

    // move into smart-ptr (will incr. ref. count to one)
    return CComPtr<T>(static_cast<T*>(tmp));
}


/** Get remote process ID (PID) during handling of incoming COM calls.
    Only supported for the "ncalrpc" local RPC protocol. */
static size_t GetRemoteProcessID() {
    RPC_CALL_ATTRIBUTES attribs{};
    attribs.Version = RPC_CALL_ATTRIBUTES_VERSION; // 3
    attribs.Flags = RPC_QUERY_CLIENT_PID;
    RPC_BINDING_HANDLE binding = nullptr;
    RPC_STATUS status = RpcServerInqCallAttributesW(binding, &attribs);
    if (status != RPC_S_OK)
        throw std::runtime_error("RPC_QUERY_CLIENT_PID failed");

    return (size_t)attribs.ClientPID;
}
