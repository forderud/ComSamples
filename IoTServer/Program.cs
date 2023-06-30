using System;
using System.Diagnostics;
using System.IO;

namespace ComServerExample
{
    class Program
    {
        private static readonly string exePath = Path.Combine(AppContext.BaseDirectory, "IoTServer.exe");

#if EMBEDDED_TYPE_LIBRARY
        private static readonly string tlbPath = exePath;
#else
        private static readonly string tlbPath = Path.Combine(AppContext.BaseDirectory, "IoTServer.Contract.tlb");
#endif

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
                    ComSupport.LocalServer.Register(Constants.IoTAgentClassGuid, exePath, tlbPath);
                    return;
                }
                else if (regCommandMaybe.Equals("/unregserver", StringComparison.OrdinalIgnoreCase) || regCommandMaybe.Equals("-unregserver", StringComparison.OrdinalIgnoreCase))
                {
                    // Unregister local server and type library
                    ComSupport.LocalServer.Unregister(Constants.IoTAgentClassGuid, tlbPath);
                    return;
                }
            }

            using var server = new ComSupport.LocalServer();
            server.RegisterClass<IoTServerImpl>(Constants.IoTAgentClassGuid);

            server.Run();
        }
    }
}
