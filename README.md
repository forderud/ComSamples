# COM Server Example
Based on https://github.com/dotnet/samples/tree/main/core/extensions/OutOfProcCOM

## Prerequisites
The sample is currently focused on Windows.
* [Visual Studio](https://visualstudio.microsoft.com/)


## Build and Run
1. Open the solution in [Visual Studio](https://visualstudio.microsoft.com/).
1. Build all projects.
1. Follow the instructions for registering the server.
1. Run the client:
    * C++: run `IoTClientCpp.exe`
    * C#: run `IoTClientCs.exe`
1. Follow the instructions for **un**registering the server.

The client program should output the value: `pi = 3.140616091322624`

### Embedded Type Library

The [.NET 6.0 SDK](https://dotnet.microsoft.com/download) (Preview 5 or later) supports [embedding type libraries into the COM DLL](https://docs.microsoft.com/dotnet/core/native-interop/expose-components-to-com#embedding-type-libraries-in-the-com-host). To use this functionality for the DLL surrogate (`DllServer`) in this sample, build with `-p:EmbedTypeLibrary=true`.

**Note:** Remember to unregister the COM server when the demo is complete.
