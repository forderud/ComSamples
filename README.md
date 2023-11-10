Examples of language-neutral [Component Object Model (COM)](https://learn.microsoft.com/en-us/windows/win32/com/the-component-object-model) interfaces for [IPC](https://en.wikipedia.org/wiki/Inter-process_communication) and interoperability between programming languages.

Project listing:

| Module       | Description                                 |
|--------------|---------------------------------------------|
| ComSupport   | .Net support functions for COM registration |
| MyInterfaces | COM interface definitions ([MyInterfaces.idl](MyInterfaces/MyInterfaces.idl)) |
| MyClientCpp  | Sample C++ _client_ |
| MyClientCs   | Sample C# _client_ |
| MyClientPy   | Sample Python _client_ |
| MyServerCpp  | C++ _server_ implementation |
| MyServerCs   | C# _server_ implementation |

Both servers are implemented as on-demand loaded COM EXE servers. The processes can also be started manually to facilitate background service deployment.

The .Net samples are based on https://github.com/dotnet/samples/tree/main/core/extensions/OutOfProcCOM


## COM description

Interface definition and C++/C#/Python integration workflow:  
![Interface Integration](InterfaceIntegration.svg)

#### COM strengths:
* **Language neutral** interface definitions.
* COM is **in-built in Windows**. There's no need to install any additional "runtime" or "brokers". COM is used for GPU programming with Direct3D, HW diagnostics with WMI and most new Windows features are exposed through COM-based [WinRT](https://github.com/microsoft/xlang) interfaces.
* Support for **type-safe** interfaces where type mismatches are detected at compile-time instead of run-time.
* Support for bidirectional **object-oriented** communication.
* Support for **interface versioning** for controlled API evolution.
* **Transparent IPC marshaling** of method calls. The client doesn't need to know that an object resides in a different process.
* Languages like C++, C# and Python natively support COM, which **avoids** the need for any manually maintained **language bindings**.
* **Rock solid** technology that has been available since 1993.
* Inherently **secure** with design protection against remote attacks. Does unlike sockets _not_ depend on opening any ports that need to be secured through authentication and firewall lockdown. Security settings are furthermore highly configurable.

#### COM limitations:
* Best practice is poorly documented.
* Not as easily available on non-Windows platforms. The [MiniCOM](https://github.com/forderud/MiniCOM) project can partly mitigate this for in-process needs.

### COM server types
COM servers can be compiled as either:
* **DLL**: Server that runs in the _same process_ as the client (in-process)[1]. This allows for more efficient direct communication without IPC. However, it does not allow for failure isolation or security sandboxing.
* **EXE**: Server that runs in a _separate process_ (out-of-process). This leads to some per-call marshalling overhead due to IPC. However, you also get failure isolation so that a crashing server doesn't bring the client process down. The server can also run with more or fewer security privileges compared to the client.

COM provides transparent marshalling. This means that the client code doesn't need to know if the COM server runs in the same or a different process. The object that exposes COM interfaces looks the same, regardless of if it's running in the same or a separate process or is implemented in a different programming language.

This repo focuses on IPC and does therefore only contain EXE-based servers, since there are fewer online resources on that. Microsoft provides [COMServerDemo](https://github.com/dotnet/samples/tree/main/core/extensions/COMServerDemo) sample code for DLL-based servers implemented in C#/.Net and [How to: Create a Classic COM Component Using WRL](https://learn.microsoft.com/en-us/cpp/cppcx/wrl/how-to-create-a-classic-com-component-using-wrl) and [Component Object Model (COM) Sample](https://github.com/microsoft/component-object-model-sample) for servers implemented in C++.

[1] DLL-based COM servers can also be started in a separate process if configuring AppID [DllSurrogate](https://learn.microsoft.com/en-us/windows/win32/com/dllsurrogate).

### Exception mapping
Both C++ and .Net can automatically map COM `HRESULT` error codes to exceptions, so that developers doesn't need to explicitly check each call for failure.

#### Details:
* **.Net**: `HRESULT` error codes are automatically [mapped to comparable .Net exceptions](https://learn.microsoft.com/en-us/dotnet/framework/interop/how-to-map-hresults-and-exceptions).
* **C++** with generated TLH-wrappers: `HRESULT` error codes are automatically mapped to [`_com_error`](https://learn.microsoft.com/en-us/cpp/cpp/com-error-class) exceptions. It's still possible to call the "old" HRESULT versions by adding a `raw_` prefix to the method names.

### Threading
COM clients and servers can decide their [threading model](https://learn.microsoft.com/en-us/troubleshoot/windows/win32/descriptions-workings-ole-threading-models) for _incoming_ calls[1] by configuring the thread associated with the class(es) receiving callbacks to run in either:
* **Single-threaded apartment (STA)**[2]: Incoming calls are automatically serialized. This means that the client doesn't need to worry about thread safety, since the COM runtime is ensuring that only one incoming call is received at a time.
* **Multi-threaded apartment (MTA)**: Incoming calls are _not_ serialized and might arrive concurrently. This means that the client need to use mutexes or similar to protect against race conditions.

[1] The threading model only affect _incoming_ calls marshalled by the COM runtime. This typically means COM servers implemented in a different programming language or running in a different process. Direct C++ communication between COM objects in the same process are not affected by the threading model.

[2] STA threads need to [pump messages](https://learn.microsoft.com/en-us/windows/win32/winmsg/using-messages-and-message-queues) to process incoming calls - just like all GUI applications does to process mouse & keyboard events. The implementation then needs to consider that _reentrancy can occur_ as part of the message pumping _if_ pumping messages while processing an incoming call.

### Security
Most security settings for a COM server can be configured through [AppID](https://learn.microsoft.com/en-us/windows/win32/com/appid-key) registry entries. The [COM Elevation Moniker](https://learn.microsoft.com/en-us/windows/win32/com/the-com-elevation-moniker) can furthermore be used to request startup of a COM server in an elevated process. See the [RunInSandbox](https://github.com/forderud/RunInSandbox) project for how to configure security sandboxing and elevation in conjunction with COM.

## How to test
1. Ensure that you have a [Python](https://www.python.org/) interpreter associated with `.py` files.
1. Open the solution in [Visual Studio](https://visualstudio.microsoft.com/).
1. Build all projects.
1. Register the server:
    * Either run `MyServerCs.exe" /regserver`  with admin privileges,
    * Or run `MyServerCpp.exe" /regserver`  with admin privileges.
1. Run the test clients:
    * C++: run `MyClientCpp.exe`
    * C#: run `MyClientCs.exe`
    * Python: run `MyClientPy.py`
1. **Un**register the server to clean up:
    * Either run `MyServerCs.exe" /unregserver` with admin privileges,
    * Or run `MyServerCpp.exe" /unregserver`  with admin privileges,
    * Or run `UNREGISTER.bat` with admin privileges.

Server registration is only needed for on-demand loaded COM EXE servers and can be skipped if instead running the server in a background service that is auto-started. In that case, the TypeLib will still need to be registered at server startup.

The client programs should output something resembling this:
```
pi = 3.141592653589793
Received message:
  sev=Info
  time=7/4/2023 10:27:57 AM
  value=1.23
  desc=Hello there!
  color=(255, 0, 0)
  data=[0,1,2,3]
...
```
