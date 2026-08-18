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
        private static extern IntPtr create_engine(string path, string proto_port, int timeout, string headers, string dns_server);

        [DllImport("scan_cpp_module.so", CallingConvention = CallingConvention.Cdecl)]
        private static extern CppScanOutput engine_scan(IntPtr engine, string path, ref bool cancelFlag);

        [DllImport("scan_cpp_module.so", CallingConvention = CallingConvention.Cdecl)]
        private static extern void engine_destroy(IntPtr engine);

        private string Target = string.Empty;
        private int Timeout;
        private int Delay;
        private string port = string.Empty;
        private string Headers = string.Empty;
        private string DNSServer = string.Empty;

        public CppScan(string target, int timeout, int delay, string Port, string headers, string dns_server)
        {
            Target = target;
            Timeout = timeout;
            Delay = delay;
            port = Port;
            Headers = headers;
            DNSServer = dns_server;
        }

        public async Task<ScanOutput> SendAsync(string path, CancellationToken cts)
        {
            var scanOutput = new ScanOutput();
            IntPtr engine = create_engine(Target, port, Timeout, Headers, DNSServer);
            
            // Shared cancellation state across managed/unmanaged boundary
            bool cancelFlag = false;
            
            // FIX 1: Safely handle cancellation token without double-free race conditions
            using (cts.Register(() =>
            {
                cancelFlag = true; // Signal the C++ internal loops to safely halt execution
                Console.WriteLine("[!] Abort signal piped safely to C++ native engine state layer...");
            }))
            {
                string sanitizedTarget = new GlobalWires().SanitizeTarget(Target);

                Random jitter = new Random();
                var value = jitter.Next(Delay, Delay * 100);
                Logger.Info($"Delay in scan :- {value}");
                
                // Handling pre-scan async delay tracking
                await Task.Delay(value, cts); 

                if (cts.IsCancellationRequested)
                {
                    engine_destroy(engine);
                    return scanOutput;
                }

                string cleanPath = path.StartsWith("/") ? path : "/" + path;
                
                // Execute unmanaged scanner assembly block safely
                CppScanOutput resultPtr = engine_scan(engine, cleanPath, ref cancelFlag);
                
                if (cts.IsCancellationRequested)
                {
                    engine_destroy(engine);
                    return scanOutput;
                }

                string resHeader = resultPtr.response_headers;
                var headers = new GlobalWires().ParseHeaders(resHeader);
                
                scanOutput.Headers = headers;
                scanOutput.StatusCode = resultPtr.status_code;
                scanOutput.Message = resultPtr.reason_phrase;
                scanOutput.LatencyMS = resultPtr.latency_ms;
                scanOutput.Target = resultPtr.domain;
                
                // FIX 2: Single, unified destroy footprint at the end of normal execution channel
                engine_destroy(engine);
                return scanOutput;
            }
        }
    }
}