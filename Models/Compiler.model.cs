using System.Runtime.InteropServices;
namespace CompilerModel
{
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    internal struct CompilerDataPacket
    {
        public IntPtr domain;
        public IntPtr reason_phrase;
        public IntPtr status_code;
        public IntPtr latencies;
        public int total_records;
    }
}