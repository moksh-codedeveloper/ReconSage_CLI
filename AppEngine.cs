using ScanModels.CLIVersion;
using ScanOutputModel;
using System.Text.Json;
using System.Text.Json.Serialization;
using ResoParser;
using FirewallAnalysis;
using Analysis;
using FirewallAnalysis.Model;
using RateLimitAnalysis;
using RateLimitDetector.Model;
using IParser;
using NormalScanCliModel;
using Wire;
using WarmUpScan;
using System.Security;
using StealthStack;
using System.Security.Cryptography.X509Certificates;

namespace AppEngine
{
    public class App
    {
        private string Target = string.Empty;
        private int Concurrency;
        private int Timeout; 
        private int TorPort;
        private string TorIP = string.Empty;
        private string Host = string.Empty;
        private string WordlistPath = string.Empty;
        private string JsonFilePath = string.Empty;
        public async Task RunScan(string[] args)
        {
            if (args.Length <= 1)
            {
                throw new Exception("Not args have been passed  you should pass a proper args and you should read docs for that...");
            }
            switch (args[0])
            {  
                case "--waf-analysis":
                    {
                        if (args.Length == 0)
                        {
                            Console.WriteLine("Error: Please provide a JSON file path.");
                            break;
                        }

                        string wafFilePath = args[1];

                        if (!File.Exists(wafFilePath))
                        {
                            Console.WriteLine($"Error: The file '{wafFilePath}' does not exist.");
                            break;
                        }

                        IAnalysis<WebFirewallAnalysisOutput> waf = new WafAnalysis();
                        WebFirewallAnalysisOutput result = await waf.RunAnalysis(wafFilePath);
                        WafAnalysis waf1 = new WafAnalysis();
                        waf1.PrintResultInvestigation(result);
                        return;
                    }
                case "--rate-limit":
                    {
                        if (args.Length == 0)
                        {
                            Console.WriteLine("ERROR please provided a file a valid");
                            break;
                        }
                        string rlFilePath = args[1];
                        if (!File.Exists(rlFilePath))
                        {
                            Console.WriteLine($"Error the file path in which file is present does not exist {rlFilePath}");
                        }
                        IAnalysis<RateLimitDetectionOutputModel> rateLimit = new RateLimit();
                        RateLimitDetectionOutputModel rate = await rateLimit.RunAnalysis(rlFilePath);
                        new RateLimit().PrintResult(rate);
                        return;
                    }
                case "--brute-force":
                    ICLIParser<NormalScanCliParserModel> parser = new CLIMainEngine();
                    NormalScanCliParserModel cliParserModel = parser.ArgsProcess(args);
                    Target = cliParserModel.Target;
                    Concurrency = cliParserModel.Concurrency;
                    Timeout = cliParserModel.Timeout;
                    WordlistPath = cliParserModel.WordlistPath;
                    JsonFilePath = cliParserModel.JsonFilePath;
                    await RunBruteScan();
                    break;
                case "--sequential-scan":
                    ICLIParser<NormalScanCliParserModel> parser1 = new CLIMainEngine();
                    NormalScanCliParserModel parserModel = parser1.ArgsProcess(args);
                    Target = parserModel.Target;
                    Timeout = parserModel.Timeout;
                    Concurrency = parserModel.Concurrency;
                    JsonFilePath = parserModel.JsonFilePath;
                    WordlistPath = parserModel.WordlistPath;
                    int Delay = parserModel.delay;
                    await RunSequentialScan(Delay);
                    break;
                default:
                    throw new Exception("Unknown argument type. Use --config-file or --args.");
            }
        }
        public async Task WriteToJsonAsync<T>(T data, string filePath)
        {
            var options = new JsonSerializerOptions
            {
                WriteIndented = true,
                DefaultIgnoreCondition = JsonIgnoreCondition.WhenWritingNull
            };

            // Handle duplicate file names
            string directory = Path.GetDirectoryName(filePath);
            string fileName = Path.GetFileNameWithoutExtension(filePath);
            string extension = Path.GetExtension(filePath);

            string newFilePath = filePath;
            int count = 1;

            while (File.Exists(newFilePath))
            {
                newFilePath = Path.Combine(directory, $"{fileName}({count}){extension}");
                count++;
            }

            var json = JsonSerializer.Serialize(data, options);

            await File.WriteAllTextAsync(newFilePath, json);

            Console.WriteLine($"JSON output written to: {newFilePath}");
        }
        public void PrintToConsole(MainScanOutput mainOutput)
        {
            Console.WriteLine("========== ReconSage Scan Results ==========\n");

            foreach (var result in mainOutput.Result)
            {
                Console.WriteLine("--------------------------------------------");
                Console.WriteLine($"Target     : {result.Target}");
                Console.WriteLine($"StatusCode : {result.StatusCode}");
                Console.WriteLine($"Latency    : {result.LatencyMS} ms");
                Console.WriteLine($"Message    : {result.Message}");

                if (result.Headers.Count > 0)
                {
                    Console.WriteLine("Headers:");
                    foreach (var header in result.Headers)
                    {
                        Console.WriteLine($"   {header.Key} : {header.Value}");
                    }
                }
            }
            Console.WriteLine("\n============================================");
        }
        public async Task RunBruteScan()
        {
            GlobalWires wires = new GlobalWires();
            string[] wordlists = await wires.ProcessWordlist(WordlistPath);
            var scan = new Scan(Target, Concurrency, Timeout);
            var result = await scan.RunBruteFastScan(wordlists);
            await WriteToJsonAsync(result, JsonFilePath);
        }
        public async Task RunSequentialScan(int Delay)
        {
            var scan = new Scan(Target, Concurrency, Timeout);
            var wires = new GlobalWires();
            var wordlists = await  wires.ProcessWordlist(WordlistPath);
            var result = await scan.RunSequentialSafeScan(wordlists, Delay);
            await WriteToJsonAsync(result, JsonFilePath);
            PrintToConsole(result);
        }
    }
}