using System;
using System.Diagnostics;
using Microsoft.Win32;

namespace ComSupport
{
    public static class AppID
    {
        /** Uses the CLSID also as AppID for convenience (same as on https://github.com/dotnet/samples/blob/main/core/extensions/OutOfProcCOM/COMRegistration/DllSurrogate.cs) */
        public static void Register(Guid clsid)
        {
            Trace.WriteLine($"Registering server with system-supplied DLL surrogate:");
            Trace.Indent();
            Trace.WriteLine($"CLSID: {clsid:B}");
            Trace.Unindent();

            // write HKCR\CLSID\{clsid}\AppID = {appid}
            using RegistryKey clsidKey = Registry.LocalMachine.CreateSubKey(string.Format(KeyFormat.CLSID, clsid));
            clsidKey.SetValue("AppID", clsid.ToString("B"));

            // write HKCR\AppID\{clsid}
            // done to create placeholder for later RunAs values
            using RegistryKey appIdKey = Registry.LocalMachine.CreateSubKey(string.Format(KeyFormat.AppID, clsid));
            appIdKey.SetValue(null, "MyServerCs Object");
        }

        public static void Unregister(Guid clsid)
        {
            Trace.WriteLine($"Unregistering server:");
            Trace.Indent();
            Trace.WriteLine($"CLSID: {clsid:B}");
            Trace.Unindent();

            // delete HKCR\CLSID\{clsid}\AppID
            using RegistryKey clsidKey = Registry.LocalMachine.OpenSubKey(string.Format(KeyFormat.CLSID, clsid), writable: true);
            if (clsidKey != null)
                clsidKey.DeleteValue("AppID");

            // delete HKCR\AppID\{clsid}
            Registry.LocalMachine.DeleteSubKey(string.Format(KeyFormat.AppID, clsid), throwOnMissingSubKey: false);
        }
    }
}
