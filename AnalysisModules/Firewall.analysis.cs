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
            foreach(var codes in statusCode)
            {
                if (CommonCodes.Contains(codes))
                {
                    DetectedCodes.Add(codes);
                }
            }
            return DetectedCodes;
        }
        public async Task<WebFirewallAnalysisOutput> RunAnalysis(string jsonFilePath)
        {
            MainScanOutput jsonOutput = await ReadJson(jsonFilePath);
            var resultWafMode = new WebFirewallAnalysisOutput();
            var StatusCode = jsonOutput.Result.Select(x => x.StatusCode).ToList();
            var LatencyMS = jsonOutput.Result.Select(x => x.LatencyMS).ToList();
            var TargetList = jsonOutput.Result.Select(x => x.Target).ToList();
            var HeadersList = jsonOutput.Result.Select(x => x.Headers).ToList();
            List<int> DetectedCodes = CodesAnalysis(StatusCode);

            return resultWafMode;
        }
    }
}