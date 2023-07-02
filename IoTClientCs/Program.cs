using System;
using System.Runtime.InteropServices;
using System.Threading;

namespace IoTClientCs
{
    class Program : IoTServer.Contract.IIoTClient
    {
        public const string IoTServerClass = "AF080472-F173-4D9D-8BE7-435776617347";
        public static readonly Guid IoTServerClassGuid = Guid.Parse(IoTServerClass);

        static void Main(string[] _)
        {
            new Program();

            // wait 5 seconds before exiting to give the server time to send messages
            Thread.Sleep(5000);
        }

        Program ()
        {
            // If the COM server is registered as an in-proc server (as is the case when using
            // a DLL surrogate), activation through the Activator will only use the out-of-proc
            // server if the client and the registered COM server are not the same bitness.
            //
            // Type t = Type.GetTypeFromCLSID(Contract.Constants.IoTAgentClassGuid);
            // var server = (IIoTAgent)Activator.CreateInstance(t);
            //
            // This demo explicitly calls CoCreateInstance with CLSCTX_LOCAL_SERVER to force
            // usage of the out-of-proc server.
            object obj;
            int hr = Ole32.CoCreateInstance(IoTServerClassGuid, IntPtr.Zero, Ole32.CLSCTX_LOCAL_SERVER, typeof(IoTServer.Contract.IIoTAgent).GUID, out obj);
            if (hr < 0) {
                Marshal.ThrowExceptionForHR(hr);
            }

            var server = (IoTServer.Contract.IIoTAgent)obj;
            server.Subscribe(this);

            double pi = server.ComputePi();
            Console.WriteLine($"pi = {pi}");
        }

        public void PushMessage(IoTServer.Contract.Message msg)
        {
            Console.WriteLine("Received message:");
            Console.WriteLine("  sev=" + msg.sev);
            Console.WriteLine("  time=" + msg.time);
            Console.WriteLine("  value=" + msg.value);
            Console.WriteLine("  desc=" + msg.desc);
            Console.WriteLine("  color=(" + msg.color[0] + ", "+ msg.color[1]+", "+ msg.color[2]+")");
            Console.WriteLine("  data=" + msg.data);
        }

        private class Ole32
        {
            // https://docs.microsoft.com/windows/win32/api/wtypesbase/ne-wtypesbase-clsctx
            public const int CLSCTX_LOCAL_SERVER = 0x4;

            // https://docs.microsoft.com/windows/win32/api/combaseapi/nf-combaseapi-cocreateinstance
            [DllImport(nameof(Ole32))]
            public static extern int CoCreateInstance(
                [In, MarshalAs(UnmanagedType.LPStruct)] Guid rclsid,
                IntPtr pUnkOuter,
                uint dwClsContext,
                [In, MarshalAs(UnmanagedType.LPStruct)] Guid riid,
                [MarshalAs(UnmanagedType.IUnknown)] out object ppv);
        }
    }
}
