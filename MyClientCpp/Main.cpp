#include <atlbase.h>
#include <atlcom.h> // for CComObject
#include <atlsafe.h> // for CComSafeArray
#include <iostream>
#include <vector>
#include <MyInterfaces.tlh>


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
    /** SendMessage impl. */
    HRESULT raw_SendMessage(MyInterfaces::Message * msg) override {
        if (!msg)
            return E_INVALIDARG;

        using namespace std;

        wcout << L"Received message:\n";
        wcout << L"  sev=" << msg->sev << L"\n";
        wcout << L"  time=" << msg->time << L"\n";
        wcout << L"  value=" << msg->value << L"\n";
        wcout << L"  desc=" << msg->desc << L"\n";
        wcout << L"  color=(" << msg->color[0] << L", " << msg->color[1] << L", " << msg->color[2] << L")\n";
        {
            wcout << L"  data=[";
            for (BYTE elm : ToStdVector<BYTE>(msg->data))
                wcout << elm << L",";
            wcout << L"]\n";
        }
        return S_OK;
    }

    /** Factory function. */
    static CComPtr<MyClient> Create() {
        // create an object (with ref. count zero)
        CComObject<MyClient> * obj = nullptr;
        CComObject<MyClient>::CreateInstance(&obj);
        // move into smart-ptr (will incr. ref. count to one)
        return obj;
    }

    BEGIN_COM_MAP(MyClient)
        COM_INTERFACE_ENTRY(MyInterfaces::IMyClient)
    END_COM_MAP()
};


int main() {
    CoInitializeEx(0, COINITBASE_MULTITHREADED);

    // create or connect to server object in a separate process
    MyInterfaces::IMyServerPtr server;
    HRESULT hr = server.CreateInstance(__uuidof(MyInterfaces::MyServer));
    if (FAILED(hr)) {
        _com_error err(hr);
        std::wcout << L"CoCreateInstance failure: " << err.ErrorMessage() << std::endl;
        return 1;
    }

    try {
        auto cruncher = server->GetNumberCruncher();
        double pi = cruncher->ComputePi();
        std::wcout << L"pi = " << pi << std::endl;

        auto callback = MyClient::Create();
        server->Subscribe(callback);

        // wait 5 seconds before exiting to give the server time to send messages
        Sleep(5000);
    } catch (const _com_error& e) {
        std::wcout << L"Call failure: " << e.ErrorMessage() << std::endl;
        return 1;
    }

    CoUninitialize();

    return 0;
}

// instantiate ATL
class MyClientModule : public CAtlExeModuleT<MyClientModule> {};
MyClientModule _Module;
