using System;
using System.Runtime.InteropServices;
using System.Threading;

namespace MyClientCs
{
    class Program
    {
        static void Test()
        {
            // create or connect to server object in a separate process
            // equivalent to Activator.CreateInstance(Type.GetTypeFromCLSID(typeof(MyInterfaces.MyServerClass).GUID))
            var server = new MyInterfaces.MyServerClass();

            {
                var cruncher = server.GetNumberCruncher();
                double pi = cruncher.ComputePi();
                Console.WriteLine($"pi = {pi}");

                // release reference to help GC clean up
                cruncher = null;
            }
            {
                var callback = new ClientCallback();
                server.Subscribe(callback);

                // wait 5 seconds before exiting to give the server time to send messages
                Thread.Sleep(5000);

                server.Unsubscribe(callback);
                // release reference to help GC clean up
                callback = null;
            }

            // release reference to help GC clean up
            server = null;
        }

        [MTAThread] // or [STAThread]
        static void Main(string[] _)
        {
            Test();

            // trigger GC to release references seen by server
            System.GC.Collect();
            System.GC.WaitForPendingFinalizers();
        }
    }

    /** Non-creatable COM class that doesn't need any CLSID. */
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
