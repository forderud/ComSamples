using System;
using System.Threading;

namespace MyClientCs
{
    class Program : MyInterfaces.IMyClient
    {
        static void Main(string[] _)
        {
            new Program();

            // wait 5 seconds before exiting to give the server time to send messages
            Thread.Sleep(5000);
        }

        Program ()
        {
            // same as Activator.CreateInstance(Type.GetTypeFromCLSID(typeof(MyInterfaces.MyServerClass).GUID))
            var server = new MyInterfaces.MyServerClass();

            server.Subscribe(this);

            double pi = server.ComputePi();
            Console.WriteLine($"pi = {pi}");
        }

        public void PushMessage(MyInterfaces.Message msg)
        {
            Console.WriteLine("Received message:");
            Console.WriteLine("  sev=" + msg.sev);
            Console.WriteLine("  time=" + msg.time);
            Console.WriteLine("  value=" + msg.value);
            Console.WriteLine("  desc=" + msg.desc);
            Console.WriteLine("  color=(" + msg.color[0] + ", "+ msg.color[1]+", "+ msg.color[2]+")");
            Console.WriteLine("  data=" + msg.data);
        }
    }
}
