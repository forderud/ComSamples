﻿using System;
using System.Runtime.InteropServices;
using System.Threading;

namespace ComSupport
{
    // https://docs.microsoft.com/windows/win32/api/unknwn/nn-unknwn-iclassfactory
    [ComImport]
    [ComVisible(false)]
    [Guid("00000001-0000-0000-C000-000000000046")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    internal interface IClassFactory
    {
        void CreateInstance(
            [MarshalAs(UnmanagedType.Interface)] object pUnkOuter,
            ref Guid riid,
            out IntPtr ppvObject);

        void LockServer([MarshalAs(UnmanagedType.Bool)] bool fLock);
    }

    internal class BasicClassFactoryBase
    {
        internal static int m_lock_cnt = 0;

        public static int GetLockCount()
        {
            return m_lock_cnt;
        }
    }

    [ComVisible(true)]
    internal class BasicClassFactory<T> : BasicClassFactoryBase, IClassFactory where T : new()
    {
        public void CreateInstance(
            [MarshalAs(UnmanagedType.Interface)] object pUnkOuter,
            ref Guid riid,
            out IntPtr ppvObject)
        {
            Type interfaceType = GetValidatedInterfaceType(typeof(T), ref riid, pUnkOuter);

            object obj = new T();
            if (pUnkOuter != null)
                obj = CreateAggregatedObject(pUnkOuter, obj);

            ppvObject = GetObjectAsInterface(obj, interfaceType);

            LocalServer.m_active = true; // first object created
        }

        public void LockServer([MarshalAs(UnmanagedType.Bool)] bool fLock) {
            if (fLock)
                Interlocked.Increment(ref m_lock_cnt);
            else
                Interlocked.Decrement(ref m_lock_cnt);
        }

        private static readonly Guid IID_IUnknown = Guid.Parse("00000000-0000-0000-C000-000000000046");

        private static Type GetValidatedInterfaceType(Type classType, ref Guid riid, object outer)
        {
            if (riid == IID_IUnknown)
                return typeof(object);

            // Aggregation can only be done when requesting IUnknown.
            if (outer != null)
            {
                const int CLASS_E_NOAGGREGATION = unchecked((int)0x80040110);
                throw new COMException(string.Empty, CLASS_E_NOAGGREGATION);
            }

            // Verify the class implements the desired interface
            foreach (Type i in classType.GetInterfaces())
            {
                if (i.GUID == riid)
                    return i;
            }

            // E_NOINTERFACE
            throw new InvalidCastException();
        }

        private static IntPtr GetObjectAsInterface(object obj, Type interfaceType)
        {
            // If the requested "interface type" is type object then return as IUnknown
            if (interfaceType == typeof(object))
                return Marshal.GetIUnknownForObject(obj);

            IntPtr interfaceMaybe = Marshal.GetComInterfaceForObject(obj, interfaceType, CustomQueryInterfaceMode.Ignore);
            if (interfaceMaybe == IntPtr.Zero)
                throw new InvalidCastException(); // E_NOINTERFACE

            return interfaceMaybe;
        }

        private static object CreateAggregatedObject(object pUnkOuter, object comObject)
        {
            IntPtr outerPtr = Marshal.GetIUnknownForObject(pUnkOuter);

            try
            {
                IntPtr innerPtr = Marshal.CreateAggregatedObject(outerPtr, comObject);
                return Marshal.GetObjectForIUnknown(innerPtr);
            }
            finally
            {
                // Decrement the above 'Marshal.GetIUnknownForObject()'
                Marshal.Release(outerPtr);
            }
        }
    }
}
