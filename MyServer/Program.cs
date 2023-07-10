using ComSupport;
using System;
using System.Diagnostics;
using System.IO;

namespace MyServer
{
    class Program
    {
        private static readonly string exePath = Path.Combine(AppContext.BaseDirectory, "MyServer.exe");

        static void Main(string[] args)
        {
            using var consoleTrace = new ConsoleTraceListener();
            Trace.Listeners.Add(consoleTrace);

            if (args.Length == 1)
            {
                string regCommandMaybe = args[0];
                if (regCommandMaybe.Equals("/regserver", StringComparison.OrdinalIgnoreCase) || regCommandMaybe.Equals("-regserver", StringComparison.OrdinalIgnoreCase))
                {
                    // Register local server and type library
                    LocalServer.Register(typeof(MyInterfaces.MyServerClass).GUID, exePath);
                    TypeLib.Register(exePath);
                    return;
                }
                else if (regCommandMaybe.Equals("/unregserver", StringComparison.OrdinalIgnoreCase) || regCommandMaybe.Equals("-unregserver", StringComparison.OrdinalIgnoreCase))
                {
                    // Unregister local server and type library
                    LocalServer.Unregister(typeof(MyInterfaces.MyServerClass).GUID);
                    TypeLib.Unregister(exePath);
                    return;
                }
            }

            using var server = new LocalServer();
            server.RegisterClass<MyServerImpl>(typeof(MyInterfaces.MyServerClass).GUID);

            server.Run();
        }
    }
}
