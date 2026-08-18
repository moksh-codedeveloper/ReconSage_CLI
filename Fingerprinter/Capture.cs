/*
 * ReconSage_Cli - Advanced Network & Telemetry Reconnaissance Framework
 * Copyright (C) 2026 ReconSage_Cli Authors
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
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
                result.target = bodyStruct.domain;
                result.bodyResponse = bodyStruct.captured_body;
                result.statusCode = bodyStruct.statusCode;
                Logger.Info($"[Debug C++] Here is your body {bodyStruct.captured_body}");
                destroy_res_captio_engine(engine);
                return result;
            }
        }
    }
}