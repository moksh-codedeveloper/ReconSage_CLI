using System.Text.Json;
using Analysis;
using ScanOutputModel;
using FirewallAnalysis.Model;

namespace FirewallAnalysis
{
    public class WafAnalysis : IAnalysis<WebFirewallAnalysisOutput>
    {
        public async Task<MainScanOutput> ReadJson(string jsonFilePath)
        {
            try
            {
                string jsonString = await File.ReadAllTextAsync(jsonFilePath);
                MainScanOutput jsonDeserialised = JsonSerializer.Deserialize<MainScanOutput>(jsonString);
                return jsonDeserialised;
            }
            catch (FileNotFoundException)
            {
                Console.WriteLine($"Error: The file '{jsonFilePath}' was not found.");
                return new MainScanOutput();
            }
            catch (JsonException ex)
            {
                Console.WriteLine($"Error decoding JSON: {ex.Message}");
                return new MainScanOutput();
            }
        }
        public List<int> CodesAnalysis(List<int> statusCode)
        {
            List<int> CommonCodes = new List<int>()
            {
              403, 429, 406, 413, 401, 451, 400, 444, 520
            };
            List<int> DetectedCodes = new();
            foreach (var codes in statusCode)
            {
                if (CommonCodes.Contains(codes))
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

        public int LatencyIncreasing(List<double> latency)
        {
            int increasingCount = 0;
            for(int i = 0; i < latency.Count; i++)
            {
                if(latency[i] < latency[i++])
                {
                    increasingCount++;
                }
            }
            return increasingCount;
        }
        public int LatencyDecreasing(List<double> latency)
        {
            int decreasingCount = 0;
            for(int i = 0; i < latency.Count; i++)
            {
                if(latency[i] > latency[i++])
                {
                    decreasingCount++;
                }
            }
            return decreasingCount;
        }
        public async Task<WebFirewallAnalysisOutput> RunAnalysis(string jsonFilePath)
        {
            MainScanOutput jsonOutput = await ReadJson(jsonFilePath);
            // Values fetching and declaration 
            List<int> CommonCodes = new List<int>()
            {
              403, 429, 406, 413, 401, 451, 400, 444, 520
            };
            var resultWafMode = new WebFirewallAnalysisOutput();
            var StatusCode = jsonOutput.Result.Select(x => x.StatusCode).ToList();
            var LatencyMS = jsonOutput.Result.Select(x => x.LatencyMS).ToList();
            var TargetList = jsonOutput.Result.Select(x => x.Target).ToList();
            var HeadersList = jsonOutput.Result.Select(x => x.Headers).ToList();
            var MessageList = jsonOutput.Result.Select(x => x.Message).ToList();
            // values analysis using the functions 
            List<int> DetectedCodes = CodesAnalysis(StatusCode);
            List<double> Spike = SpikedLatency(latency:LatencyMS);
            int IncreasingLatCount = LatencyIncreasing(LatencyMS);
            int DecreasingLatCount = LatencyDecreasing(LatencyMS);
        // Check for some sus patterns of codes 
            int susCodePatterns = 0;
        
            for(int i = 0; i < StatusCode.Count; i++)
            {
                if(StatusCode[i] ==  200 && CommonCodes.Contains(StatusCode[i++]))
                {
                    susCodePatterns++;
                }
            }
            // Value assigning parts 
            resultWafMode.DetectedStatusCodes = DetectedCodes;
            resultWafMode.LatencyDecreasing = DecreasingLatCount;
            resultWafMode.LatencyInreasing = IncreasingLatCount;
            resultWafMode.Headers = HeadersList;
            resultWafMode.ListOfTarget = TargetList;
            resultWafMode.SpikedLatency = Spike;
            resultWafMode.Message = MessageList;
            resultWafMode.LatencyList = LatencyMS;
            resultWafMode.StatusCodeList = StatusCode;
            resultWafMode.SusCodePattern = susCodePatterns;
            return resultWafMode;
        }
    }
}