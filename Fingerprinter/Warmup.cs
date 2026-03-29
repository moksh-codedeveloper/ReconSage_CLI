using System.Diagnostics;
using ReconSageLogger;
using ScanOutputModel;
using Wire;

namespace WarmUpScan
{
    public class Scan
    {
        private readonly string Target;
        private readonly int Timeout;
        private readonly int Concurrency;
        private readonly HttpClient _client;
        public Scan(string target, int timeout, int concurrency)
        {
            Target = target;
            Timeout = timeout;
            Concurrency = concurrency;
            _client = new HttpClient();
        }
        public async Task<ScanOutput> DomainScan(string domain)
        {
            ScanOutput scan = new ScanOutput();
            string subDomainTarget = Target + domain;
            using var cts = new CancellationTokenSource(TimeSpan.FromSeconds(Timeout));
            var sw = Stopwatch.StartNew();
            try
            {
                var result = await _client.GetAsync(subDomainTarget, cts.Token);
                sw.Stop();
                scan.Target = subDomainTarget;
                scan.StatusCode = (int)result.StatusCode;
                scan.Message = result.ReasonPhrase ?? string.Empty;
                scan.Headers = result.Headers.ToDictionary(h => h.Key, h => string.Join(", ", h.Value));
                scan.LatencyMS = sw.ElapsedMilliseconds;
            }
            catch (Exception ex)
            {
                sw.Stop();
                Logger.Error($"Unexpected Error - {ex.Message}");
                scan.Message = ex.Message;
                scan.StatusCode = 0;
                scan.Target = subDomainTarget;
                scan.LatencyMS = sw.ElapsedMilliseconds;
                scan.Headers = new Dictionary<string, string>();
            }
            return scan;
        }
        public async Task<MainScanOutput> RunSequentialSafeScan(string[] wordlists, int delayMs = 500)
        {
            Logger.Scan($"[!] Starting Sequential Scan :- {Target}.....");
            var mainScan = new MainScanOutput();
            var wires = new GlobalWires();
            var _jitterValue = new Random().Next(delayMs, delayMs * 10);
            for (int i = 0; i < wordlists.Length; i++)
            {
                await Task.Delay(delayMs);
                wires.ShowProgress(i, wordlists.Length, wordlists[i]);
                var result = await DomainScan(wordlists[i]);
                if (wires.IsDetected((int)result.StatusCode))
                {
                    Logger.Warn($"Detected at Directory :- {wordlists[i]}");
                    Logger.Info($"Status code :- {(int)result.StatusCode}");
                    Logger.Info($"Delay :- {_jitterValue}");
                    await Task.Delay(_jitterValue);
                    Logger.Scan($"Resuming the scan after wait :- {_jitterValue}ms");
                }
                mainScan.Result.Add(result);
            }
            Logger.Done("Sequential Scan Completed");
            return mainScan;
        }
    }
}