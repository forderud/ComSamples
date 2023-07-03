Repo content:

| Module       | Description                                 |
|--------------|---------------------------------------------|
| ComSupport   | .Net support functions for COM registration |
| MyInterfaces | Language-neutral COM interface definitions for IPC |
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

The client program should output the value: `pi = 3.140616091322624`
