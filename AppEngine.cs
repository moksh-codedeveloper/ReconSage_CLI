using ScannerCore;
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

namespace AppEngine
{
    public class App
    {
        public string Target { set; get; } = string.Empty;
        public int Concurrency { set; get; }
        public int Timeout { set; get; }
        public string JsonFilePath { set; get; } = string.Empty;
        public string WordlistPath { set; get; } = string.Empty;

        public async Task RunScan(string[] args)
        {
            if (args.Length <= 1)
            {
                throw new Exception("Not args have been passed  you should pass a proper args and you should read docs for that...");
            }
            switch (args[0])
            { // --tor-normal-scan, --tor-ttls-scan change some parts of the function and add the warmup scan classes in here and remove the old functions and classes from here and also from the projects and also add the warmup related scan here  
                case "--config-file":
                    string filePath = args[1];
                    if (!filePath.EndsWith(".rso", StringComparison.OrdinalIgnoreCase))
                    {
                        throw new Exception("The file you passed is not acceptable please use the rso not any other one");
                    }
                    Parser parser = new Parser(filePath);
                    Dictionary<string, string> data = parser.Parse();
                    var parserToDict = parser.ParseDictToObject(data);
                    Target = parserToDict.Target;
                    Concurrency = parserToDict.Concurrency;
                    Timeout = parserToDict.Timeout;
                    JsonFilePath = parserToDict.JsonFilePath;
                    WordlistPath = parserToDict.WordlistPath;
                    break;
                case "--args":
                    var cliEngine = new CLIMainEngine().ProcessCLiArgs(args);
                    Target = cliEngine.Target;
                    Concurrency = cliEngine.Concurrency;
                    Timeout = cliEngine.Timeout;
                    JsonFilePath = cliEngine.JsonFilePath;
                    WordlistPath = cliEngine.WordlistPath;
                    break;
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
                        if(args.Length == 0)
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
                default:
                    throw new Exception("Unknown argument type. Use --config-file or --args.");
            }
            Scanner scanner = new Scanner(Target, Concurrency, Timeout, WordlistPath);
            MainScanOutput mainScan = await scanner.ExecuteScan();
            PrintToConsole(mainScan);
            await WriteToJsonAsync(mainScan, JsonFilePath);
        }
        public async Task WriteToJsonAsync(MainScanOutput mainOutput, string filePath)
        {
            var options = new JsonSerializerOptions
            {
                WriteIndented = true,
                DefaultIgnoreCondition = JsonIgnoreCondition.WhenWritingNull
            };

            var json = JsonSerializer.Serialize(mainOutput, options);

            await File.WriteAllTextAsync(filePath, json);

            Console.WriteLine($"JSON output written to: {filePath}");
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
    }
}