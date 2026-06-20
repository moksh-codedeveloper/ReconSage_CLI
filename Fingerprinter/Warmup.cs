using System.Runtime.InteropServices;
using Interface.Network;
using ReconSageLogger;
using ScanOutputModel;
using Wire;
using Struct;

namespace NormalScan
{
    public class CppScan : INetwork
    {
        [DllImport("scan_cpp_module.so", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr create_engine(string path, string proto_port, int timeout, string headers);

        [DllImport("scan_cpp_module.so", CallingConvention = CallingConvention.Cdecl)]
        //now see here in args 
        private static extern CppScanOutput engine_scan(IntPtr engine, string path, ref bool cancelFlag);

        [DllImport("scan_cpp_module.so", CallingConvention = CallingConvention.Cdecl)]
        private static extern void engine_destroy(IntPtr engine);

        private string Target = string.Empty;
        private int Timeout;
        private int Delay;
        private string port = string.Empty;
        private string Headers = string.Empty;
        public CppScan(string target, int timeout, int delay, string Port, string headers)
        {
            Target = target;
            Timeout = timeout;
            Delay = delay;
            port = Port;
            Headers = headers;
        }

        public async Task<ScanOutput> SendAsync(string path, CancellationToken  cts)
        {
            var scanOutput = new ScanOutput();
            IntPtr engine = create_engine(Target, port, Timeout, Headers);
            bool cancelFlag = false;
            string sanitizedTarget = new GlobalWires().SanitizeTarget(Target);
            cts.Register(() =>
            {
                engine_destroy(engine);
                cancelFlag = true;
                Console.WriteLine("[!] Signal sent to C++ Engine...");
            });
            Random jitter = new Random();
            var value = jitter.Next(Delay, Delay * 100);
            Logger.Info($"Delay in scan :- {value}");
            await Task.Delay(value);

            string cleanPath = path.StartsWith("/") ? path : "/" + path;
            CppScanOutput resultPtr = engine_scan(engine, cleanPath, ref cancelFlag);
            string resHeader = resultPtr.response_headers;
            var headers = new GlobalWires().ParseHeaders(resHeader);
            scanOutput.Headers = headers;
            scanOutput.StatusCode = resultPtr.status_code;
            scanOutput.Message = resultPtr.reason_phrase;
            scanOutput.LatencyMS = resultPtr.latency_ms;
            scanOutput.Target = resultPtr.domain;
            engine_destroy(engine);
            return scanOutput;
        }
    }
}