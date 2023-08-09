using ComSupport;
using System;
using System.Diagnostics;
using System.IO;
using System.Threading;

namespace MyServerCs
{
    class Program
    {
        private static readonly string exePath = Path.Combine(AppContext.BaseDirectory, "MyServerCs.exe");

        static void Main(string[] args)
        {
            using var consoleTrace = new ConsoleTraceListener();
            Trace.Listeners.Add(consoleTrace);

            if (args.Length == 1)
            {
                string regCommandMaybe = args[0];
                if (regCommandMaybe.Equals("/regserver", StringComparison.OrdinalIgnoreCase) ||
                    regCommandMaybe.Equals("-regserver", StringComparison.OrdinalIgnoreCase))
                {
                    // Register server and type library
                    TypeLib.Register(exePath);
                    LocalServer.Register(typeof(MyInterfaces.MyServerClass).GUID, exePath);
                    AppID.Register(typeof(MyInterfaces.MyServerClass).GUID, "Interactive User");
                    return;
                }
                else if (regCommandMaybe.Equals("/unregserver", StringComparison.OrdinalIgnoreCase) ||
                    regCommandMaybe.Equals("-unregserver", StringComparison.OrdinalIgnoreCase))
                {
                    // Unregister server and type library
                    AppID.Unregister(typeof(MyInterfaces.MyServerClass).GUID);
                    LocalServer.Unregister(typeof(MyInterfaces.MyServerClass).GUID);
                    TypeLib.Unregister(exePath);
                    return;
                }
                else if (regCommandMaybe.Equals("/Embedding", StringComparison.OrdinalIgnoreCase) ||
                    regCommandMaybe.Equals("-Embedding", StringComparison.OrdinalIgnoreCase))
                {
                    // auto-started by COM runtime
                    using var server = new LocalServer();
                    server.RegisterClass<MyServerImpl>(typeof(MyInterfaces.MyServerClass).GUID);
                    // terminate process after last client disconnect
                    server.WaitForRefCountsToReachZero();
                    return;
                }
            }
            else if(args.Length == 0)
            {
                // process manually started
                using var server = new LocalServer();
                server.RegisterClass<MyServerImpl>(typeof(MyInterfaces.MyServerClass).GUID);
                // run until terminated
                Thread.Sleep(Timeout.Infinite);
            }
        }
    }
}
