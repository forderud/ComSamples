# COM Server Example
Based on https://github.com/dotnet/samples/tree/main/core/extensions/OutOfProcCOM

## Prerequisites
The sample is currently focused on Windows.
* [Visual Studio](https://visualstudio.microsoft.com/)


## Build and Run
1. Open a [Developer Command Prompt for Visual Studio](https://docs.microsoft.com/cpp/build/building-on-the-command-line#developer_command_prompt_shortcuts).
1. Navigate to the root directory and build the solution:
    * `msbuild ComSeverExample.sln -restore`.
1. Show the instructions for COM server registration:
    * Executable server: `dotnet msbuild -target:ServerUsage ExeServer`
1. Follow the instructions for registering the server.
1. Run the client:
    * Native: run the `NativeClient.exe` binary
        * Example: `x64\Debug\NativeClient.exe`
    * Managed: run the `ManagedClient.exe` binary
        * Example: `ManagedClient\bin\Debug\netcoreapp3.1\ManagedClient.exe`

The client program should output an estimated value of &#960;:
```
π = 3.140616091322624
```

### Embedded Type Library

The [.NET 6.0 SDK](https://dotnet.microsoft.com/download) (Preview 5 or later) supports [embedding type libraries into the COM DLL](https://docs.microsoft.com/dotnet/core/native-interop/expose-components-to-com#embedding-type-libraries-in-the-com-host). To use this functionality for the DLL surrogate (`DllServer`) in this sample, build with `-p:EmbedTypeLibrary=true`.

**Note:** Remember to unregister the COM server when the demo is complete.
