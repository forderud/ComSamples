﻿using MyInterfaces;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Threading;

namespace ComServerExample
{
    [ComVisible(true)]
    [Guid("AF080472-F173-4D9D-8BE7-435776617347")] // MyInterfaces.MyServerClass
    [ComDefaultInterface(typeof(MyInterfaces.IMyServer))]
    public sealed class MyServerImpl : IDisposable, MyInterfaces.IMyServer
    {
        private List<IMyClient> m_clients = new List<IMyClient>();
        private bool m_active = false;

        public MyServerImpl()
        {
            m_active = true;

            _ = ComSupport.ComTask.Run<object>(System.Threading.ApartmentState.MTA, () => {
                while (m_active)
                {
                    if (m_clients.Count > 0)
                    {
                        // broadcast message to all clients
                        Console.WriteLine("Broadcasting message to all clients.");
                        Message msg = default;
                        msg.sev = Severity.Info;
                        msg.time =  DateTime.Now;
                        msg.value = 1.23;
                        msg.desc = "Hello there!";
                        msg.color = new byte[3] { 255, 0, 0 };

                        BroadcastMessage(msg);
                    } else
                    {
                        Console.WriteLine("No connected clients.");

                    }

                    Thread.Sleep(2000); // wait 2 seconds
                }
                return null;
            });
        }

        public void Dispose()
        {
            m_active = false;
        }

        /** Broadcast message to all connected clients. Will disconnect clients on RPC failure. */
        private void BroadcastMessage(Message msg)
        {
            for (int i = 0; i < m_clients.Count;)
            {
                IMyClient client = m_clients[i];
                try
                {
                    // throws COMException if client is disconnected
                    client.PushMessage(msg);

                    // advance to next index if call doesn't throw
                    i++;
                }
                catch (COMException ex)
                {
                    Console.WriteLine("Disconnecting client after call failure (err: " + ex.ErrorCode.ToString("X2") + ")");
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
