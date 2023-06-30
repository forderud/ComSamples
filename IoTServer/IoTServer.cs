using IoTServer.Contract;
using System;
using System.Diagnostics;
using System.Runtime.InteropServices;

namespace ComServerExample
{
    class Constants {
        public const string IoTAgentClass = "AF080472-F173-4D9D-8BE7-435776617347";
        public static readonly Guid IoTAgentClassGuid = Guid.Parse(IoTAgentClass);
    }

    [ComVisible(true)]
    [Guid(Constants.IoTAgentClass)]
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
