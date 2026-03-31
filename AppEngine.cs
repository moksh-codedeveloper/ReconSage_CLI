using ScanOutputModel;
using System.Text.Json;
using System.Text.Json.Serialization;
using ResoParser;
using RfoModel;
using FirewallAnalysis;
using Analysis;
using FirewallAnalysis.Model;
using RateLimitAnalysis;
using RateLimitDetector.Model;
using IParser;
using Wire;
using WarmUpScan;
using ResoModel;
using TorConfigParser;
using Interface.Network;
using NormalTorScan;
using ControlPortUse;
using ReconSageLogger;
using Proxy_Scan;

namespace AppEngine
{
    public class App
    {
        private GlobalWires wires = new GlobalWires();
        private string Target = string.Empty;
        private int Concurrency;
        private int Timeout;
        private int TorPort;
        private string TorIP = string.Empty;
        private string Host = string.Empty;
        private int CPPort;
        private string Password = string.Empty;
        private string WordlistPath = string.Empty;
        private string JsonFilePath = string.Empty;
        private int Delay;
        public async Task RunScan(string[] args)
        {
            if (args.Length <= 1)
            {
                throw new Exception("Not args have been passed  you should pass a proper args and you should read docs for that...");
            }
            switch (args[0])
            {
                case "--waf-analysis":
                    if(args.Length < 2)
                    {
                        Logger.Error("Args length is too low pass the required fields and data which scanner needs");
                        break;
                    }

                    string wafFilePath = args[1];

                    if (!File.Exists(wafFilePath))
                    {
                        Logger.Error("Files doesn't exist are you fool or what?");
                        break;
                    }

                    IAnalysis<WebFirewallAnalysisOutput> waf = new WafAnalysis();
                    WebFirewallAnalysisOutput result = await waf.RunAnalysis(wafFilePath);
                    WafAnalysis waf1 = new WafAnalysis();
                    waf1.PrintResultInvestigation(result);
                    break;
                case "--rate-limit":
                    if(args.Length < 2)
                    {
                        Logger.Error("Args length is too low pass the required fields and data which scanner needs");
                        break;
                    }
                    string rlFilePath = args[1];
                    if (!File.Exists(rlFilePath))
                    {
                        Logger.Error("Files doesn't exist are you fool or what?");
                        break;
                    }
                    IAnalysis<RateLimitDetectionOutputModel> rateLimit = new RateLimit();
                    RateLimitDetectionOutputModel rate = await rateLimit.RunAnalysis(rlFilePath);
                    new RateLimit().PrintResult(rate);
                    break;
                case "--sequential-scan":
                    if(args.Length < 2)
                    {
                        Logger.Error("Args length is too low pass the required fields and data which scanner needs");
                        break;
                    }

                    IFileParser<RModel> fileParser1 = new RsoParser(args[1]);
                    RModel rsoModel1 = fileParser1.ParseDictToModel();
                    Target = rsoModel1.Target;
                    Concurrency = rsoModel1.Concurrency;
                    Timeout = rsoModel1.Timeout;
                    JsonFilePath = rsoModel1.JsonFilePath;
                    WordlistPath = rsoModel1.WordlistPath;
                    Delay = rsoModel1.Delay;
                    await RunSequentialScan();
                    break;
                case "--tor-normal-scan":
                    if(args.Length < 2)
                    {
                        Logger.Error("Args length is too low pass the required fields and data which scanner needs");
                        break;
                    }
                    IFileParser<RfoParsedModel> RfoModel = new RfoParser(args[1]);
                    RfoParsedModel normalTorParsedModel = RfoModel.ParseDictToModel();
                    Target = normalTorParsedModel.Target;
                    Timeout = normalTorParsedModel.Timeout;
                    JsonFilePath = normalTorParsedModel.JsonFilePath;
                    Host = normalTorParsedModel.host;
                    Password = normalTorParsedModel.Password;
                    TorIP = normalTorParsedModel.tor_ip;
                    TorPort = normalTorParsedModel.tor_port;
                    CPPort = normalTorParsedModel.Port;
                    WordlistPath = normalTorParsedModel.WordlistPath;
                    Delay = normalTorParsedModel.delay;
                    await NormalTorScan();
                    break;
                case "--tls-normal-tor-scan":
                    if(args.Length < 2)
                    {
                        Logger.Error("Args length is too low pass the required fields and data which scanner needs");
                        break;
                    }
                    IFileParser<RfoParsedModel> rfoParsedModel = new RfoParser(args[1]);
                    RfoParsedModel tlsTorScan = rfoParsedModel.ParseDictToModel();
                    Target = tlsTorScan.Target;
                    Timeout = tlsTorScan.Timeout;
                    JsonFilePath = tlsTorScan.JsonFilePath;
                    WordlistPath = tlsTorScan.WordlistPath;
                    Host = tlsTorScan.host;
                    CPPort = tlsTorScan.Port;
                    Password = tlsTorScan.Password;
                    TorIP = tlsTorScan.tor_ip;
                    TorPort = tlsTorScan.tor_port;
                    Delay = tlsTorScan.delay;
                    await TlsNormalTorScan();
                    break;
                case "--control-port-normal-scan":
                    if(args.Length < 2)
                    {
                        Logger.Error("Args length is too low pass the required fields and data which scanner needs");
                        break;
                    }
                    Console.WriteLine("WARNING! Now you are gonna be using the one of most powerful module of this whole tool and its painfully slow so start reading book now if you can");
                    IFileParser<RfoParsedModel> controlPortNormalScan = new RfoParser(args[1]);
                    RfoParsedModel controlParsedModel = controlPortNormalScan.ParseDictToModel();
                    Target = controlParsedModel.Target;
                    Timeout = controlParsedModel.Timeout;
                    Host = controlParsedModel.host;
                    CPPort = controlParsedModel.Port;
                    Password = controlParsedModel.Password;
                    Delay = controlParsedModel.delay;
                    WordlistPath = controlParsedModel.WordlistPath;
                    JsonFilePath = controlParsedModel.JsonFilePath;
                    await ControlPortScan();
                    break;
                case "--control-port-tls-scan":
                    if(args.Length < 2)
                    {
                        Logger.Error("Args length is too low pass the required fields and data which scanner needs");
                        break;
                    }
                    Console.WriteLine("WARNING! Now you are gonna be using the one of most powerful module of this whole tool and its painfully slow so start reading book now if you can");
                    IFileParser<RfoParsedModel> controlPortTlsScan = new RfoParser(args[1]);
                    RfoParsedModel controlTlsParsedModel = controlPortTlsScan.ParseDictToModel();
                    Target = controlTlsParsedModel.Target;
                    Timeout = controlTlsParsedModel.Timeout;
                    Host = controlTlsParsedModel.host;
                    CPPort = controlTlsParsedModel.Port;
                    Password = controlTlsParsedModel.Password;
                    Delay = controlTlsParsedModel.delay;
                    WordlistPath = controlTlsParsedModel.WordlistPath;
                    JsonFilePath = controlTlsParsedModel.JsonFilePath;
                    await ControlPortTlsScan();
                    break;
                case "--proxy-scan":
                    if(args.Length < 2)
                    {
                        Logger.Error("Args length is too low pass the required fields and data which scanner needs");
                        break;
                    }
                    IFileParser<RfoParsedModel>  fileParser = new RfoParser(args[1]);
                    RfoParsedModel proxyScanData = fileParser.ParseDictToModel();
                    Target = proxyScanData.Target;
                    Timeout = proxyScanData.Timeout;
                    var proxyHost = proxyScanData.host;
                    var proxyPort = proxyScanData.Port;
                    JsonFilePath = proxyScanData.JsonFilePath;
                    WordlistPath = proxyScanData.WordlistPath;
                    TorIP = proxyScanData.tor_ip;
                    TorPort  = proxyScanData.tor_port;
                    Delay = proxyScanData.delay;
                    await ProxyScan(ProxyHost:proxyHost, ProxyPort:proxyPort);
                    break;
                default:
                    throw new Exception("Unknown argument type. Use --config-file or --args.");
            }
        }
        public async Task RunSequentialScan()
        {
            var scan = new Scan(Target, Timeout, Concurrency);
            var wires = new GlobalWires();
            var wordlists = await wires.ProcessWordlist(WordlistPath);
            var result = await scan.RunSequentialSafeScan(wordlists, Delay);
            await wires.WriteToJsonAsync(result, JsonFilePath);
        }
        public async Task NormalTorScan()
        {
            Logger.Scan($"Initializing the Normal Tor scan on {Target}.......");
            INetwork normalScan = new TorScan(Target, Timeout, CPPort, TorPort, Password, Host, TorIP, Delay);
            var wordlists = await new GlobalWires().ProcessWordlist(WordlistPath);
            var mainScanOutput = new MainScanOutput();
            var wires = new GlobalWires();
            int total = wordlists.Length;

            for (int i = 0; i < total; i++)
            {
                wires.ShowProgress(i, total, wordlists[i]);
                var result = await normalScan.SendAsync(wordlists[i]);
                mainScanOutput.Result.Add(result);
            }
            Logger.Done("Normal Tor Scan Complete.");
            await wires.WriteToJsonAsync(mainScanOutput, JsonFilePath);
        }
        public async Task TlsNormalTorScan()
        {
            Logger.Scan($"Initializing the Tls Normal Tor Scan on {Target}......");
            ITlsScan tlsScan = new TorScan(Target, Timeout, CPPort, TorPort, Password, Host, TorIP, Delay);
            var wordlists = await new GlobalWires().ProcessWordlist(WordlistPath);
            var mainScanOutput = new MainTorScan();
            var wires = new GlobalWires();
            for (int i = 0; i < wordlists.Length; i++)
            {
                wires.ShowProgress(i, wordlists.Length, wordlists[i]);
                var result = await tlsScan.TlsScan(wordlists[i]);
                mainScanOutput.Results.Add(result);
            }
            Logger.Done("Tls Normal Tor Scan Done.");
            await wires.WriteToJsonAsync(mainScanOutput, JsonFilePath);
        }

