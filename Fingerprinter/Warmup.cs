using System.Runtime.InteropServices;
using Interface.Network;
using ReconSageLogger;
using ScanOutputModel;
using Wire;

namespace NormalScan
{
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    internal struct ScanStruct
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

    public class CppScan : INetwork
    {
        [DllImport("scan_cpp_module.so", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr CreateEngine(int timeout);

        [DllImport("scan_cpp_module.so", CallingConvention = CallingConvention.Cdecl)]
        //now see here in args 
        private static extern IntPtr PerformScan(IntPtr res, string target, string path, string port, ref bool cancelFlag);

        [DllImport("scan_cpp_module.so", CallingConvention = CallingConvention.Cdecl)]
        private static extern void DestroyResult(IntPtr res);

        private string Target = string.Empty;
        private int Timeout;
        private int Delay;
        private string port = string.Empty;

        public CppScan(string target, int timeout, int delay, string Port)
        {
            Target = target;
            Timeout = timeout;
            Delay = delay;
            port = Port;
        }

        public async Task<ScanOutput> SendAsync(string domain)
        {
            bool cancelFlag = false;
            string sanitizedTarget = new GlobalWires().SanitizeTarget(Target);
            var cts = new CancellationTokenSource();
            cts.Token.Register(() =>
            {
                cancelFlag = true;
                Console.WriteLine("[!] Signal sent to C++ Engine...");
            });
            Random jitter = new Random();
            var value = jitter.Next(Delay, Delay * 100);
            Logger.Info($"Delay in scan :- {value}");
            await Task.Delay(value);

            // Engine create kiya
            IntPtr engine = CreateEngine(Timeout);

            string cleanPath = domain.StartsWith("/") ? domain : "/" + domain;
            IntPtr resultPtr = PerformScan(engine, sanitizedTarget, cleanPath, port, ref cancelFlag);

            if (resultPtr == IntPtr.Zero)
            {
                Logger.Error("Scan has been either aborted or it has failed");
                return new ScanOutput { Message = "Scan Failed" };
            }

            try
            {
                ScanStruct scanResult = Marshal.PtrToStructure<ScanStruct>(resultPtr);
                string orHeaders = scanResult.response_headers;
                Dictionary<string, string> rlHeadears = new GlobalWires().ParseHeaders(orHeaders);

                ScanOutput scanOutput = new ScanOutput
                {
                    Target = scanResult.target,
                    StatusCode = scanResult.status_code,
                    Headers = rlHeadears,
                    LatencyMS = scanResult.latency_ms,
                    Message = scanResult.reason_phrase
                };

                return scanOutput;
            }
            finally
            {
                DestroyResult(resultPtr);
            }
        }
    }
}