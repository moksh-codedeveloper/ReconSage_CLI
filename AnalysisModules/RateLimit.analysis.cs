using System.Runtime.CompilerServices;
using System.Text.Json;
using Analysis;
using RateLimitDetector.Model;
using ScanOutputModel;
using Wire;
namespace RateLimitAnalysis
{
    public class RateLimit : IAnalysis<RateLimitDetectionOutputModel>
    {
        public async Task<RateLimitDetectionOutputModel> RunAnalysis(string jsonFilePath)
        {
            GlobalWires wires = new GlobalWires();
            RateLimitDetectionOutputModel rateLimit = new RateLimitDetectionOutputModel();
            MainScanOutput output = await wires.ReadJson(jsonFilePath);
            var StatusCode = output.Result.Select(x => x.StatusCode).ToList();
            var Latency = output.Result.Select(x => x.LatencyMS).ToList();
            rateLimit.Target = output.Result.Select(x => x.Target).ToList();
            rateLimit.StatusCode = StatusCode;
            rateLimit.LatencyMS = Latency;
            rateLimit.DetectedStatusCodeList = wires.DetectRLStatusCodes(StatusCode);
            rateLimit.SpikedLatencyMS = wires.SpikedLatency(Latency);
            rateLimit.isDecreasingLat = wires.isDecreasingLatency(Latency);
            rateLimit.isIncreasingLat = wires.isIncreasingLatency(Latency);
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