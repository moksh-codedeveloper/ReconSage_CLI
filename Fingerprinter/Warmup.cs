using System.Diagnostics;
using ScanOutputModel;
using Wire;

namespace WarmUpScan
{
    public class Scan
    {
        private readonly string Target;
        private readonly int Timeout;
        private readonly int Concurrency;

        private readonly string WordlistPath;
        private readonly HttpClient _client;
        public Scan(string target, int timeout, int concurrency, string wordlistPath)
        {
            Target = target;
            Timeout = timeout;
            WordlistPath = wordlistPath;
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
            catch (System.Exception ex)
            {
                sw.Stop();
                scan.Message = ex.Message;
                scan.StatusCode = 0;
                scan.Target = subDomainTarget;
                scan.LatencyMS = sw.ElapsedMilliseconds;
                scan.Headers = new Dictionary<string, string>();
            }
            return scan;
        }
        public async Task<MainScanOutput> RunBruteFastScan()
        {
            object _resultLock = new object();
            MainScanOutput scanOutput = new MainScanOutput();
            using var semaphore = new SemaphoreSlim(Concurrency);
            GlobalWires wires = new GlobalWires();
            string[] wordlists = await wires.ProcessWordlist(WordlistPath);
            var tasks = wordlists.Select(async domain =>
            {
                await semaphore.WaitAsync();
                try
                {
                    var result = await DomainScan(domain);
                    lock (_resultLock)
                    {
                        scanOutput.Result.Add(result);
                    }
                }
                finally
                {
                    semaphore.Release();
                }
            });
            await Task.WhenAll(tasks);
            return scanOutput;
        }

        public async Task<MainScanOutput> RunSmartScan(string[] wordlistToUse)
        {
            using var semaphore = new SemaphoreSlim(Concurrency);
            object _resultLock = new object();
            MainScanOutput mainScan = new MainScanOutput();
            var tasks = wordlistToUse.Select(async domain =>
            {
                await semaphore.WaitAsync();
                try
                {
                    var result = await DomainScan(domain);
                    lock (_resultLock) { mainScan.Result.Add(result); }
                }
                finally
                {
                    semaphore.Release();
                }
            });
            await Task.WhenAll(tasks);
            return mainScan;
        }
        public async Task<MainScanOutput> RunSequentialSafeScan(string[] wordlists, int delayMs = 500)
        {
            MainScanOutput mainScan = new MainScanOutput();
            GlobalWires wires = new GlobalWires();

            Console.WriteLine($"[!] Starting Sequential Scan. Delay: {delayMs}ms");

            foreach (var domain in wordlists)
            {
                var result = await DomainScan(domain);
                if (wires.IsDetected(result.StatusCode))
                {
                    Console.WriteLine($"[!] Detection Triggered ({result.StatusCode}) at: {domain}");
                    Console.WriteLine("[!] Pausing scan for 30 seconds to cool down...");
                    await Task.Delay(30000); 
                }
                mainScan.Result.Add(result);

                int jitter = new Random().Next(-100, 100);
                await Task.Delay(Math.Max(50, delayMs + jitter));

                Console.WriteLine($"[+] Scanned: {domain} | Status: {result.StatusCode}");
            }
            return mainScan;
        }
    }
}