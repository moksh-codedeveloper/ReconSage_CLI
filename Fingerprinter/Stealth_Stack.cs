using System.Diagnostics;
using System.Net;
using System.Net.Security;
using System.Security.Cryptography.X509Certificates;
using MihaZupan;
using ScanOutputModel;
using WarmUpScan;
using Wire;

namespace StealthStack
{
    public static class HeaderDisguise
    {
        private static readonly Random _rnd = new Random();

        private static readonly string[] UserAgents = {
        "Mozilla/5.0 (X11; Arch Linux; Linux x86_64; rv:124.0) Gecko/20100101 Firefox/124.0",
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/122.0.0.0 Safari/537.36",
        "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/122.0.0.0 Safari/537.36",
        "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/121.0.0.0 Safari/537.36"
    };

        public static void Apply(HttpRequestMessage request)
        {
            // Set the User-Agent
            request.Headers.UserAgent.ParseAdd(UserAgents[_rnd.Next(UserAgents.Length)]);

            // Add the "Sec-Ch-Ua" headers (Modern Browser Fingerprinting)
            request.Headers.Add("sec-ch-ua", "\"Chromium\";v=\"122\", \"Not(A:Brand\";v=\"24\", \"Google Chrome\";v=\"122\"");
            request.Headers.Add("sec-ch-ua-mobile", "?0");
            request.Headers.Add("sec-ch-ua-platform", "\"Linux\"");

            // Essential for bypassing basic bot filters
            request.Headers.Add("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8");
            request.Headers.Add("Accept-Language", "en-US,en;q=0.5");
            request.Headers.Add("Sec-Fetch-Dest", "document");
            request.Headers.Add("Sec-Fetch-Mode", "navigate");
            request.Headers.Add("Sec-Fetch-Site", "none");
        }
    }
    // TODO :- add the tor scanning and integrated the tor rotation interface here and add the actual tor scanner and integrate proxy layer 
    public class StealthEngine
    {
        public readonly string Target;
        private readonly int Timeout;
        private readonly string JsonFilePath;
        private readonly string WordlistPath;
        private readonly string host;
        private readonly int port;
        private readonly string password;
        private readonly string tor_ip;
        private readonly int tor_port;
        private readonly Random _jitter = new();
        public StealthEngine(string target, int timeout,  string jsonFilePath, string wordlistPath, string _host, int _port, string _password, string _tor_host, int _tor_port)
        {
            Target = target;
            Timeout = timeout;
            JsonFilePath = jsonFilePath;
            WordlistPath = wordlistPath;
            host = _host;
            port = _port;
            password = _password;
            tor_ip = _tor_host;
            tor_port = _tor_port;
        }

        public async Task<ScanOutput> NormalTorScan(string url, int minJitterValueMS)
        {
            var scan = new ScanOutput{Target = url};
            using var handler = new SocketsHttpHandler
            {
                Proxy = new HttpToSocks5Proxy(tor_ip, tor_port),
                UseProxy = true,
            };
            var request = new HttpRequestMessage(HttpMethod.Get, url);
            using var client = new HttpClient(handler);
            var wires = new GlobalWires();
            var sw = Stopwatch.StartNew();
            using var cts = new CancellationTokenSource(TimeSpan.FromSeconds(Timeout));
            try
            {
                await Task.Delay(_jitter.Next(minJitterValueMS, minJitterValueMS * 10));
                var result = await client.SendAsync(request, cts.Token);
                sw.Stop();
                if (wires.IsDetected((int)result.StatusCode))
                {
                    HeaderDisguise.Apply(request);
                }
                scan.StatusCode = (int)result.StatusCode;
                scan.Headers = result.Headers.ToDictionary(h => h.Key, h => string.Join(",", h.Value));
                scan.LatencyMS = sw.ElapsedMilliseconds;
                scan.Message = $"{result.Version} | {result.ReasonPhrase}";
            }
            catch (Exception ex)
            {
                sw.Stop();
                scan.LatencyMS = sw.ElapsedMilliseconds;
                scan.Headers = new Dictionary<string, string>();
                scan.StatusCode = 0;
                scan.Message = ex.Message;
            }
            return scan;
        }
    }
}