        public async Task ControlPortScan()
        {
            Logger.Scan($"Initializing the Control Port Version Tor Scan {Target}.......");
            INetwork normalScan = new ControlPortTorScan(target: Target, timeout: Timeout, host: Host, tor_ip: TorIP, tor_port: TorPort, password: Password, port: CPPort, delay: Delay);
            var wordlists = await new GlobalWires().ProcessWordlist(WordlistPath);
            var mainScanOutput = new MainScanOutput();
            for (int i = 0; i < wordlists.Length; i++)
            {
                wires.ShowProgress(i, wordlists.Length, wordlists[i]);
                var result = await normalScan.SendAsync(wordlists[i]);
                mainScanOutput.Result.Add(result);
            }
            Logger.Done($"Control Port Version of Tor Scan is Done");
            await wires.WriteToJsonAsync(mainScanOutput, JsonFilePath);
        }

        public async Task ControlPortTlsScan()
        {
            Logger.Scan($"Initialising the Tls Scan on Tor Control Port Version Scan {Target}.......");
            ITlsScan tlsScan = new ControlPortTorScan(target: Target, timeout: Timeout, host: Host, tor_ip: TorIP, tor_port: TorPort, password: Password, port: CPPort, delay: Delay);
            var wordlists = await new GlobalWires().ProcessWordlist(WordlistPath);
            var mainScanOutput = new MainTorScan();
            for (int i = 0; i < wordlists.Length; i++)
            {
                var result = await tlsScan.TlsScan(wordlists[i]);
                wires.ShowProgress(i, wordlists.Length, wordlists[i]);
                mainScanOutput.Results.Add(result);
            }
            Logger.Done("Control Port TLS Scan Done.....");
            await wires.WriteToJsonAsync(mainScanOutput, JsonFilePath);
        }
        public async Task ProxyScan(string ProxyHost, int ProxyPort)
        {
            var wordlists = await new GlobalWires().ProcessWordlist(WordlistPath);
            var wires = new GlobalWires();
            INetwork proxyScan = new ProxyScan(Target, Timeout, Delay, ProxyHost, ProxyPort, TorIP, TorPort);
            var mainScan = new MainScanOutput();
            for(int i = 0; i < wordlists.Length; i++)
            {
                var result = await proxyScan.SendAsync(wordlists[i]);
                wires.ShowProgress(i, wordlists.Length, wordlists[i]);
                mainScan.Result.Add(result);
            }
            Logger.Done("[!] Proxy Scan is done lets goooo....");
            await wires.WriteToJsonAsync(mainScan, JsonFilePath);
        }
    }
}