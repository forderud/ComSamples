Based on https://github.com/dotnet/samples/tree/main/core/extensions/OutOfProcCOM

## Build and Run
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
