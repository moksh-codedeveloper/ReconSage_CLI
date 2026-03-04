using System.Diagnostics;
using System.Net;
using Interface.Network;
using ScanOutputModel;

namespace ScanInterface
{
    public class NormalScan : INetwork
    {
        public string Target { set; get; } = string.Empty;
        public int Concurrency { set; get; }
        public int Timeout { set; get; }
        private readonly HttpClient _client;
        public NormalScan(string target, int concurrency, int timeout)
        {
            Target = target;
            Concurrency = concurrency;
            Timeout = timeout;
            _client = new HttpClient();
            Concurrency = concurrency;
        }

        public async Task<ScanOutput> SendAsync(string domain)
        {
            string subDomain = Target + domain;
            ScanOutput scanOutputModel = new ScanOutput();
            scanOutputModel.Target = subDomain;
            CancellationTokenSource cts = new CancellationTokenSource(TimeSpan.FromSeconds(Timeout));
            var sw = Stopwatch.StartNew();
            try
            {
                var result = await _client.GetAsync(subDomain, cts.Token);
                sw.Stop();
                var Latency = sw.ElapsedMilliseconds;
                var Message = result.ReasonPhrase;
                Dictionary<string, string> Header = result.Headers.ToDictionary(h => h.Key, h => string.Join(",", h.Value));
                scanOutputModel.LatencyMS = Latency;
                scanOutputModel.StatusCode = (int)result.StatusCode;
                scanOutputModel.Message = Message ?? string.Empty;
                scanOutputModel.Headers = Header;
            }
            catch (Exception ex)
            {
                Console.WriteLine("This is the Exception which keeps coming..... :- ", ex.Message);
                scanOutputModel.Message = ex.Message;
            }
            return scanOutputModel;
        }
    }
}