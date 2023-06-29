using System;

namespace Contract
{
    internal sealed class Constants
    {
        public const string IoTAgentClass = "AF080472-F173-4D9D-8BE7-435776617347";
        public static readonly Guid IoTAgentClassGuid = Guid.Parse(IoTAgentClass);

        public const string TypeLibraryName = "IoTServer.Contract.tlb";
    }
}
