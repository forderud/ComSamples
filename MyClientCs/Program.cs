using System;
using System.Threading;

namespace MyClientCs
{
    class Program : MyInterfaces.IMyClient
    {
        static void Main(string[] _)
        {
            // create MyServer object in a separate process
            // equivalent to Activator.CreateInstance(Type.GetTypeFromCLSID(typeof(MyInterfaces.MyServerClass).GUID))
            var server = new MyInterfaces.MyServerClass();

            var callback = new Program();
            server.Subscribe(callback);

            var cruncher = server.GetNumberCruncher();
            double pi = cruncher.ComputePi();
            Console.WriteLine($"pi = {pi}");

            // wait 5 seconds before exiting to give the server time to send messages
            Thread.Sleep(5000);
        }

        public void SendMessage(MyInterfaces.Message msg)
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
