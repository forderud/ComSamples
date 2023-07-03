#include <atlbase.h>
#include <iostream>

#include <IoTAgent.tlh>

int main() {
    CoInitializeEx(0, COINITBASE_MULTITHREADED);

    IoTAgent::IMyServerPtr server;
    HRESULT hr = server.CreateInstance(__uuidof(IoTAgent::MyServer), nullptr, CLSCTX_LOCAL_SERVER);
    if (FAILED(hr)) {
        std::wcout << L"CoCreateInstance failure: " << hr << std::endl;
        return 1;
    }

    try {
        double pi = server->ComputePi();
        std::wcout << L"pi = " << pi << std::endl;
    } catch (const _com_error& e) {
        std::wcout << L"Call failure: " << e.ErrorMessage() << std::endl;
        return 1;
    }

    CoUninitialize();

    return 0;
}
