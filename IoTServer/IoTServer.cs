using System;
using System.Diagnostics;
using System.Runtime.InteropServices;

namespace ComServerExample
{
    [ComVisible(true)]
    [Guid(Contract.Constants.IoTAgentClass)]
    [ComDefaultInterface(typeof(IoTServer.Contract.IIoTAgent))]
    public sealed class IoTServerImpl : IoTServer.Contract.IIoTAgent
    {
        public double ComputePi()
        {
            Trace.WriteLine($"Running {nameof(IoTServer)}.{nameof(ComputePi)}");
            double sum = 0.0;
            int sign = 1;
            for (int i = 0; i < 1024; ++i)
            {
                sum += sign / (2.0 * i + 1.0);
                sign *= -1;
            }

            return 4.0 * sum;
        }
    }
}
