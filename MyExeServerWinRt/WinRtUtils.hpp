#pragma once
/* Utility code for COM class factory and COM class registration that seem to be missing from C++/WinRT.
   Please delete this header if a better alternative becomes available. */
#include <windows.h>
#include <atlbase.h> // for CRegKey
#include <Shlobj.h> // for IsUserAnAdmin
#include <unknwn.h>
#include <winrt/Windows.Foundation.h>
#include "../support/CurrentModule.hpp"


/** Minimal COM class factory implementation. */
template <class T>
struct ClassFactory : winrt::implements<ClassFactory<T>, IClassFactory> {
    ClassFactory() {
    }

    ~ClassFactory() {
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
    static inline bool s_active = true; ///< keep process alive
    static inline int  s_obj_cnt = 0;    ///< live object count
};


/** COM class (un)registration function. */
void RegisterComClass(bool do_register, CLSID clsid, std::wstring exe_path) {
    if (!IsUserAnAdmin()) {
        wprintf(L"ERROR: Admin privileges required for registration.\n");
        return;
    }

    ::GUID tlbGuid{};
    if (do_register) {
        // register typelib
        CComPtr<ITypeLib> tlb;
        HRESULT hr = LoadTypeLibEx(exe_path.c_str(), REGKIND_REGISTER, &tlb);
        assert(SUCCEEDED(hr));

        TLIBATTR* attrPtr = nullptr;
        hr = tlb->GetLibAttr(&attrPtr);
        assert(SUCCEEDED(hr));

        tlbGuid = attrPtr->guid;

        tlb->ReleaseTLibAttr(attrPtr);
    } else {
        // unregister typelib
        CComPtr<ITypeLib> tlb;
        HRESULT hr = LoadTypeLibEx(exe_path.c_str(), REGKIND_NONE, &tlb);
        assert(SUCCEEDED(hr));

        TLIBATTR* attrPtr = nullptr;
        hr = tlb->GetLibAttr(&attrPtr);
        assert(SUCCEEDED(hr));

        tlbGuid = attrPtr->guid;

        UnRegisterTypeLib(attrPtr->guid, attrPtr->wMajorVerNum, attrPtr->wMinorVerNum, attrPtr->lcid, attrPtr->syskind); // will fail if already unregistered

        tlb->ReleaseTLibAttr(attrPtr);
    }

    wchar_t clsid_str[39] = {};
    StringFromGUID2(clsid, clsid_str, static_cast<int>(std::size(clsid_str)));

    if (do_register) {
        // register COM class
        {
            std::wstring key_path = L"CLSID\\" + std::wstring(clsid_str) + L"\\LocalServer32";
            CRegKey key;
            LSTATUS res = key.Create(HKEY_CLASSES_ROOT, key_path.c_str());
            assert(res == ERROR_SUCCESS);

            key.SetStringValue(nullptr, exe_path.c_str());
        }
        {
            std::wstring key_path = L"CLSID\\" + std::wstring(clsid_str) + L"\\TypeLib";
            CRegKey key;
            LSTATUS res = key.Create(HKEY_CLASSES_ROOT, key_path.c_str());
            assert(res == ERROR_SUCCESS);

            wchar_t tlb_str[39] = {};
            StringFromGUID2(tlbGuid, tlb_str, static_cast<int>(std::size(tlb_str)));
            key.SetStringValue(nullptr, tlb_str);
        }
    } else {
        // unregister COM class
        CRegKey parent;
        LSTATUS res = parent.Open(HKEY_CLASSES_ROOT, L"CLSID", KEY_READ | KEY_WRITE);
        assert(res == ERROR_SUCCESS);

        parent.RecurseDeleteKey(clsid_str); // will fail if already unregistered
    }
}
