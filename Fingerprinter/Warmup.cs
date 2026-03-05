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
        private readonly string JsonFilePath;
        private readonly string WordlistPath;
        private readonly HttpClient _client;
        public Scan(string target, int timeout,int concurrency, string jsonFilePath, string wordlistPath)
        {
            Target = target;
            Timeout = timeout;
            JsonFilePath = jsonFilePath;
            WordlistPath = wordlistPath;
            Concurrency = concurrency;
            _client = new HttpClient();
        }
        // TODO :- ADD Brute force function which brute force in speed and other one will be batch oriented 
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
    }
}