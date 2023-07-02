using System;
using System.Threading;

namespace IoTClientCs
{
    class Program : IoTAgent.IIoTClient
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
            Type t = Type.GetTypeFromCLSID(IoTServerClassGuid);
            var server = (IoTAgent.IIoTAgent)Activator.CreateInstance(t);

            server.Subscribe(this);

            double pi = server.ComputePi();
            Console.WriteLine($"pi = {pi}");
        }

        public void PushMessage(IoTAgent.Message msg)
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
