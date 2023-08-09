using System;
using System.Runtime.InteropServices;
using System.Threading;

namespace MyClientCs
{
    class Program
    {
        static void Main(string[] _)
        {
            // create server object in a separate process
            // equivalent to Activator.CreateInstance(Type.GetTypeFromCLSID(typeof(MyInterfaces.MyServerClass).GUID))
            var server = new MyInterfaces.MyServerClass();

            var callback = new ClientCallback();
            server.Subscribe(callback);

            {
                var cruncher = server.GetNumberCruncher();
                double pi = cruncher.ComputePi();
                Console.WriteLine($"pi = {pi}");

                // release server reference
                Marshal.ReleaseComObject(cruncher);
            }

            // wait 5 seconds before exiting to give the server time to send messages
            Thread.Sleep(5000);

            // release server reference
            Marshal.ReleaseComObject(server);
        }
    }

    class ClientCallback : MyInterfaces.IMyClient
    {
        public void SendMessage(ref MyInterfaces.Message msg)
        {
            Console.WriteLine("Received message:");
            Console.WriteLine("  sev=" + msg.sev);
            Console.WriteLine("  time=" + msg.time);
            Console.WriteLine("  value=" + msg.value);
            Console.WriteLine("  desc=" + msg.desc);
            Console.WriteLine("  color=(" + string.Join(", ", msg.color)+")");
            Console.WriteLine("  data=[" + string.Join(",", msg.data)+"]");
        }
    }
}
