using System.Runtime.InteropServices;
using Interface.Network;
using ScanOutputModel;

namespace OnionScan
{
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    internal struct OnionModule
    {
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
        public string target;
        public long statusCode;
        public IntPtr response_headers; // char* = IntPtr
        public IntPtr response_body;    // char* = IntPtr
        public double latencyMs;
    }

    public class OnionScanModule : INetwork
    {
        [DllImport("parser_cpp_module.so", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr scan(string target, string tor_ip, string host, string password, string wordlist_path, string json_file_name, ushort tor_port, ushort port, int timeout, int delay);
        [DllImport("parser_cpp_module.so", CallingConvention = CallingConvention.Cdecl)]
        private static extern void free_scan(IntPtr scan);

        private string Target;
        private string Tor_IP;
        private string Host;
        private string WordlistPath;
        private string JsonFilePath;
        private string Password;
        private int Timeout;
        private int Delay;
        private int Tor_port;
        private int Port;

        public OnionScanModule(string target, string host, string tor_ip, string wordlist_path, string json_file_name, string password, int port, int tor_port, int timeout, int delay)
        {
            Target = target;
            Host = host;
            Tor_IP = tor_ip;
            WordlistPath = wordlist_path;
            JsonFilePath = json_file_name;
            Timeout = timeout;
            Delay = delay;
            Tor_port = tor_port;
            Port = port;
            Password = password;
        }
        private Dictionary<string, string> ParseHeaders(string rawHeaders)
        {
            var dict = new Dictionary<string, string>();
            var lines = rawHeaders.Split("\r\n",
                        StringSplitOptions.RemoveEmptyEntries);

            foreach (var line in lines)
            {
                int colon = line.IndexOf(':');
                if (colon == -1) continue; // skip status line
                string key = line[..colon].Trim();
                string value = line[(colon + 1)..].Trim();
                dict.TryAdd(key, value);
            }
            return dict;
        }
        public async Task<ScanOutput> SendAsync(string domain)
        {
            IntPtr scan_result = scan(Target, Tor_IP, Host, Password, WordlistPath, JsonFilePath, (ushort)Tor_port, (ushort)Port, Timeout, Delay);
            if (scan_result == IntPtr.Zero) throw new Exception("Onion scan failed");
            OnionModule onion = Marshal.PtrToStructure<OnionModule>(scan_result);
            string headers = Marshal.PtrToStringAnsi(onion.response_headers) ?? "";

            free_scan(scan_result);
            return new ScanOutput
            {
                Target = Target,
                StatusCode = (int)onion.statusCode,
                Headers = ParseHeaders(headers),
                Message = ((System.Net.HttpStatusCode)onion.statusCode).ToString(),
                LatencyMS = onion.latencyMs
            };
        }
    }
}