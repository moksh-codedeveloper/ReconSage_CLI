using System.Text.Json;
using Analysis;
using ScanOutputModel;
using FirewallAnalysis.Model;
using Wire;

namespace FirewallAnalysis
{
    public class WafAnalysis : IAnalysis<WebFirewallAnalysisOutput>
    {
        public void PrintResultInvestigation(WebFirewallAnalysisOutput output)
        {
            Console.WriteLine("========== ReconSage Analysis Results ==========\n");
            Console.WriteLine("\n------------------------------------------\n");
            // var targets = output.ListOfTarget;
            foreach(var target in output.ListOfTarget)
            {
                Console.WriteLine($"Targets :- {target}");
            }
            foreach(var codes in output.DetectedStatusCodes)
            {
                Console.WriteLine($"StatusCodes :- {codes}");
            }
            foreach(var otherCodes in output.StatusCodeList)
            {
                Console.WriteLine($"Other Status Codes :- {otherCodes}");
            }
            foreach(var otherLat in output.LatencyList)
            {
                Console.WriteLine($"Latency List :- {otherLat}");
            }
            foreach(var codes in output.DetectedStatusCodes)
            {
                Console.WriteLine($"Detected status codes :- {codes}");
            }
            Console.WriteLine("------- Other details like ---------\n");
            Console.WriteLine($"Latency Decreasing trend    {output.isLatencyDecreasing}");
            Console.WriteLine($"Latency Increasing trend    {output.isLatencyInreasing}");
            Console.WriteLine("------ Here are some messages that you might like to know -----------");
            foreach(var msg in output.Message)
            {
                Console.WriteLine($"Message :- {msg}");
            }
            Console.WriteLine("For headers go ahead and check out your orignal file......");
        }
        public async Task<WebFirewallAnalysisOutput> RunAnalysis(string jsonFilePath)
        {
            GlobalWires wires = new GlobalWires();
            MainScanOutput jsonOutput = await wires.ReadJson(jsonFilePath);
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
            List<int> DetectedCodes = wires.DetectWafStatusCodes(StatusCode);
            List<double> Spike = wires.SpikedLatency(latency:LatencyMS);
            bool isIncreasingLats = wires.isIncreasingLatency(LatencyMS);
            bool isDecreasingLats = wires.isDecreasingLatency(LatencyMS);
            // Value assigning parts 
            resultWafMode.DetectedStatusCodes = DetectedCodes;
            resultWafMode.isLatencyDecreasing = isDecreasingLats;
            resultWafMode.isLatencyInreasing = isIncreasingLats;
            resultWafMode.Headers = HeadersList;
            resultWafMode.ListOfTarget = TargetList;
            resultWafMode.SpikedLatency = Spike;
            resultWafMode.Message = MessageList;
            resultWafMode.LatencyList = LatencyMS;
            resultWafMode.StatusCodeList = StatusCode;
            return resultWafMode;
        }
    }
}