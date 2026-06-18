using Wire;
using Struct;
using System.Runtime.InteropServices;
using ScanOutputModel;
using ReconSageLogger;
using Interface.Network;

namespace HttpsProxyScan
{
    public class HttpsScan : INetwork
    {
        [DllImport("proxy_scan_cpp_module.so", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr https_create(string domain, string proto_port, string proxy_host, string headers, int timeout, int proxy_port);

        [DllImport("proxy_scan_cpp_module.so", CallingConvention = CallingConvention.Cdecl)]
        private static extern CppScanOutput https_scan(string path, IntPtr engine, ref bool cancel_flag);

        [DllImport("proxy_scan_cpp_module.so", CallingConvention = CallingConvention.Cdecl)]
        private static extern void https_destroy(IntPtr engine);

        private string Target = string.Empty;
        private string Headers = string.Empty;
        private string ProxyHost = string.Empty;
        private string ProtoPort = string.Empty;
        private int Timeout;
        private int Delay;
        private int ProxyPort;

        public HttpsScan(string target, string proxy_host, string proto_port, string headers, int timeout, int proxy_port, int delay)
        {
            Target = target;
            Headers = headers;
            ProxyHost = proxy_host;
            ProtoPort = proto_port;
            Timeout = timeout;
            Delay = delay;
            ProxyPort = proxy_port;
        }
        public async Task<ScanOutput> SendAsync(string path, CancellationToken cts)
        {
            var scanResult = new ScanOutput();
            var wires = new GlobalWires();
            bool cancelFlag = false;
            cts.Register(() =>
            {
                cancelFlag = true;
                Logger.Warn("Scan Cancelled and flag updated......");
            });
            Random jitter = new Random();
            var value = jitter.Next(Delay, Delay * 1000);
            Logger.Info($"Value of jitter :- {value}");
            await Task.Delay(value);
            IntPtr engine = https_create(Target, ProtoPort, ProxyHost, Headers, Timeout, ProxyPort);
            CppScanOutput scan = https_scan(path, engine, ref cancelFlag);
            var oldHeaders = scan.response_headers;
            scanResult.Headers = wires.ParseHeaders(oldHeaders);
            scanResult.StatusCode = scan.status_code;
            scanResult.Message = scan.reason_phrase;
            scanResult.Target = scan.domain;
            scanResult.LatencyMS = scan.latency_ms;
            https_destroy(engine);
            return scanResult;
        }
    }
}