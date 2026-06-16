using System.Runtime.InteropServices;

namespace Struct
{
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    internal struct CppScanOutput
    {
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 360)]
        public string target;

        public int status_code;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 65536)]
        public string response_headers; // Flat array, no pointer!

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
        public string reason_phrase;

        public double latency_ms;
    }

}