using System.Diagnostics;
using System.Net;
using Interface.Network;
using ReconSageLogger;
using ScanOutputModel;
using StealthStack;
using Wire;

namespace Proxy_Scan
{
    public class ProxyScan : INetwork
    {
        private readonly string Target = string.Empty;
        private readonly int Timeout;
        private readonly int Delay;
        private readonly string ProxyHost = string.Empty;
        private readonly int ProxyPort;
        private readonly string TorIP = string.Empty;
        private readonly int TorPort;
        private HttpClient client;
        public ProxyScan(string target, int timeout, int delay, string proxy_host, int proxy_port, string tor_ip, int tor_port)
        {
            Target = target;
            Timeout = timeout;
            Delay = delay;
            ProxyHost = proxy_host;
            ProxyPort = proxy_port;
            TorIP = tor_ip;
            TorPort = tor_port;
            var handler = new SocketsHttpHandler
            {
                Proxy = new WebProxy(proxy_host, proxy_port),
                UseProxy = true,
                PooledConnectionLifetime = TimeSpan.FromMinutes(3)
            };
            client = new HttpClient(handler);
        }

        private async Task<ScanOutput> SendAsyncInternal(string domain, int retryCount = 0)
        {
            
            Logger.Scan($"[!] Proxy Scan started on Target :- {Target}....");
            if(retryCount >= 5)
            {
                Logger.Error("You have hit a your max retries and i suppose you have exhaust them all");
                return new ScanOutput{Message = "Max Retries hit"};
            }
            var scan = new ScanOutput();
            var domainTarget = Target + domain;
            var request = new HttpRequestMessage(HttpMethod.Get, domainTarget);
            var sw = new Stopwatch();
            var cts = new CancellationTokenSource(TimeSpan.FromSeconds(Timeout));
            var wires = new GlobalWires();
            var random = new Random();
            var _jitterValue = random.Next(Delay, Delay * 10);
            try
            {
                Logger.Info("Headers will be randomised before the scan begins");
                HeaderDisguise.Apply(request);
                sw.Start();
                var result = await client.SendAsync(request, cts.Token);
                sw.Stop();
                if (wires.IsDetected((int)result.StatusCode))
                {
                    Logger.Warn("[!]We have been detected we shall now change the proxy to the tor shall we");
                    client.Dispose();
                    client = wires.BuildClient(proxyHost:TorIP, proxyPort:TorPort);
                    Logger.Info($"Delay - {_jitterValue}");
                    await Task.Delay(_jitterValue);
                    Logger.Scan("[+] Lets restart the scan shall we");
                    return await SendAsyncInternal(domain, retryCount + 1);
                }
                scan.Target = domainTarget;
                scan.StatusCode = (int)result.StatusCode;
                scan.LatencyMS = sw.ElapsedMilliseconds;
                scan.Headers = result.Headers.ToDictionary(h => h.Key, h => string.Join(",", h.Value));
                scan.Message = result.ReasonPhrase ?? string.Empty;
            } catch(HttpRequestException ex)
            {
                Logger.Error(ex.Message);
                scan.Message = ex.Message;
            } catch (TaskCanceledException ex)
            {
                Logger.Error($"{ex.Message} | Guess the task got cancelled");
                scan.Message = ex.Message;
            } catch(Exception ex)
            {
                Logger.Error($"Exception is this :- {ex.Message}");
                scan.Message = ex.Message;
            }
            finally
            {
                if (sw.IsRunning) sw.Stop();
                cts.Dispose();
            }
            return scan;
        }
        public async Task<ScanOutput> SendAsync(string domain)
        {
            return await SendAsyncInternal(domain, 0);
        }
    }
}