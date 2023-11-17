using System;
using System.Runtime.InteropServices;

class ComMarshal
{
    /** Check if the underlying COM objects are the same, regardless of apartment differences. */
    static public bool EqualComObjects(object obj_a, object obj_b)
    {
        // Marshal objects to the _same_ apartment (the current thread apartment) before comparing.
        // The target apartment doesn't necessarily need to be the "native" apartment for the object,
        // but could point to a COM proxy. This doesn't matter, since identical objects will then point to the same proxy.
        // Please note that marshalling is redundant if both objects already reside in the same apartment.
        IntPtr obj_a_prt = Marshal.GetIUnknownForObject(obj_a);
        IntPtr obj_b_prt = Marshal.GetIUnknownForObject(obj_b);

        bool equal = Marshal.GetObjectForIUnknown(obj_a_prt) == Marshal.GetObjectForIUnknown(obj_b_prt);

        Marshal.Release(obj_b_prt);
        Marshal.Release(obj_a_prt);

        return equal;
    }
}
