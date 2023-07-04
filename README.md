Examples of language-neutral [Component Object Model (COM](https://learn.microsoft.com/en-us/windows/win32/com/the-component-object-model) interfaces for IPC.

Repo content:

| Module       | Description                                 |
|--------------|---------------------------------------------|
| ComSupport   | .Net support functions for COM registration |
| MyInterfaces | Language-neutral COM interface definitions for IPC ([MyInterfaces.idl](MyInterfaces/MyInterfaces.idl)) |
| MyClientCpp  | Sample C++ client |
| MyClientCs   | Sample C# client |
| MyServer     | Server implementation. Implemented as a on-demand loaded COM EXE server. |
|              | Can easily be converted into a background service. The console window can also be hidden by either changing the _subsystem_ from `Console` to `Windows`or by moving the process to session 0.|

Based on https://github.com/dotnet/samples/tree/main/core/extensions/OutOfProcCOM


## Motivation
Some benefits of using COM:
* **Language neutral** interfaces.
* COM is **in-built in Windows**. There's no need to install any additional "runtime" or "brokers".
* Suport for **type-safe** interfaces where type mismatches are detected at compile-time instead of run-time.
* Support for bidirectional **object-oriented** communication.
* **Transparent IPC marshaling** of method calls. The client doesn't need to know that an object resides in a different process.
* Languages like C++, C# and Python natively support COM, which avoids the need for any manually maintained language bindings.
* **Rock solid** technology that have been available since 1993.

Limitations:
* Not as easily available on non-Windows platforms.


## How to test
1. Open the solution in [Visual Studio](https://visualstudio.microsoft.com/).
1. Build all projects.
1. Register the server:
    * Run `<path>\MyServer.exe" /regserver`  with admin privileges.
1. Run the client:
    * C++: run `MyClientCpp.exe`
    * C#: run `MyClientCs.exe`
1. **Un**register the server to clean up:
    * Run `<path>\MyServer.exe" /unregserver`  or `UNREGISTER.bat` with admin privileges.

Server registration is only needed for on-demand loaded COM EXE servers, and can be skipped if instead running the server in a background service that is auto-started. In that case, the TypeLib will still need to be registred at server startup.

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
