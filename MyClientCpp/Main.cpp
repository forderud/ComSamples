#include <atlbase.h>
#include <atlcom.h> // for CComObject
#include <atlsafe.h> // for CComSafeArray
#include <iostream>
#include <mutex>
#include <vector>
#include "../MyInterfaces/MyInterfaces_h.h"
#include "../support/ComSupport.hpp"
#include "SharedRef.hpp"

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


/** Non-creatable COM class that doesn't need any CLSID. */
class MyClient : 
    public CComObjectRootEx<CComMultiThreadModel>, // also compatible with STA
    public CComCoClass<MyClient>, // no CLSID needed
    public IMyClient {
public:
    MyClient() {
        wprintf(L"MyClient constructor\n");
        ++s_obj_count;
    }
    ~MyClient() {
        wprintf(L"MyClient destructor\n");
        --s_obj_count;
    }

    /** XmitMessage impl. */
    HRESULT XmitMessage(Message * msg) override {
        if (!msg)
            return E_INVALIDARG;

        using namespace std;

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
        COM_INTERFACE_ENTRY(IMyClient)
    END_COM_MAP()

    static inline ULONG s_obj_count = 0;
};


int main() {
    ComInitialize com(COINIT_MULTITHREADED);

    {
        CComPtr<IUnknown> strong;
        CComPtr<IWeakRef> weak;
        {
            CComPtr<IUnknown> obj(new SharedRef<IMyClient, MyClient>());

            obj->QueryInterface(__uuidof(IUnknown), (void**)&strong);
            obj->QueryInterface(__uuidof(IWeakRef), (void**)&weak);
        }

        {
            // verify that Resolve succeed when use-count>0
            CComPtr<IUnknown> tmp;
            weak->Resolve(&tmp);

            CComPtr<IMyClient> obj;
            obj = tmp;
            Message m;
            obj->XmitMessage(&m);
        }

        {
            // cast IWeakRef to IUnknown
            CComPtr<IUnknown> tmp;
            weak->QueryInterface(&tmp);
        }

        strong.Release();

        {
            // verify that Resolve fail when use-count=0
            CComPtr<IUnknown> tmp;
            HRESULT hr = weak->Resolve(&tmp);
            assert(hr == E_FAIL);
        }
    }
    auto count = SharedRef<IMyClient, MyClient>::ObjectCount();
    assert(count == 0);
    assert(MyClient::s_obj_count == 0);

    return 0;
}

// instantiate ATL
class MyClientModule : public CAtlExeModuleT<MyClientModule> {};
MyClientModule _Module;
