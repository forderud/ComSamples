#include <atlbase.h>
#include <iostream>

#include <IoTAgent_h.h>
#include <IoTAgent_i.c>

int main() {
    CoInitializeEx(0, COINITBASE_MULTITHREADED);

    CComPtr<IIoTAgent> server;
    HRESULT hr = ::CoCreateInstance(CLSID_IoTAgent, nullptr, CLSCTX_LOCAL_SERVER, __uuidof(IIoTAgent), (void **)&server);
    if (FAILED(hr)) {
        std::cout << "CoCreateInstance failure: " << hr << std::endl;
        return 1;
    }

    double pi = 0;
    hr = server->ComputePi(&pi);
    if (FAILED(hr)) {
        std::cout << "Failure: " << hr << std::endl;
        return 1;
    }

    std::cout << "pi = " << pi << std::endl;

    CoUninitialize();

    return 0;
}
