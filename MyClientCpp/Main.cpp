#include <atlbase.h>
#include <atlcom.h>
#include <iostream>

#include <MyInterfaces.tlh>


class MyClient : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<MyClient>, // no registry entries
    public MyInterfaces::IMyClient {
public:
    MyClient() {
    }

    ~MyClient() {
    }

    HRESULT raw_SendMessage(MyInterfaces::Message msg) override {
        std::wcout << L"Received message:\n";
        std::wcout << L"  sev=" << msg.sev << L"\n";
        std::wcout << L"  time=" << msg.time << L"\n";
        std::wcout << L"  value=" << msg.value << L"\n";
        std::wcout << L"  desc=" << msg.desc << L"\n";
        std::wcout << L"  color=(" << msg.color[0] << L", " << msg.color[1] << L", " << msg.color[2] << L")\n";
        std::wcout << L"  data=" << msg.data << L"\n";;

        return S_OK;
    }

    static CComPtr<MyClient> Create() {
        // create an object (with ref. count zero)
        CComObject<MyClient> * tmp = nullptr;
        CComObject<MyClient>::CreateInstance(&tmp);
        // move into smart-ptr (will incr. ref. count to one)
        return tmp;
    }

    BEGIN_COM_MAP(MyClient)
        COM_INTERFACE_ENTRY(MyInterfaces::IMyClient)
    END_COM_MAP()
};


int main() {
    CoInitializeEx(0, COINITBASE_MULTITHREADED);

    // create MyServer object in a separate process
    MyInterfaces::IMyServerPtr server;
    HRESULT hr = server.CreateInstance(__uuidof(MyInterfaces::MyServer));
    if (FAILED(hr)) {
        std::wcout << L"CoCreateInstance failure: " << hr << std::endl;
        return 1;
    }

    try {
        auto callback = MyClient::Create();
        server->Subscribe(callback);

        auto cruncher = server->GetNumberCruncher();
        double pi = cruncher->ComputePi();
        std::wcout << L"pi = " << pi << std::endl;
    } catch (const _com_error& e) {
        std::wcout << L"Call failure: " << e.ErrorMessage() << std::endl;
        return 1;
    }

    // wait 5 seconds before exiting to give the server time to send messages
    Sleep(5000);

    CoUninitialize();

    return 0;
}

// instantiate ATL
class MyClientModule : public CAtlExeModuleT<MyClientModule> {};
MyClientModule _Module;
