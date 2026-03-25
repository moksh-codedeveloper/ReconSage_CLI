using System.Diagnostics;
using Interface.Network;
using MihaZupan;
using ScanOutputModel;

namespace NormalTorScan
{
    public class TorScan : INetwork
    {
        private string Target = string.Empty;
        private int Timeout;
        private int TorPort;
        private string TorIP = string.Empty;
        private string Host = string.Empty;
        private string Password = string.Empty;
        private int CPPort;
        public TorScan(string target, int timeout, int port, int tor_port, string password, string host, string tor_ip)
        {
            Target = target;
            Timeout = timeout;
            CPPort = port;
            Host = host;
            Password = password;
            TorIP = tor_ip;
            TorPort = tor_port;
        }
        public async Task<ScanOutput> SendAsync(string Domain)
        {
            var scan = new ScanOutput();
            var proxy = new HttpToSocks5Proxy(TorIP, TorPort);
            var handler = new SocketsHttpHandler
            {
                Proxy = proxy,
                UseProxy = true
            };
            var client = new HttpClient(handler);
            var targetedDomain = Target + Domain;
            var request = new HttpRequestMessage(HttpMethod.Get, targetedDomain);
            var sw = new Stopwatch();
            var cts = new CancellationTokenSource(TimeSpan.FromSeconds(Timeout));
            try
            {
                sw.Start();
                var result = await client.SendAsync(request, cts.Token);
                sw.Stop();
                scan.Target = targetedDomain;
                scan.StatusCode = (int)result.StatusCode;
                scan.Headers = result.Headers.ToDictionary(h => h.Key, h => string.Join(",", h.Value));
                scan.LatencyMS = sw.ElapsedMilliseconds;
            }catch(HttpRequestException ex)
            {
                scan.Message = ex.Message;
                scan.LatencyMS = sw.ElapsedMilliseconds;
            }
            catch(Exception ex)
            {
                scan.Message = ex.Message;
                scan.LatencyMS = sw.ElapsedMilliseconds;
            }
            finally
            {
                if (sw.IsRunning) sw.Stop();
            }
            return scan;
        }
    }
}