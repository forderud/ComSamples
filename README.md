Repo content:

| Module       | Description                                 |
|--------------|---------------------------------------------|
| ComSupport   | .Net support functions for COM registration |
| MyInterfaces | Language-neutral COM interface definitions for IPC: [MyInterfaces.idl](MyInterfaces/MyInterfaces.idl) |
| MyClientCpp  | Sample C++ client |
| MyClientCs   | Sample C# client |
| MyServer     | Server implementation. Implemented as a on-demand loaded COM EXE server, but can easily be converted into a background service. |

Based on https://github.com/dotnet/samples/tree/main/core/extensions/OutOfProcCOM

## How to test
1. Open the solution in [Visual Studio](https://visualstudio.microsoft.com/).
1. Build all projects.
1. Follow the instructions for registering the server.
    * Run `<path>\MyServer.exe" /regserver`  with admin privileges.
1. Run the client:
    * C++: run `MyClientCpp.exe`
    * C#: run `MyClientCs.exe`
1. Follow the instructions for **un**registering the server.
    * Run `<path>\MyServer.exe" /unregserver`  or `UNREGISTER.bat` with admin privileges.

The client programs should output something resembling this:
```
pi = 3.141592653589793
Received message:
  sev=Info
  time=7/4/2023 10:27:57 AM
  value=1.23
  desc=Hello there!
  color=(255, 0, 0)
  data=
...
```
