using System;
using System.Runtime.InteropServices;

[ComVisible(true)]
[Guid("F586D6F4-AF37-441E-80A6-3D33D977882D")]
public interface IServer
{
    /// <summary>
    /// Compute the value of the constant Pi.
    /// </summary>
    double ComputePi();
}
