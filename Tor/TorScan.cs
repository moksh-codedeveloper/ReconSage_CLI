using System.Runtime.InteropServices;
using Interface.Network;
using ReconSageLogger;
using ScanOutputModel;
using Wire;
using Struct;

namespace TorScan
{
    public class MainTorScan : INetwork
    {
        // FIX 1: Exact byte-by-byte sequence matching with C++ extern "C" create_engine parameters
        [DllImport("tor_cpp_module.so", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr create_engine(
            string domain,      // char domain[256]
            string proto_port,  // char proto_port[128]
            string headers,     // char headers[8192]
            string tor_ip,      // char tor_ip[256]
            string password,    // char password[8192]
            int timeout,        // int timeout
            int tor_port,       // int tor_port
            int cp_tor_port     // int cp_tor_port
        );

        [DllImport("tor_cpp_module.so", CallingConvention = CallingConvention.Cdecl)]
        private static extern CppScanOutput tor_scan_engine(string path, IntPtr engine, ref bool cancelFlag);

        [DllImport("tor_cpp_module.so", CallingConvention = CallingConvention.Cdecl)]
        private static extern void destroy_tor_engine(IntPtr engine);

        private string Target = string.Empty;
        private string Password = string.Empty;
        private string Port = string.Empty;
        private string TorIP = string.Empty;
        private int TorPort;
        private int CpTorPort;
        private int Timeout;
        private int Delay;
        private GlobalWires wires;

        public MainTorScan(string _target, string _port, string _password, string _tor_ip, int _tor_port, int _cp_tor_port, int _timeout, int _delay)
        {
            Target = _target;
            Password = _password;
            Port = _port;
            TorIP = _tor_ip;
            TorPort = _tor_port;
            CpTorPort = _cp_tor_port;
            Timeout = _timeout;
            Delay = _delay;
            wires = new GlobalWires();
        }

        public async Task<ScanOutput> SendAsync(string domain, CancellationToken cts)
        {
            var randomJitter = new Random();
            var jitter = randomJitter.Next(Delay, Delay * 10);
            Logger.Info($"Delay :- {jitter}");
            
            await Task.Delay(jitter, cts);
            
            bool cancelFlag = false;
            var scan = new ScanOutput();
            
            using (cts.Register(() =>
            {
                cancelFlag = true;
                Logger.Info("Abort signal sent to C++ Tor engine state layer...");
            }))
            {
                if (cts.IsCancellationRequested) return scan;

                // FIX 2: Sequenced values passed in precise order to match the DLL stack allocation layout
                IntPtr engine = create_engine(
                    domain: Target, 
                    proto_port: Port, 
                    headers: "", 
                    tor_ip: TorIP, 
                    password: Password, 
                    timeout: Timeout, 
                    tor_port: TorPort, 
                    cp_tor_port: CpTorPort
                );
                
                if (engine == IntPtr.Zero)
                {
                    Logger.Error("Failed to initialize unmanaged Tor Engine context pointer.");
                    return new ScanOutput { Message = "Engine initialization failed" };
                }

                string cleanPath = domain.StartsWith("/") ? domain : "/" + domain;
                
                try
                {
                    CppScanOutput torScanModel = tor_scan_engine(cleanPath, engine, ref cancelFlag);

                    if (string.IsNullOrEmpty(torScanModel.domain) && torScanModel.status_code == 0)
                    {
                        Logger.Error("Empty return payload structure or operation cancelled.");
                        return new ScanOutput { Message = "Result empty, Scan Operation failed or halted" };
                    }

                    var oldHeaders = torScanModel.response_headers;
                    Dictionary<string, string> newHeaders = wires.ParseHeaders(oldHeaders);
                    
                    scan.Target = torScanModel.domain;
                    scan.LatencyMS = torScanModel.latency_ms;
                    scan.StatusCode = torScanModel.status_code;
                    scan.Message = torScanModel.reason_phrase;
                    scan.Headers = newHeaders;
                }
                catch (Exception ex)
                {
                    Logger.Error($"Internal marshalling conversion error caught: {ex.Message}");
                    scan.Message = $"Marshalling Failure: {ex.Message}";
                }
                finally
                {
                    destroy_tor_engine(engine);
                }
                
                return scan;
            }
        }
    }
}