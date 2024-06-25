#include <atlbase.h>
#include <atlcom.h> // for CComObject
#include <atlsafe.h> // for CComSafeArray
#include <iostream>
#include <vector>
#include <MyInterfaces.tlh>
#include "../support/ComSupport.hpp"


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


    return 0;
}

// instantiate ATL
class MyClientModule : public CAtlExeModuleT<MyClientModule> {};
MyClientModule _Module;
