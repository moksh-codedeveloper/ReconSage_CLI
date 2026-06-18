using HttpScan;
using Interface.Network;
using NormalScan;
using ScanOutputModel;
using TorScan;
using Wire;

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
        public static async Task ExecCppScan(string target, string port, int timeout, int delay, string jsonFilePath, string wordlistPath, CancellationTokenSource cts)
        {
            var mainScan = new MainScanOutput();
            INetwork cppScan = new CppScan(target, timeout, delay, port);
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
    }
}