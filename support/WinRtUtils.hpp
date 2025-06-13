#pragma once
/* Utility code for COM class factory and COM class registration that seem to be missing from C++/WinRT.
   Please delete this header if a better alternative becomes available. */
#include <atomic>
#include <windows.h>
#include <atlbase.h> // for CRegKey
#include <Shlobj.h> // for IsUserAnAdmin
#include <unknwn.h>
#include <winrt/Windows.Foundation.h>
#include "../support/CurrentModule.hpp"


/** Minimal COM class factory implementation. */
template <class T>
class ClassFactory : public winrt::implements<ClassFactory<T>, IClassFactory> {
public:
    ClassFactory() {
#ifndef NDEBUG
        wprintf(L"ClassFactory ctor\n");
#endif
    }

    ~ClassFactory() {
#ifndef NDEBUG
        wprintf(L"ClassFactory dtor\n");
#endif
    }

    HRESULT CreateInstance(IUnknown* outer, const IID& iid, void** result) noexcept override {
        *result = nullptr;
        if (outer)
            return CLASS_E_NOAGGREGATION; // aggregation not supported yet

        // create object
        return winrt::make<T>()->QueryInterface(iid, result);
    }

    HRESULT LockServer(BOOL) noexcept override {
        return S_OK;
    }
};


/** Support class for keeping track of the object count. */
class LifetimeTracker {
public:
    LifetimeTracker() {
        s_obj_cnt++;
    }

    ~LifetimeTracker() {
        s_obj_cnt--;
        if (s_obj_cnt == 0)
            s_active = false; // ready to terminate process
    }

    /** The class is active until the object count is first incremeted, then decremented back to zero. */
    static bool IsActive() {
        return s_active;
    }

private:
    static inline std::atomic<bool>         s_active = true; ///< keep process alive
    static inline std::atomic<unsigned int> s_obj_cnt = 0;   ///< live object count
};



/** COM type library (un)registration function. */
::GUID RegisterTypeLibrary(bool do_register, std::wstring tlb_path) {
    if (!IsUserAnAdmin()) {
        wprintf(L"ERROR: Admin privileges required for registration.\n");
        abort();
    }

    ::GUID tlbGuid{};
    if (do_register) {
        // register typelib
        CComPtr<ITypeLib> tlb;
        HRESULT hr = LoadTypeLibEx(tlb_path.c_str(), REGKIND_REGISTER, &tlb);
        assert(SUCCEEDED(hr));

        TLIBATTR* attrPtr = nullptr;
        hr = tlb->GetLibAttr(&attrPtr);
        assert(SUCCEEDED(hr));

        tlbGuid = attrPtr->guid;

        tlb->ReleaseTLibAttr(attrPtr);
    } else {
        // unregister typelib
        CComPtr<ITypeLib> tlb;
        HRESULT hr = LoadTypeLibEx(tlb_path.c_str(), REGKIND_NONE, &tlb);
        assert(SUCCEEDED(hr));

        TLIBATTR* attrPtr = nullptr;
        hr = tlb->GetLibAttr(&attrPtr);
        assert(SUCCEEDED(hr));

        tlbGuid = attrPtr->guid;

        UnRegisterTypeLib(attrPtr->guid, attrPtr->wMajorVerNum, attrPtr->wMinorVerNum, attrPtr->lcid, attrPtr->syskind); // will fail if already unregistered

        tlb->ReleaseTLibAttr(attrPtr);
    }

    return tlbGuid;
}


/** COM class (un)registration function. */
void RegisterComExeClass(bool do_register, CLSID clsid, ::GUID tlbGuid, std::wstring exe_path) {
    if (!IsUserAnAdmin()) {
        wprintf(L"ERROR: Admin privileges required for registration.\n");
        abort();
    }

    std::wstring clsid_str(38, L'\0');
    int ret = StringFromGUID2(clsid, clsid_str.data(), static_cast<int>(clsid_str.size() + 1));
    assert(ret == 39); ret; // includes zero-termination

    if (do_register) {
        // register COM class
        {
            std::wstring key_path = L"CLSID\\" + clsid_str + L"\\LocalServer32";
            CRegKey key;
            LSTATUS res = key.Create(HKEY_CLASSES_ROOT, key_path.c_str());
            assert(res == ERROR_SUCCESS); res;

            key.SetStringValue(nullptr, exe_path.c_str());
        }
        {
            std::wstring key_path = L"CLSID\\" + clsid_str + L"\\TypeLib";
            CRegKey key;
            LSTATUS res = key.Create(HKEY_CLASSES_ROOT, key_path.c_str());
            assert(res == ERROR_SUCCESS); res;

            wchar_t tlb_str[39] = {};
            StringFromGUID2(tlbGuid, tlb_str, static_cast<int>(std::size(tlb_str)));
            key.SetStringValue(nullptr, tlb_str);
        }
    } else {
        // unregister COM class
        CRegKey parent;
        LSTATUS res = parent.Open(HKEY_CLASSES_ROOT, L"CLSID", KEY_READ | KEY_WRITE);
        assert(res == ERROR_SUCCESS); res;

        parent.RecurseDeleteKey(clsid_str.c_str()); // will fail if already unregistered
    }
}
