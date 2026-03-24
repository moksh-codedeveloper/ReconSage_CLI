using System.Diagnostics;
using System.Net.Security;
using MihaZupan;
using ScanOutputModel;
using Wire;
using System.Security.Cryptography.X509Certificates;

namespace StealthStack
{
    public static class HeaderDisguise
    {
        private static readonly Random _rnd = new Random();

        private static readonly string[] UserAgents = {
        "Mozilla/5.0 (X11; Arch Linux; Linux x86_64; rv:124.0) Gecko/20100101 Firefox/124.0",
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/122.0.0.0 Safari/537.36",
        "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/122.0.0.0 Safari/537.36",
        "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/121.0.0.0 Safari/537.36"
    };

        public static void Apply(HttpRequestMessage request)
        {
            // Set the User-Agent
            request.Headers.UserAgent.ParseAdd(UserAgents[_rnd.Next(UserAgents.Length)]);

            // Add the "Sec-Ch-Ua" headers (Modern Browser Fingerprinting)
            request.Headers.Add("sec-ch-ua", "\"Chromium\";v=\"122\", \"Not(A:Brand\";v=\"24\", \"Google Chrome\";v=\"122\"");
            request.Headers.Add("sec-ch-ua-mobile", "?0");
            request.Headers.Add("sec-ch-ua-platform", "\"Linux\"");

            // Essential for bypassing basic bot filters
            request.Headers.Add("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8");
            request.Headers.Add("Accept-Language", "en-US,en;q=0.5");
            request.Headers.Add("Sec-Fetch-Dest", "document");
            request.Headers.Add("Sec-Fetch-Mode", "navigate");
            request.Headers.Add("Sec-Fetch-Site", "none");
        }
    }
    // TODO :- add the tor scanning and integrated the tor rotation interface here and add the actual tor scanner and integrate proxy layer 
    public class StealthEngine
    {
        public readonly string Target;
        private readonly int Timeout;
        private readonly string host;
        private readonly int port;
        private readonly string password;
        private readonly string tor_ip;
        private readonly int tor_port;
        private readonly Random _jitter = new();
        private readonly int minJitterValue;
        public StealthEngine(string target, int timeout, string _host, int _port, string _password, string _tor_host, int _tor_port, int MinJitterValue)
        {
            Target = target;
            Timeout = timeout;
            host = _host;
            port = _port;
            password = _password;
            tor_ip = _tor_host;
            tor_port = _tor_port;
            minJitterValue = MinJitterValue;
        }

        public async Task<ScanOutput> NormalTorScan(HttpRequestMessage request)
        {
            var scan = new ScanOutput();
            using var handler = new SocketsHttpHandler
            {
                Proxy = new HttpToSocks5Proxy(tor_ip, tor_port),
                UseProxy = true,
            };
            var client = new HttpClient(handler);
            var wires = new GlobalWires();
            var sw = Stopwatch.StartNew();
            using var cts = new CancellationTokenSource(TimeSpan.FromSeconds(Timeout));
            try
            {
                await Task.Delay(_jitter.Next(minJitterValue, minJitterValue * 10));
                var result = await client.SendAsync(request, cts.Token);
                sw.Stop();
                scan.StatusCode = (int)result.StatusCode;
                scan.Headers = result.Headers.ToDictionary(h => h.Key, h => string.Join(",", h.Value));
                scan.LatencyMS = sw.ElapsedMilliseconds;
                scan.Message = $"{result.Version} | {result.ReasonPhrase}";
                scan.Target = request.RequestUri?.ToString() ?? string.Empty;
            }
            catch (Exception ex)
            {
                sw.Stop();
                scan.LatencyMS = sw.ElapsedMilliseconds;
                scan.Headers = new Dictionary<string, string>();
                scan.StatusCode = 0;
                scan.Message = ex.Message;
            }
            return scan;
        }

        public async Task<MainScanOutput> ExecuteScan(string[] wordlists)
        {
            MainScanOutput main = new MainScanOutput();
            var wires = new GlobalWires();
            foreach (var words in wordlists)
            {
                var request = new HttpRequestMessage(HttpMethod.Get, words);
                var result = await NormalTorScan(request);
                if (result.LatencyMS >= 1000)
                {
                    Console.WriteLine("Seems like the server is trying to increase the latency so lets go slowlyy");
                    await Task.Delay(_jitter.Next(minJitterValue, minJitterValue * 5));
                }
                if (wires.IsDetected(result.StatusCode))
                {
                    HeaderDisguise.Apply(request);
                }
                main.Result.Add(result);
            }
            return main;
        }

