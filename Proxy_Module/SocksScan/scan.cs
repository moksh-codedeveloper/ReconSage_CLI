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
using Struct;
using ScanOutputModel;
using Interface.Network;
using System.Runtime.InteropServices;
using ReconSageLogger;
using Wire;
namespace SocksProxyModule
{
    public class SocksScan : INetwork
    {
        private string Target = string.Empty;
        private string ProxyHost = string.Empty;
        private string ProtoPort = string.Empty;
        private string Headers = string.Empty;
        private int Timeout;
        private int ProxyPort;
        private int Delay;

        [DllImport("proxy_scan_cpp_module.so", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr socks_create(string domain, string proto_port, string proxy_host, string headers, int timeout, int proxy_port);
        
        [DllImport("proxy_scan_cpp_module.so", CallingConvention = CallingConvention.Cdecl)]
        private static extern CppScanOutput socks_scan(string path, IntPtr engine, ref bool cancel_flag);

        [DllImport("proxy_scan_cpp_module.so", CallingConvention = CallingConvention.Cdecl)]
        private static extern void socks_destroy(IntPtr engine);

        public SocksScan(string target, string proxy_host, string proto_port, string headers, int timeout, int proxy_port, int delay)
        {
            Target = target;
            ProxyHost = proxy_host;
            ProtoPort = proto_port;
            Headers = headers;
            ProxyPort = proxy_port;
            Timeout = timeout;
            Delay = delay;
        }
        public async Task<ScanOutput> SendAsync(string path, CancellationToken cts)
        {
            var scanOutput = new ScanOutput();
            bool cancel_flag = false;
            var random = new Random();
            var value = random.Next(Delay, Delay * 1000);
            await Task.Delay(value);
            string sanitizeDomain = new GlobalWires().SanitizeTarget(Target);
            IntPtr engine = socks_create(sanitizeDomain, ProtoPort, ProxyHost, Headers, Timeout, ProxyPort);
            cts.Register(() =>
            {
                cancel_flag = true;
                Logger.Success("Scan Cancelled successfully...");
            });
            CppScanOutput result = socks_scan(path, engine, ref cancel_flag);
            var oldHeaders = result.response_headers;
            scanOutput.Headers = new GlobalWires().ParseHeaders(oldHeaders);
            scanOutput.StatusCode = result.status_code;
            scanOutput.Message = result.reason_phrase;
            scanOutput.Target = result.domain;
            scanOutput.LatencyMS = result.latency_ms;
            socks_destroy(engine);
            return scanOutput;
        }
    }
}