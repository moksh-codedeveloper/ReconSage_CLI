using System.Runtime.InteropServices;
using Interface.Network;
using ReconSageLogger;
using ScanOutputModel;
using Wire;

namespace TorScan
{
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    internal struct TorScanModel
    {
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 360)]
        public string target;
        public int status_code;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 65536)]
        public string response_headers;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
        public string reason_phrase;
        public double latency_ms;
    }
    public class MainTorScan : INetwork
    {
        [DllImport("tor_cpp_module.so", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr CreateEngine(string target, string port, string tor_ip, string password, int tor_port, int cp_tor_port, int timeout);

        [DllImport("tor_cpp_module.so", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr EngineScan(IntPtr engine, string path, ref bool cancelFlag, string target);

        [DllImport("tor_cpp_module.so", CallingConvention = CallingConvention.Cdecl)]
        private static extern void DestroyResult(IntPtr res);

        [DllImport("tor_cpp_module.so", CallingConvention = CallingConvention.Cdecl)]
        private static extern void DestroyEngine(IntPtr engine);

        [DllImport("tor_cpp_module.so", CallingConvention = CallingConvention.Cdecl)]
        public static extern void RotateTorCircuits(IntPtr engine);

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
            await Task.Delay(jitter);
            bool cancelFlag = false;
            var scan = new ScanOutput();
            string sanitizedDomain = wires.SanitizeTarget(Target);
            cts.Register(() =>
            {
                cancelFlag = true;
                Logger.Info("Signal sent to C++ the cleanup will begin shortly");
            });
            IntPtr engine = CreateEngine(target: Target, port: Port, tor_ip: TorIP, password: Password, tor_port: TorPort, cp_tor_port: CpTorPort, timeout: Timeout);
            string cleanPath = domain.StartsWith("/") ? domain : "/" + domain;
            IntPtr result = EngineScan(engine, cleanPath, ref cancelFlag, Target);
            if (result == IntPtr.Zero)
            {
                Logger.Error("Got nullptr from the scan's main engine....");
                return new ScanOutput { Message = "Result = NullPtr, Scan Operation failed" };
            }

            try
            {
                TorScanModel torScanModel = Marshal.PtrToStructure<TorScanModel>(result);
                var oldHeaders = torScanModel.response_headers;
                Dictionary<string, string> newHeaders = wires.ParseHeaders(oldHeaders);
                scan.Target = torScanModel.target;
                scan.LatencyMS = torScanModel.latency_ms;
                scan.StatusCode = torScanModel.status_code;
                scan.Message = torScanModel.reason_phrase;
                scan.Headers = newHeaders;
            }
            finally
            {
                DestroyResult(result);
                DestroyEngine(engine);
            }
            return scan;
        }
    }
}