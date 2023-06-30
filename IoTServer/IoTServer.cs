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
            return Math.PI;
        }
    }
}
