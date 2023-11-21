#pragma once

#include <atlcom.h>  // for CComObject


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

#ifndef NDEBUG
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
