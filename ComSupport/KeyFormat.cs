namespace ComSupport
{
    internal static class KeyFormat
    {
        public const string CLSID = @"SOFTWARE\Classes\CLSID\{0:B}";

        public static readonly string LocalServer32 = @$"{CLSID}\LocalServer32";
    }
}
