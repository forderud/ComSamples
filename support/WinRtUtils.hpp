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


/** Minimal COM class factory implementation.
    TODO: Investigate replacement with wil::detail::CppWinRTClassFactory. */
template <class T>
class ClassFactory : public winrt::implements<ClassFactory<T>, IClassFactory, winrt::no_module_lock> {
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


/** Support class for controlling EXE process lifetime.
    winrt::get_module_lock() does unfortunately not suffice due to https://github.com/microsoft/cppwinrt/issues/1493 */
class LifetimeTracker {
public:
    LifetimeTracker() {
        s_obj_count++;
    }

    ~LifetimeTracker() {
        uint32_t new_count = --s_obj_count;
        if (!new_count && s_shutdown)
            SetEvent(s_shutdown); // signal shutdown
    }

    /** Must be called before CoRegisterClassObject. */
    static void Initialize() {
        s_shutdown = CreateEventW(NULL, false, false, NULL);
    }

    /** Wait until the COM object count drops to zero. */
    static void WaitForShutdown() {
        // wait until object count drops to zero
        WaitForSingleObject(s_shutdown, INFINITE);

        // clean up
        CloseHandle(s_shutdown);
        s_shutdown = 0;
    }

private:
    static std::atomic<uint32_t> s_obj_count;
    static HANDLE                s_shutdown;
};

std::atomic<uint32_t> LifetimeTracker::s_obj_count = 0;
HANDLE                LifetimeTracker::s_shutdown = 0;


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
void RegisterComClass(bool do_register, CLSID clsid, ::GUID tlbGuid, std::wstring dll_exe_path) {
    if (!IsUserAnAdmin()) {
        wprintf(L"ERROR: Admin privileges required for registration.\n");
        abort();
    }

    std::wstring clsid_str(38, L'\0');
    int ret = StringFromGUID2(clsid, clsid_str.data(), static_cast<int>(clsid_str.size() + 1));
    assert(ret == 39); ret; // includes zero-termination

    if (do_register) {
        // register COM class
        std::wstring file_ext = dll_exe_path.substr(dll_exe_path.find_last_of(L'.'));

        if (!_wcsicmp(file_ext.c_str(), L".exe")) {
            std::wstring key_path = L"CLSID\\" + clsid_str + L"\\LocalServer32";
            CRegKey key;
            LSTATUS res = key.Create(HKEY_CLASSES_ROOT, key_path.c_str());
            assert(res == ERROR_SUCCESS); res;

            key.SetStringValue(nullptr, dll_exe_path.c_str());
        } else if (!_wcsicmp(file_ext.c_str(), L".dll")) {
            std::wstring key_path = L"CLSID\\" + clsid_str + L"\\InprocServer32";
            CRegKey key;
            LSTATUS res = key.Create(HKEY_CLASSES_ROOT, key_path.c_str());
            assert(res == ERROR_SUCCESS); res;

            key.SetStringValue(nullptr, dll_exe_path.c_str());
        } else {
            wprintf(L"ERROR: Unsupported file extension.\n");
            abort();
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
