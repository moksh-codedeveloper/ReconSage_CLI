using System.Runtime.InteropServices;

namespace Struct
{
    // Remove Pack = 1 to allow natural 8-byte alignment match
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    internal struct CppScanOutput
    {
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 3072)]
        public string domain;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 65536)]
        public string response_headers;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
        public string reason_phrase;

        public int status_code;
        public double latency_ms; // Will now sit at the correct byte offset position
    }
}