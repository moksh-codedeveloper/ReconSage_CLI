using System.Runtime.InteropServices;
using Struct;
using ScanOutputModel;
using Interface.Network;
using ReconSageLogger;
using Wire;
namespace HttpScan
{
    public class HttpProxyScan : INetwork
    {
        [DllImport("proxy_scan_cpp_module.so", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr http_create(string domain, string proto_port, string headers, string proxy_host, int timeout, int proxy_port);

        [DllImport("proxy_scan_cpp_module.so", CallingConvention = CallingConvention.Cdecl)]
        private static extern CppScanOutput http_scan(IntPtr res, string path, ref bool CancelFlag);

        [DllImport("proxy_scan_cpp_module.so", CallingConvention = CallingConvention.Cdecl)]

        private static extern void http_engine_destroy(IntPtr engine);

        private string Target = string.Empty;
        private string ProtoPort = string.Empty;
        private string ProxyHost = string.Empty;
        private string Headers = string.Empty;
        private int ProxyPort;
        private int Timeout;
        private int Delay;

        public HttpProxyScan(string target, string proto_port, string proxy_host, string headers, int proxy_port, int timeout, int delay)
        {
            Target = target;
            ProxyHost = proxy_host;
            ProtoPort = proto_port;
            Headers = headers;
            Timeout = timeout;
            ProxyPort = proxy_port;
            Delay = delay;
        }

        public async Task<ScanOutput> SendAsync(string path, CancellationToken cts)
        {
            var scanOutput = new ScanOutput();
            bool cancelFlag = false;
            cts.Register(() =>
            {
                cancelFlag = true;
                Console.WriteLine("[+]Scan is Cancelled");
            });
            Random jitter = new Random();
            var value = jitter.Next(Delay, Delay * 1000);
            Logger.Info($"[+]Delay :- {value}");
            await Task.Delay(value);
            string sanitizeDomain = new GlobalWires().SanitizeTarget(Target);
            IntPtr engine = http_create(sanitizeDomain, ProtoPort, Headers, ProxyHost, Timeout, ProxyPort);

            CppScanOutput result = http_scan(engine, path, ref cancelFlag);

            var oldHeaders = result.response_headers;
            scanOutput.Headers = new GlobalWires().ParseHeaders(oldHeaders);
            scanOutput.StatusCode = result.status_code;
            scanOutput.Message = result.reason_phrase;
            scanOutput.Target = result.domain;
            scanOutput.LatencyMS = result.latency_ms;
            http_engine_destroy(engine);
            return scanOutput;
        }
    }
}