        public async Task<TlsScanResult> TlsRelatedScan(HttpRequestMessage request)
        {
            var currentResult = new TlsScanResult();
            var sw = new Stopwatch();
            var wires = new GlobalWires();

            // 1. Move Handler/Client into using blocks or a shared factory to prevent socket exhaustion
            using var handler = new SocketsHttpHandler
            {
                Proxy = new HttpToSocks5Proxy(tor_ip, tor_port),
                UseProxy = true,
                SslOptions = new SslClientAuthenticationOptions
                {
                    RemoteCertificateValidationCallback = (sender, certificate, chain, errors) =>
                    {
                        if (sender is SslStream sslStream)
                        {
                            currentResult.TlsVersion = sslStream.SslProtocol.ToString();
                            currentResult.CipherSuite = sslStream.NegotiatedCipherSuite.ToString();
                        }

                        if (certificate is X509Certificate2 cert2)
                        {
                            currentResult.CertSubject = cert2.Subject;
                            currentResult.CertIssuer = cert2.Issuer;
                            currentResult.CertThumbprint = cert2.Thumbprint;
                            currentResult.CertSerialNumber = cert2.SerialNumber;
                            currentResult.CertNotBefore = cert2.NotBefore;
                            currentResult.CertNotAfter = cert2.NotAfter;
                            currentResult.RawCertificateBase64 = Convert.ToBase64String(cert2.RawData);

                            var sanExtension = cert2.Extensions["2.5.29.17"];
                            if (sanExtension != null)
                            {
                                currentResult.SubjectAlternativeNames = sanExtension.Format(false)
                                    .Split(new[] { ',', ';' }, StringSplitOptions.RemoveEmptyEntries)
                                    .Select(s => s.Trim().Replace("DNS Name=", ""))
                                    .ToList();
                            }
                        }
                        return true;
                    }
                }
            };

            using var client = new HttpClient(handler);

            try
            {
                await Task.Delay(minJitterValue); // Your custom lead-in jitter

                using var cts = new CancellationTokenSource(TimeSpan.FromSeconds(Timeout));

                sw.Start();
                var result = await client.SendAsync(request, cts.Token);
                sw.Stop();
                currentResult.Target = request.RequestUri?.ToString() ?? string.Empty;
                currentResult.LatencyMS = sw.ElapsedMilliseconds;
                currentResult.StatusCode = (int)result.StatusCode;
                currentResult.Message = $"{result.ReasonPhrase} | {result.Version}";

                // Your "Plan B" / Spotted logic
                if (wires.IsDetected(currentResult.StatusCode))
                {
                    Console.WriteLine("SPOTTED!!! Server has spotted us lets hide.......");
                    var jitterValue = _jitter.Next(minJitterValue, minJitterValue * 10);
                    await Task.Delay(jitterValue);
                }
            }
            catch (OperationCanceledException)
            {
                // Specific handling for Timeouts (common with Tor)
                currentResult.StatusCode = 408;
                currentResult.Message = "Request Timed Out (Tor Circuit Slow?)";
            }
            catch (HttpRequestException ex) when (ex.InnerException is System.Security.Authentication.AuthenticationException)
            {
                // Specific handling for TLS Handshake failures
                currentResult.StatusCode = 0;
                currentResult.Message = $"TLS Handshake Failed: {ex.Message}";
            }
            catch (Exception ex)
            {
                // Catch-all for everything else
                currentResult.StatusCode = -1;
                currentResult.Message = $"Scanner Error: {ex.GetType().Name} - {ex.Message}";
            }
            finally
            {
                if (sw.IsRunning) sw.Stop();
            }

            return currentResult;
        }
        public async Task<MainTorScan> ExecuteTorTlsScan(string[] wordlists)
        {
            if(wordlists.Count() >= 10)
            {
                return new MainTorScan();
            }
            var torMainScanOutput = new MainTorScan();
            foreach(var words in wordlists)
            {
                using var request = new HttpRequestMessage(HttpMethod.Get, words);
                var result = await TlsRelatedScan(request);
                torMainScanOutput.Results.Add(result);
            }
            return torMainScanOutput;
        }
    }
}