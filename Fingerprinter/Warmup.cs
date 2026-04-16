using System.Runtime.InteropServices;
using ScanOutputModel;
using Wire;

namespace NormalScan
{
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    internal struct ScanStruct
    {
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 357)]
        public string target;
        public int status_code;
        public IntPtr response_headers;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
        public string reason_phrase;
        public double latency_ms;
    }

    public class CppScan
    {
        [DllImport("scan_cpp_module.so", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr CreateEngine(int timeout);

        [DllImport("scan_cpp_module.so", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr PerformScan(IntPtr res, string target, string path, string port);

        [DllImport("scan_cpp_module.so", CallingConvention = CallingConvention.Cdecl)]
        private static extern void DestroyResult(IntPtr res);

        private string Target = string.Empty;
        private int Timeout;
        private int Delay;
        private string WordlistPath = string.Empty;
        private string port = string.Empty;

        public CppScan(string target, int timeout, int delay, string wordlistPath, string Port)
        {
            Target = target;
            Timeout = timeout;
            Delay = delay;
            WordlistPath = wordlistPath;
            port = Port;
        }

        public async Task<ScanOutput> ExecScanCpp(string domain)
        {
            await Task.Delay(Delay);

            // Engine create kiya
            IntPtr engine = CreateEngine(Timeout);

            // Scan perform kiya
            IntPtr resultPtr = PerformScan(engine, Target, domain, port);

            if (resultPtr == IntPtr.Zero) return new ScanOutput { Message = "Scan Failed" };

            try
            {
                // Unmanaged memory ko C# struct mein map kiya
                ScanStruct scanResult = Marshal.PtrToStructure<ScanStruct>(resultPtr);
                string orHeaders = Marshal.PtrToStringAnsi(scanResult.response_headers) ?? "";

                // Dictionary transform
                Dictionary<string, string> rlHeadears = new GlobalWires().ParseHeaders(orHeaders);

                // ScanOutput model fill kiya
                ScanOutput scanOutput = new ScanOutput
                {
                    Target = scanResult.target,
                    StatusCode = scanResult.status_code,
                    Headers = rlHeadears,
                    LatencyMS = scanResult.latency_ms,
                    Message = scanResult.reason_phrase
                };

                return scanOutput; // Asli data return karo!
            }
            finally
            {
                // Sabse important: C++ ki memory saaf karo (No leaks!)
                DestroyResult(resultPtr);
            }
        }
    }
}