using System;
using System.Diagnostics;
using Microsoft.Win32;

namespace ComSupport
{
    public static class AppID
    {
        public static void Register(Guid clsid, string runAsUser)
        {
            Trace.WriteLine($"Registering server with system-supplied DLL surrogate:");
            Trace.Indent();
            Trace.WriteLine($"CLSID: {clsid:B}");
            Trace.Unindent();

            string serverKey = string.Format(KeyFormat.CLSID, clsid);

            // Register App ID - use the CLSID as the App ID
            using RegistryKey regKey = Registry.LocalMachine.CreateSubKey(serverKey);
            regKey.SetValue("AppID", clsid.ToString("B"));
            string appIdKey = string.Format(KeyFormat.AppID, clsid);
            using RegistryKey appIdRegKey = Registry.LocalMachine.CreateSubKey(appIdKey);

            // Register RunAs to allow an non-elevated client to connect to an already running elevated server
            if (runAsUser.Length > 0)
                appIdRegKey.SetValue("RunAs", runAsUser);
        }

        public static void Unregister(Guid clsid)
        {
            Trace.WriteLine($"Unregistering server:");
            Trace.Indent();
            Trace.WriteLine($"CLSID: {clsid:B}");
            Trace.Unindent();

            // Remove the App ID value
            string serverKey = string.Format(KeyFormat.CLSID, clsid);
            using RegistryKey regKey = Registry.LocalMachine.OpenSubKey(serverKey, writable: true);
            if (regKey != null)
                regKey.DeleteValue("AppID");

            // Remove the App ID key
            string appIdKey = string.Format(KeyFormat.AppID, clsid);
            Registry.LocalMachine.DeleteSubKey(appIdKey, throwOnMissingSubKey: false);
        }
    }
}
