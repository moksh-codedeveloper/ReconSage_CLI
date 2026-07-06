using ReconSageLogger;
using ResponseBodyStruct;
using System.Runtime.InteropServices;
using Wire;

namespace Capture
{
    public class CaptureScan
    {
        private string domain = string.Empty;
        private string proto_port = string.Empty;
        private string proxy_host = string.Empty;
        private string dns_server = string.Empty;
        private int proxy_port;
        private int timeout;

        public CaptureScan(string Domain, string ProtoPort, string ProxyHost, string DnsServer, int ProxyPort, int Timeout)
        {
            domain = Domain;
            proto_port = ProtoPort;
            proxy_host = ProxyHost;
            dns_server = DnsServer;
            proxy_port = ProxyPort;
            timeout = Timeout;
        }

        [DllImport("res_body_scan_cpp_module.so", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr create_res_body_capture_engine(string domain, string proxy_host, string proto_port, string dns_server, int proxy_port, int timeout);

        [DllImport("res_body_scan_cpp_module.so", CallingConvention = CallingConvention.Cdecl)]
        private static extern BodyStruct res_cap_scan(IntPtr engine, string path, ref bool cancelFlag);

        [DllImport("res_body_scan_cpp_module.so", CallingConvention = CallingConvention.Cdecl)]
        private static extern void destroy_res_captio_engine(IntPtr engine);

        public ScanResponseBodyModel Scan(string path, CancellationToken cts)
        {
            ScanResponseBodyModel result = new ScanResponseBodyModel();
            IntPtr engine = create_res_body_capture_engine(domain, proxy_host, proto_port, dns_server, proxy_port, timeout);
            bool cancelFlag = false;
            using (cts.Register(() =>
            {
                Logger.Error("[+] Scan is Cancelled is by the attacker");
                cancelFlag = true;
            }))
            {
                string sanitizedTarget = new GlobalWires().SanitizeTarget(domain);
                if (cts.IsCancellationRequested)
                {
                    destroy_res_captio_engine(engine);
                    return result;
                }
                string cleanPath = path.StartsWith("/") ? path : "/" + path;
                BodyStruct bodyStruct = res_cap_scan(engine, path, ref cancelFlag);
                if (!cts.IsCancellationRequested)
                {
                    destroy_res_captio_engine(engine);
                    return result;
                }
                result.target = bodyStruct.domain;
                result.bodyResponse = bodyStruct.captured_body;
                Logger.Info($"[Debug C++] Here is your body {result.bodyResponse}");
                destroy_res_captio_engine(engine);
                return result;
            }
        }
    }
}