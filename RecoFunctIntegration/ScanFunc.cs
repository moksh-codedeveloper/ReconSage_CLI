using HttpScan;
using Interface.Network;
using NormalScan;
using ScanOutputModel;
using SocksProxyModule;
using TorScan;
using Wire;
using ResponseBodyStruct;
using Capture;
using System.Net;

namespace AllScansInOne
{
    public static class AllScans
    {
        public static GlobalWires wires = new GlobalWires();
        public static async Task ExecTorScan(string jsonFilePath, string wordlistPath, string target, string Password, string TorIp,  string Port, int TorPort, int CpTorPort, int Timeout, int Delay, CancellationTokenSource cts)
        {
            var scanOutput = new  MainScanOutput();
            INetwork torScan = new MainTorScan(target, Port, Password, TorIp, TorPort, CpTorPort, Timeout, Delay);
            var wordlists  = await wires.ProcessWordlist(wordlistPath);
            for(int i = 0; i < wordlists.Length; i++)
            {
                wires.ShowProgress(i, wordlists.Length, wordlists[i]);
                var result = await torScan.SendAsync(wordlists[i], cts.Token);
                scanOutput.Result.Add(result);
            }
            await wires.WriteToJsonAsync<MainScanOutput>(scanOutput, jsonFilePath);
        }
        public static async Task ExecCppScan(string target, string port, string headers, int timeout, int delay, string jsonFilePath, string wordlistPath, CancellationTokenSource cts, string DNSServer)
        {
            var mainScan = new MainScanOutput();
            INetwork cppScan = new CppScan(target, timeout, delay, port, headers, DNSServer);
            var wordlists = await wires.ProcessWordlist(wordlistPath);
            for(int i = 0; i < wordlists.Length; i++)
            {
                wires.ShowProgress(i, wordlists.Length, wordlists[i]);
                var result = await cppScan.SendAsync(wordlists[i], cts.Token);
                mainScan.Result.Add(result);
            }
            await wires.WriteToJsonAsync<MainScanOutput>(mainScan, jsonFilePath);
        }

        public static async Task ExecHttpProxy(string target, string proto_port, string proxy_host, string headers, string jsonFilePath, string wordlistPath, int timeout, int delay, int proxy_port, CancellationTokenSource cts)
        {
            var mainScan = new MainScanOutput();
            INetwork httpProxy = new HttpProxyScan(target, proto_port, proxy_host, headers, proxy_port, timeout, delay);
            var wordlists = await new GlobalWires().ProcessWordlist(wordlistPath);
            for(int it = 0; it < wordlists.Length; it++)
            {
                new GlobalWires().ShowProgress(it, wordlists.Length, wordlists[it]);
                var result = await httpProxy.SendAsync(wordlists[it], cts.Token);
                mainScan.Result.Add(result);
            }
            await new GlobalWires().WriteToJsonAsync<MainScanOutput>(mainScan, jsonFilePath);
        }

        public static async Task ExecSockProxy(string target, string proto_port, string proxy_host, string headers, string jsonFilePath, string wordlistPath, int timeout, int proxy_port, CancellationTokenSource cts, int delay)
        {
            var mainScan = new MainScanOutput();
            INetwork socksProxy = new SocksScan(target, proxy_host, proto_port, headers, timeout, proxy_port, delay);
            var wordlists = await new GlobalWires().ProcessWordlist(wordlistPath);
            for(int it = 0; it < wordlists.Length; it++)
            {
                new GlobalWires().ShowProgress(it, wordlists.Length, wordlists[it]);
                var result = await socksProxy.SendAsync(wordlists[it], cts.Token);
                mainScan.Result.Add(result);
            }
            await new GlobalWires().WriteToJsonAsync<MainScanOutput>(mainScan, jsonFilePath);
        }

        public static async Task ExecCaptureScan(string target, string proto_port, string proxy_host, string jsonFilePath, string wordlistPath, int timeout, int proxy_port, CancellationTokenSource cts, string dns_server)
        {
            var mainScan = new MainScanResponseBodyModel();
            var capture = new CaptureScan(target, proto_port, proxy_host, dns_server, proxy_port, timeout);
            var wordlists = await new GlobalWires().ProcessWordlist(wordlistPath);
            for(int i = 0; i < wordlists.Length; i++)
            {
                new GlobalWires().ShowProgress(i, wordlists.Length, wordlists[i]);
                var result = capture.Scan(wordlists[i], cts.Token);
                mainScan.Result.Add(result);
            }
            await new GlobalWires().WriteToJsonAsync<MainScanResponseBodyModel>(mainScan, jsonFilePath);
        }
    }
}