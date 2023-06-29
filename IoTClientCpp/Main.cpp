#include <atlbase.h>
#include <iostream>
#include <iomanip>

#include <IoTAgent_h.h>
#include <IoTAgent_i.c>

int main() {
    HRESULT hr  = ::CoInitializeEx(0, COINITBASE_MULTITHREADED);
    if (FAILED(hr)) {
        std::cout << "CoInitializeEx failure: " << std::hex << std::showbase << hr << std::endl;
        return 1;
    }

    CComPtr<IIoTAgent> server;
    hr = ::CoCreateInstance(CLSID_IoTAgent, nullptr, CLSCTX_LOCAL_SERVER, __uuidof(IIoTAgent), (void **)&server);
    if (FAILED(hr)) {
        std::cout << "CoCreateInstance failure: " << std::hex << std::showbase << hr << std::endl;
        return 1;
    }

    double pi = 0;
    hr = server->ComputePi(&pi);
    if (FAILED(hr)) {
        std::cout << "Failure: " << std::hex << std::showbase << hr << std::endl;
        return 1;
    }

    std::cout << "pi = " << std::setprecision(16) << pi << std::endl;

    ::CoUninitialize();

    return 0;
}
