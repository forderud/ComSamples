using System;
using System.Threading;

namespace MyClientCs
{
    class Program : IDisposable, MyInterfaces.IMyClient
    {
        MyInterfaces.MyServerClass m_server = null;

        static void Main(string[] _)
        {
            var p = new Program();

            var cruncher = p.m_server.GetNumberCruncher();
            double pi = cruncher.ComputePi();
            Console.WriteLine($"pi = {pi}");

            // wait 5 seconds before exiting to give the server time to send messages
            Thread.Sleep(5000);
        }

        Program ()
        {
            // same as Activator.CreateInstance(Type.GetTypeFromCLSID(typeof(MyInterfaces.MyServerClass).GUID))
            m_server = new MyInterfaces.MyServerClass();

            m_server.Subscribe(this);
        }

        public void Dispose()
        {
            m_server = null;
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
