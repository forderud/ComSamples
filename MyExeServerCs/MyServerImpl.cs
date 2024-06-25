using MyInterfaces;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Threading;

namespace MyExeServerCs
{
    /** Client callback handler. Kept in a separate class to avoid introducing
     *  a reference to MyServerImpl when creating a lambda for m_task. */
    class ClientHandler
    {
        private List<IMyClient> m_clients = new List<IMyClient>(); // subscribed clients
        private bool m_active = false;
        private System.Threading.Tasks.Task<object> m_task;

        public ClientHandler()
        {
            m_active = true;

            m_task = ComSupport.ComTask.Run<object>(ApartmentState.MTA, "COM MTA", () => {
                while (m_active)
                {
                    if (m_clients.Count > 0)
                    {
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

        /** Broadcast message to all connected clients. Will disconnect clients on RPC failure. */
        private void BroadcastMessage(Message msg)
        {
            for (int i = 0; i < m_clients.Count;)
            {
                IMyClient client = m_clients[i];
                try
                {
                    // throws InvalidCastException or COMException if client is disconnected
                    client.XmitMessage(msg);

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

        public void Close ()
        {
            m_active = false;
            m_task.Wait();
        }

        public List<IMyClient> Clients () {
            return m_clients;
        }
    }

    /** Creatable COM class that needs a CLSID. */
    [ComVisible(true)]
    [Guid("AF080472-F173-4D9D-8BE7-435776617347")] // MyInterfaces.MyExeServerCsClass
    [ComDefaultInterface(typeof(MyInterfaces.IMyServer))]
    public sealed class MyServerImpl : ComSupport.ComClass, MyInterfaces.IMyServer
    {
        ClientHandler m_clients = null;

        public MyServerImpl()
        {
            Console.WriteLine("MyServerImpl ctor.");
            m_clients = new ClientHandler();
        }

        ~MyServerImpl()
        {
            Console.WriteLine("MyServerImpl dtor.");
            m_clients.Close();
        }

        public MyInterfaces.INumberCruncher GetNumberCruncher()
        {
            Trace.WriteLine($"Running {nameof(MyExeServerCs)}.{nameof(GetNumberCruncher)}");
            return new NumberCruncher();
        }

        public void Subscribe(IMyClient client)
        {
            Trace.WriteLine($"Running {nameof(MyExeServerCs)}.{nameof(Subscribe)}");
            m_clients.Clients().Add(client);
        }

        public void Unsubscribe(IMyClient client)
        {
            Trace.WriteLine($"Running {nameof(MyExeServerCs)}.{nameof(Unsubscribe)}");
            m_clients.Clients().Remove(client);
        }
    }

    /** Non-creatable COM class that doesn't need any CLSID. */
    public sealed class NumberCruncher : ComSupport.ComClass, MyInterfaces.INumberCruncher
    {
        public NumberCruncher()
        {
            Console.WriteLine("NumberCruncher ctor.");
        }

        ~NumberCruncher()
        {
            Console.WriteLine("NumberCruncher dtor.");
        }

        public double ComputePi ()
        {
            return Math.PI;
        }
    }
}
