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
        public List<double> LatencyIncreasing(List<double> LatencyList)
        {
            List<double> LatIncreasing = new();
            for(int i = 0; i < LatencyList.Count; i++)
            {
                if(LatencyList[i] > LatencyList[i++])
                {
                    LatIncreasing.Add(LatencyList[i]);
                }
            }
            return LatIncreasing;
        }
        public async Task<RateLimitDetectionOutputModel> RunAnalysis(string jsonFilePath)
        {
            RateLimitDetectionOutputModel rateLimit = new RateLimitDetectionOutputModel();
            MainScanOutput output = await ReadJson(jsonFilePath);
            var Target = output.Result.Select(x => x.Target).ToList();
            var StatusCode = output.Result.Select(x => x.StatusCode).ToList();
            var Latency = output.Result.Select(x => x.LatencyMS).ToList();
            return rateLimit;
        }
    }
}