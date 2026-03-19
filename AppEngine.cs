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
using IParser;
using NormalScanCliModel;

namespace AppEngine
{
    public class App
    {
        public string Target { set; get; } = string.Empty;
        public int Concurrency { set; get; }
        public int Timeout { set; get; }
        public string JsonFilePath { set; get; } = string.Empty;
        public string WordlistPath { set; get; } = string.Empty;
        public string Host{set;get;} = string.Empty;
        public  int Port{set;get;}
        public string Password{set;get;} = string.Empty;
        public  string Tor_IP{set;get;} = string.Empty;
        public int Tor_Port{set;get;}
        public async Task RunScan(string[] args)
        {
            if (args.Length <= 1)
            {
                throw new Exception("Not args have been passed  you should pass a proper args and you should read docs for that...");
            }
            switch (args[0])
            { // --tor-normal-scan, --tor-ttls-scan change some parts of the function and add the warmup scan classes in here and remove the old functions and classes from here and also from the projects and also add the warmup related scan here  
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
                case "--normal-scan":
                    ICLIParser<NormalScanCliParserModel> parser = new CLIMainEngine();
                    NormalScanCliParserModel cliParserModel = parser.ArgsProcess(args);

                    break;
                default:
                    throw new Exception("Unknown argument type. Use --config-file or --args.");
            }
        }
        
        // Writing to Json and Printing the output section for normal scan 
        // TODO 1: Convert the WriteToJsonAsync function to accept the any models and write json for any model
        // TODO 2 : Write down code of 4 scanning function :- 1. All Normal Scans and all the warmup scans functions class from the warmup scan 2. Tor Normal scan 3. TorTlsScan 4. add batch processing and a condition of break when the scans goes wrong
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