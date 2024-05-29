using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Threading;
using Microsoft.Win32;

namespace ComSupport
{
    internal static class KeyFormat
    {
        public const string CLSID = @"SOFTWARE\Classes\CLSID\{0:B}";
        public const string AppID = @"SOFTWARE\Classes\AppID\{0:B}";
    }

    public sealed class LocalServer : IDisposable
    {
        public static bool m_active = false; // first object have been created

        public static void Register(Guid clsid, string exePath, Guid typeLib, string description)
        {
            Trace.WriteLine($"Registering server:");
            Trace.Indent();
            Trace.WriteLine($"CLSID: {clsid:B}");
            Trace.WriteLine($"Executable: {exePath}");
            Trace.Unindent();

            // write HKCR\CLSID\{clsid} = {desc}
            string clsidPath = string.Format(@$"{KeyFormat.CLSID}", clsid);
            using RegistryKey clsidKey = Registry.LocalMachine.CreateSubKey(clsidPath);
            clsidKey.SetValue(null, description);

            // write HKCR\CLSID\{clsid}\LocalServer32 = {exePath}
            using RegistryKey lsKey = Registry.LocalMachine.CreateSubKey(clsidPath + "\\LocalServer32");
            lsKey.SetValue(null, exePath);

            // write HKCR\CLSID\{clsid}\TypeLib = {typeLib}
            using RegistryKey tlbKey = Registry.LocalMachine.CreateSubKey(clsidPath + "\\TypeLib");
            tlbKey.SetValue(null, "{"+typeLib+"}");
        }

        public static void Unregister(Guid clsid)
        {
            Trace.WriteLine($"Unregistering server:");
            Trace.Indent();
            Trace.WriteLine($"CLSID: {clsid:B}");
            Trace.Unindent();

            string clsidPath = string.Format(@$"{KeyFormat.CLSID}", clsid);

            // delete HKCR\CLSID\{clsid}\LocalServer32
            Registry.LocalMachine.DeleteSubKey(clsidPath + "\\LocalServer32", throwOnMissingSubKey: false);

            // delete HKCR\CLSID\{clsid}\TypeLib
            Registry.LocalMachine.DeleteSubKey(clsidPath + "\\TypeLib", throwOnMissingSubKey: false);

            // delete HKCR\CLSID\{clsid}
            Registry.LocalMachine.DeleteSubKey(clsidPath, throwOnMissingSubKey: false);
        }

        private readonly List<int> registrationCookies = new List<int>();

        public void RegisterClass<T>(Guid clsid) where T : new()
        {
            Trace.WriteLine($"Registering class object:");
            Trace.Indent();
            Trace.WriteLine($"CLSID: {clsid:B}");
            Trace.WriteLine($"Type: {typeof(T)}");

            int cookie;
            int hr = Ole32.CoRegisterClassObject(ref clsid, new BasicClassFactory<T>(), Ole32.CLSCTX_LOCAL_SERVER, Ole32.REGCLS_MULTIPLEUSE | Ole32.REGCLS_SUSPENDED, out cookie);
            if (hr < 0)
                Marshal.ThrowExceptionForHR(hr);

            registrationCookies.Add(cookie);
            Trace.WriteLine($"Cookie: {cookie}");
            Trace.Unindent();

            hr = Ole32.CoResumeClassObjects();
            if (hr < 0)
                Marshal.ThrowExceptionForHR(hr);
        }

        public void WaitForRefCountsToReachZero()
        {
            // For details around ref counting and locking of out-of-proc COM servers, see
            // https://docs.microsoft.com/windows/win32/com/out-of-process-server-implementation-helpers
            while (!m_active || (ComClass.GetObjCount() != 0) || (BasicClassFactoryBase.GetLockCount() != 0))
            {
                Thread.Sleep(1000);
                GC.Collect();
            }
            
        }

        public void Dispose()
        {
            Trace.WriteLine($"Revoking class object registrations:");
            Trace.Indent();
            foreach (int cookie in registrationCookies)
            {
                Trace.WriteLine($"Cookie: {cookie}");
                int hr = Ole32.CoRevokeClassObject(cookie);
                Debug.Assert(hr >= 0, $"CoRevokeClassObject failed ({hr:x}). Cookie: {cookie}");
            }

            Trace.Unindent();
        }

        private class Ole32
        {
            // https://docs.microsoft.com/windows/win32/api/wtypesbase/ne-wtypesbase-clsctx
            public const int CLSCTX_LOCAL_SERVER = 0x4;

            // https://docs.microsoft.com/windows/win32/api/combaseapi/ne-combaseapi-regcls
            public const int REGCLS_MULTIPLEUSE = 1;
            public const int REGCLS_SUSPENDED = 4;

            // https://docs.microsoft.com/windows/win32/api/combaseapi/nf-combaseapi-coregisterclassobject
            [DllImport(nameof(Ole32))]
            public static extern int CoRegisterClassObject(ref Guid guid, [MarshalAs(UnmanagedType.IUnknown)] object obj, int context, int flags, out int register);

            // https://docs.microsoft.com/windows/win32/api/combaseapi/nf-combaseapi-coresumeclassobjects
            [DllImport(nameof(Ole32))]
            public static extern int CoResumeClassObjects();

            // https://docs.microsoft.com/windows/win32/api/combaseapi/nf-combaseapi-corevokeclassobject
            [DllImport(nameof(Ole32))]
            public static extern int CoRevokeClassObject(int register);
        }
    }

    public static class AppID
    {
        /** Uses the CLSID also as AppID for convenience (same as on https://github.com/dotnet/samples/blob/main/core/extensions/OutOfProcCOM/COMRegistration/DllSurrogate.cs) */
        public static void Register(Guid clsid, Guid appid, string description)
        {
            Trace.WriteLine($"Registering server with system-supplied DLL surrogate:");
            Trace.Indent();
            Trace.WriteLine($"CLSID: {clsid:B}");
            Trace.Unindent();

            // write HKCR\CLSID\{clsid}\AppID = {appid}
            using RegistryKey clsidKey = Registry.LocalMachine.CreateSubKey(string.Format(KeyFormat.CLSID, clsid));
            clsidKey.SetValue("AppID", appid.ToString("B"));

            // write HKCR\AppID\{appid}
            // done to create placeholder for later RunAs values
            using RegistryKey appIdKey = Registry.LocalMachine.CreateSubKey(string.Format(KeyFormat.AppID, appid));
            appIdKey.SetValue(null, description);
        }

        public static void Unregister(Guid clsid, Guid appid)
        {
            Trace.WriteLine($"Unregistering server:");
            Trace.Indent();
            Trace.WriteLine($"CLSID: {clsid:B}");
            Trace.Unindent();

            // delete HKCR\CLSID\{clsid}\AppID
            using RegistryKey clsidKey = Registry.LocalMachine.OpenSubKey(string.Format(KeyFormat.CLSID, clsid), writable: true);
            if (clsidKey != null)
                clsidKey.DeleteValue("AppID");

            // delete HKCR\AppID\{appid}
            Registry.LocalMachine.DeleteSubKey(string.Format(KeyFormat.AppID, appid), throwOnMissingSubKey: false);
        }
    }
}
