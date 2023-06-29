using System;

namespace Contract
{
    internal sealed class Constants
    {
        public const string ServerClass = "AF080472-F173-4D9D-8BE7-435776617347";
        public static readonly Guid ServerClassGuid = Guid.Parse(ServerClass);

        public const string TypeLibraryName = "Server.Contract.tlb";
    }
}
