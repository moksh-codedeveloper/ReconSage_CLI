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
        // FIX 2: Aligned parameters structure sequence with the C++ extern "C" create_engine exports
        [DllImport("tor_cpp_module.so", EntryPoint = "create_engine", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr CreateEngine(string target, string tor_ip, string proto_port, string headers, string password, int tor_port, int cp_tor_port, int timeout);

        // FIX 3: Return the struct direct-copy from stack execution frame instead of dynamic IntPtr pointer addresses
        [DllImport("tor_cpp_module.so", EntryPoint = "tor_scan_engine", CallingConvention = CallingConvention.Cdecl)]
        private static extern CppScanOutput EngineScan(IntPtr engine, string path, ref bool cancelFlag);

        [DllImport("tor_cpp_module.so", EntryPoint = "destroy_tor_engine", CallingConvention = CallingConvention.Cdecl)]
        private static extern void DestroyEngine(IntPtr engine);

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
            
            // Safe pre-scan delay cancellation integration
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

                // Fire initialization allocation
                IntPtr engine = CreateEngine(target: Target, tor_ip: TorIP, proto_port: Port, headers: "", password: Password, tor_port: TorPort, cp_tor_port: CpTorPort, timeout: Timeout);
                
                if (engine == IntPtr.Zero)
                {
                    Logger.Error("Failed to initialize unmanaged Tor Engine context pointer.");
                    return new ScanOutput { Message = "Engine initialization failed" };
                }

                string cleanPath = domain.StartsWith("/") ? domain : "/" + domain;
                
                try
                {
                    // FIX 4: Received Direct Struct payload directly mapped onto stack registers
                    CppScanOutput torScanModel = EngineScan(engine, cleanPath, ref cancelFlag);

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
                    // FIX 5: Single ownership destroy. Dropped old 'DestroyResult' because the struct lives on stack now!
                    DestroyEngine(engine);
                }
                
                return scan;
            }
        }
    }
}