using IoTServer.Contract;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Threading;

namespace ComServerExample
{
    class Constants {
        public const string IoTAgentClass = "AF080472-F173-4D9D-8BE7-435776617347";
        public static readonly Guid IoTAgentClassGuid = Guid.Parse(IoTAgentClass);
    }

    [ComVisible(true)]
    [Guid(Constants.IoTAgentClass)]
    [ComDefaultInterface(typeof(IoTServer.Contract.IIoTAgent))]
    public sealed class IoTServerImpl : IDisposable, IoTServer.Contract.IIoTAgent
    {
        private List<IIoTClient> m_clients = new List<IIoTClient>();
        private bool m_active = false;

        public IoTServerImpl()
        {
            m_active = true;

            _ = ComSupport.ComTask.Run<object>(System.Threading.ApartmentState.MTA, () => {
                while (m_active)
                {
                    // broadcast message to all clients
                    Console.WriteLine("Broadcasting message to all clients.");
                    Message msg = default;
                    msg.sev = Severity.SeverityInfo;
                    msg.time = 1.23;
                    msg.desc = "Hello there!";
                    msg.color = new byte[3] { 255, 0, 0 };

                    for (int i = 0; i < m_clients.Count;) {
                        IIoTClient client = m_clients[i];
                        try {
                            // throws COMException if client is disconnected
                            client.PushMessage(msg);

                            // advance to next index if call doesn't throw
                            i++;
                        } catch (COMException ex) {
                            Console.WriteLine("Disconnecting client after call failure (err: " + ex.ErrorCode.ToString("X2") + ")");
                            m_clients.RemoveAt(i);
                        }
                    }

                    Thread.Sleep(2000); // 2 seconds
                }
                return null;
            });
        }

        public void Dispose()
        {
            m_active = false;
        }

        public double ComputePi()
        {
            Trace.WriteLine($"Running {nameof(IoTServer)}.{nameof(ComputePi)}");
            return Math.PI;
        }

        public void Subscribe(IIoTClient client)
        {
            Trace.WriteLine($"Running {nameof(IoTServer)}.{nameof(Subscribe)}");
            m_clients.Add(client);
        }

        public void Unsubscribe(IIoTClient client)
        {
            Trace.WriteLine($"Running {nameof(IoTServer)}.{nameof(Unsubscribe)}");
            m_clients.Remove(client);
        }
    }
}
