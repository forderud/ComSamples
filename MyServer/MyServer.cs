using MyInterfaces;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Threading;

namespace MyServer
{
    [ComVisible(true)]
    [Guid("AF080472-F173-4D9D-8BE7-435776617347")] // MyInterfaces.MyServerClass
    [ComDefaultInterface(typeof(MyInterfaces.IMyServer))]
    public sealed class MyServerImpl : IDisposable, MyInterfaces.IMyServer
    {
        private List<IMyClient> m_clients = new List<IMyClient>(); // subscribed clients
        private bool m_active = false;
        private System.Threading.Tasks.Task<object> m_task;

        public MyServerImpl()
        {
            Console.WriteLine("New MyServerImpl instance.");
            m_active = true;

            m_task = ComSupport.ComTask.Run<object>(System.Threading.ApartmentState.MTA, "COM MTA", () => {
                while (m_active)
                {
                    if (m_clients.Count > 0)
                    {
                        // broadcast message to all clients
                        Console.WriteLine("Broadcasting message to all subscribed clients.");
                        Message msg = default;
                        msg.sev = Severity.Info;
                        msg.time =  DateTime.Now;
                        msg.value = 1.23;
                        msg.desc = "Hello there!";
                        msg.color = new byte[3] { 255, 0, 0 };
                        msg.data = new byte[4] { 0, 1, 2, 3 };

                        BroadcastMessage(msg);
                    } else
                    {
                        Console.WriteLine("No clients subscribed.");

                    }

                    Thread.Sleep(2000); // wait 2 seconds
                }
                return null;
            });
        }

        public void Dispose()
        {
            m_active = false;
            m_task.Wait();
        }

        /** Broadcast message to all connected clients. Will disconnect clients on RPC failure. */
        private void BroadcastMessage(Message msg)
        {
            for (int i = 0; i < m_clients.Count;)
            {
                IMyClient client = m_clients[i];
                try
                {
                    // throws InvalidCastException or COMException if client is disconnected
                    client.SendMessage(msg);

                    // advance to next index if call doesn't throw
                    i++;
                }
                catch (InvalidCastException ex)
                {
                    // expect E_NOINTERFACE (0x80004002) when client disconnect
                    Console.WriteLine("Disconnecting client after call failure (err: 0x" + ex.HResult.ToString("X2") + ")");
                    m_clients.RemoveAt(i);
                }
                catch (COMException ex)
                {
                    // expect RPC_S_SERVER_UNAVAILABLE (0x800706BA) when client disconnect
                    Console.WriteLine("Disconnecting client after call failure (err: 0x" + ex.ErrorCode.ToString("X2") + ")");
                    m_clients.RemoveAt(i);
                }
            }
        }

        public MyInterfaces.INumberCruncher GetNumberCruncher()
        {
            Trace.WriteLine($"Running {nameof(MyServer)}.{nameof(GetNumberCruncher)}");
            return new NumberCruncher();
        }

        public void Subscribe(IMyClient client)
        {
            Trace.WriteLine($"Running {nameof(MyServer)}.{nameof(Subscribe)}");
            m_clients.Add(client);
        }

        public void Unsubscribe(IMyClient client)
        {
            Trace.WriteLine($"Running {nameof(MyServer)}.{nameof(Unsubscribe)}");
            m_clients.Remove(client);
        }
    }

    public sealed class NumberCruncher : MyInterfaces.INumberCruncher
    {
        public double ComputePi ()
        {
            return Math.PI;
        }
    }
}
