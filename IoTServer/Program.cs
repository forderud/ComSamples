using System;
using System.Diagnostics;
using System.IO;

namespace ComServerExample
{
    class Program
    {
        private static readonly string exePath = Path.Combine(AppContext.BaseDirectory, "IoTServer.exe");

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
                    ComSupport.LocalServer.Register(typeof(IoTAgent.IoTServerClass).GUID, exePath, exePath);
                    return;
                }
                else if (regCommandMaybe.Equals("/unregserver", StringComparison.OrdinalIgnoreCase) || regCommandMaybe.Equals("-unregserver", StringComparison.OrdinalIgnoreCase))
                {
                    // Unregister local server and type library
                    ComSupport.LocalServer.Unregister(typeof(IoTAgent.IoTServerClass).GUID, exePath);
                    return;
                }
            }

            using var server = new ComSupport.LocalServer();
            server.RegisterClass<IoTServerImpl>(typeof(IoTAgent.IoTServerClass).GUID);

            server.Run();
        }
    }
}
