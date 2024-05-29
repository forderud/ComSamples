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

        [MTAThread] // or [STAThread]
        static void Main(string[] args)
        {
            // allow lower privilege clients to connect
            int hr = ComSecurity.CoInitializeSecurity(IntPtr.Zero, -1, IntPtr.Zero, IntPtr.Zero, RpcAuthnLevel.Default, RpcImpLevel.Identify, IntPtr.Zero, EoAuthnCap.None, IntPtr.Zero);
            if (hr != 0) // S_OK check
                throw new Exception("CoInitializeSecurity failed");

            using var consoleTrace = new ConsoleTraceListener();
            Trace.Listeners.Add(consoleTrace);
            var clsid = typeof(MyInterfaces.MyServerClass).GUID; // COM class ID (also used as AppID)

            if (args.Length == 1)
            {
                string regCommandMaybe = args[0];
                if (regCommandMaybe.Equals("/regserver", StringComparison.OrdinalIgnoreCase) ||
                    regCommandMaybe.Equals("-regserver", StringComparison.OrdinalIgnoreCase))
                {
                    // Register server and type library
                    Guid typeLib = TypeLib.Register(exePath);
                    ExeServer.Register(clsid, exePath, typeLib, "MyServerCs Object");
                    AppID.Register(clsid, clsid, "MyServerCs Object");
                    return;
                }
                else if (regCommandMaybe.Equals("/unregserver", StringComparison.OrdinalIgnoreCase) ||
                    regCommandMaybe.Equals("-unregserver", StringComparison.OrdinalIgnoreCase))
                {
                    // Unregister server and type library
                    AppID.Unregister(clsid, clsid);
                    ExeServer.Unregister(clsid);
                    TypeLib.Unregister(exePath);
                    return;
                }
                else if (regCommandMaybe.Equals("/Embedding", StringComparison.OrdinalIgnoreCase) ||
                    regCommandMaybe.Equals("-Embedding", StringComparison.OrdinalIgnoreCase))
                {
                    // auto-started by COM runtime
                    using var server = new RunningServer();
                    server.RegisterClass<MyServerImpl>(clsid);
                    // terminate process after last client disconnect
                    server.WaitForRefCountsToReachZero();
                    return;
                }
            }
            else if(args.Length == 0)
            {
                // process manually started
                using var server = new RunningServer();
                server.RegisterClass<MyServerImpl>(clsid);
                // run until terminated
                Thread.Sleep(Timeout.Infinite);
            }
        }
    }
}
