using System.Runtime.CompilerServices;
using System.Text.Json;
using Analysis;
using RateLimitDetector.Model;
using ScanOutputModel;
namespace RateLimitAnalysis
{
    public class RateLimit : IAnalysis<RateLimitDetectionOutputModel>
    {
        private async Task<MainScanOutput> ReadJson(string jsonFilePath)
        {
            try
            {
                string jsonString = await File.ReadAllTextAsync(jsonFilePath);
                MainScanOutput jsonOutput = JsonSerializer.Deserialize<MainScanOutput>(jsonString) ?? new MainScanOutput();
                return jsonOutput;
            }
            catch (FileNotFoundException)
            {
                Console.WriteLine($"Error: The file '{jsonFilePath}' was not found");
                return new MainScanOutput();
            }
            catch (JsonException ex)
            {
                Console.WriteLine($"Error decoding JSON: {ex.Message}");
                return new MainScanOutput();
            }
        }
        public List<int> DetectStatusCodes(List<int> statusCode)
        {
            List<int> DetectedCodes = new();
            List<int> RLCodes = new() { 429, 403, 402, 420, 430, 503, 507, 509 };
            foreach(var codes in statusCode)
            {
                if (RLCodes.Contains(codes))
                {
                    DetectedCodes.Add(codes);
                }
            }
            return DetectedCodes;
        }
        public List<double> SpikedLatency(List<double> latency)
        {
            if (latency == null || latency.Count < 10) return new List<double>();

            double average = latency.Average();
            double sumOfSquare = latency.Select(val => Math.Pow(val - average, 2)).Sum();

            // Use N-1 (latency.Count - 1) for better accuracy on smaller lists (Bessel's correction)
            double stDeviation = Math.Sqrt(sumOfSquare / (latency.Count - 1));
            double thresholds = average + (3 * stDeviation);

            return latency.Where(l => l > thresholds).ToList();
        }
        public  bool isIncreasingLatency(List<double> LatencyList)
        {
            for(int i = 0; i  <  LatencyList.Count; i++)
            {
                if(LatencyList[i] < LatencyList[i++])
                {
                    return true;
                }
            }
            return false;
        }
        public  bool isDecreasingLatency(List<double> LatencyList)
        {
            for(int i = 0; i  <  LatencyList.Count; i++)
            {
                if(LatencyList[i] > LatencyList[i++])
                {
                    return true;
                }
            }
            return false;
        }
        public async Task<RateLimitDetectionOutputModel> RunAnalysis(string jsonFilePath)
        {
            RateLimitDetectionOutputModel rateLimit = new RateLimitDetectionOutputModel();
            MainScanOutput output = await ReadJson(jsonFilePath);
            var StatusCode = output.Result.Select(x => x.StatusCode).ToList();
            var Latency = output.Result.Select(x => x.LatencyMS).ToList();
            rateLimit.Target = output.Result.Select(x => x.Target).ToList();
            rateLimit.StatusCode = StatusCode;
            rateLimit.LatencyMS = Latency;
            rateLimit.DetectedStatusCodeList = DetectStatusCodes(StatusCode);
            rateLimit.SpikedLatencyMS = SpikedLatency(Latency);
            rateLimit.isDecreasingLat = isDecreasingLatency(Latency);
            rateLimit.isIncreasingLat = isIncreasingLatency(Latency);
            return rateLimit;
        }
        public void PrintResult(RateLimitDetectionOutputModel rateLimit)
        {
            Console.WriteLine("---------------------------------------------------");
            Console.WriteLine("====== Analysis of json report you submitted ======");
            Console.WriteLine("----- Main Values -----");
            Console.WriteLine("Targets :-");
            foreach(var target in rateLimit.Target)
            {
                Console.WriteLine($"     {target}");
            }
            Console.WriteLine("All Status Codes :-");
            foreach(var codes in rateLimit.StatusCode)
            {
                Console.WriteLine($"    {codes}");
            }
            Console.WriteLine("All Latency data :-");
            foreach(var lat in rateLimit.LatencyMS)
            {
                Console.WriteLine($"     {lat}");
            }
            Console.WriteLine("Spiked Latencies :-");
            foreach(var spikedLat in rateLimit.SpikedLatencyMS)
            {
                Console.WriteLine($"     {spikedLat}");
            }
            Console.WriteLine("---- Other details ----");
            Console.WriteLine($"Increasing trend ? :- {rateLimit.isIncreasingLat}");
            Console.WriteLine($"Decreasing trend ? :- {rateLimit.isDecreasingLat}");
            Console.WriteLine("---------------------------------------------------");
        }
    }
}