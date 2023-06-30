Based on https://github.com/dotnet/samples/tree/main/core/extensions/OutOfProcCOM

## Prerequisites
The sample is currently focused on Windows.
* [Visual Studio](https://visualstudio.microsoft.com/)


## Build and Run
1. Open the solution in [Visual Studio](https://visualstudio.microsoft.com/).
1. Build all projects.
1. Follow the instructions for registering the server.
    * Run `<path>\IoTServer.exe" /regserver`  with admin privileges.
1. Run the client:
    * C++: run `IoTClientCpp.exe`
    * C#: run `IoTClientCs.exe`
1. Follow the instructions for **un**registering the server.
    * Run `<path>\IoTServer.exe" /unregserver`  or `UNREGISTER.bat` with admin privileges.

The client program should output the value: `pi = 3.140616091322624`
