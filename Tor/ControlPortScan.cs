using System.Diagnostics;
using System.Net.Security;
using System.Security.Cryptography.X509Certificates;
using Interface.Network;
using ITor;
using MihaZupan;
using ScanOutputModel;
using StealthStack;
using TorRotator;
using Wire;
namespace ControlPortUse
{
    public class ControlPortTorScan : INetwork, ITlsScan
    {
        private readonly string Target;
        private readonly int Timeout;
        private readonly int Port;
        private readonly string Password;
        private readonly string Host;
        private readonly string TorIP;
        private readonly int TorPort;
        private readonly int Delay;

        public ControlPortTorScan(string target, int timeout, string host, int port, string password, string tor_ip, int tor_port, int delay)
        {
            Target = target;
            Timeout = timeout;
            Host = host;
            Password = password;
            Port = port;
            TorIP = tor_ip;
            TorPort = tor_port;
            Delay = delay;
        }
        public async Task<ScanOutput> SendAsync(string domain)
        {
            var scanOutput = new ScanOutput();
            var handler = new SocketsHttpHandler
            {
                Proxy = new HttpToSocks5Proxy(TorIP, TorPort),
                UseProxy = true
            };
            var client = new HttpClient(handler);
            var targetDomain = Target + domain;
            var request = new HttpRequestMessage(HttpMethod.Get, targetDomain);
            var sw = new Stopwatch();
            ITorController tor = new TorRotate(host: Host, port: Port, password: Password);
            var _jitterMinValue = new Random().Next(Delay, Delay * 5);
            var wires = new GlobalWires();
            try
            {
                sw.Start();
                var result = await client.SendAsync(request);
                sw.Stop();
                if (wires.IsDetected((int)result.StatusCode))
                {
                    HeaderDisguise.Apply(request);
                    await tor.RotateAsync();
                    await Task.Delay(_jitterMinValue);
                }
                scanOutput.Target = targetDomain;
                scanOutput.StatusCode = (int)result.StatusCode;
                scanOutput.Message = result.ReasonPhrase ?? string.Empty;
                scanOutput.Headers = result.Headers.ToDictionary(h => h.Key, h => string.Join(",", h.Value));
                scanOutput.LatencyMS = sw.ElapsedMilliseconds;
            }
            catch (Exception ex)
            {
                scanOutput.Target = targetDomain;
                scanOutput.Message = ex.Message;
                scanOutput.LatencyMS = sw.ElapsedMilliseconds;
            }
            finally
            {
                if (sw.IsRunning) sw.Stop();
            }
            return scanOutput;
        }

        public async Task<TlsScanResult> TlsScan(string domain)
        {
            var tlsScanModel = new TlsScanResult();
            var targetDomain = Target + domain;
            var handler = new SocketsHttpHandler
            {
                Proxy = new HttpToSocks5Proxy(TorIP, TorPort),
                UseProxy = true,
                SslOptions = new SslClientAuthenticationOptions
                {
                    RemoteCertificateValidationCallback = (sender, cert, chains, error) =>
                    {
                        if (sender is SslStream sslStream)
                        {
                            tlsScanModel.Protocol = sslStream.SslProtocol.ToString();
                            tlsScanModel.CipherSuite = sslStream.NegotiatedCipherSuite.ToString();
                        }
                        if (cert is X509Certificate2 cert2)
                        {
                            tlsScanModel.CertSubject = cert2.Subject;
                            tlsScanModel.CertIssuer = cert2.Issuer;
                            tlsScanModel.CertThumbprint = cert2.Thumbprint;
                            tlsScanModel.CertSerialNumber = cert2.SerialNumber;
                            tlsScanModel.CertNotBefore = cert2.NotBefore;
                            tlsScanModel.CertNotAfter = cert2.NotAfter;
                            tlsScanModel.RawCertificateBase64 = Convert.ToBase64String(cert2.RawData);
                            var sanExtension = cert2.Extensions["2.5.29.17"];
                            if (sanExtension != null)
                            {
                                tlsScanModel.SubjectAlternativeNames = sanExtension.Format(false)
                                    .Split(new[] { ',', ';' }, StringSplitOptions.RemoveEmptyEntries)
                                    .Select(s => s.Trim().Replace("DNS Name=", ""))
                                    .ToList();
                            }
                        }
                        return true;
                    }
                }
            };
            var client = new HttpClient(handler);
            var sw = new Stopwatch();
            var request = new HttpRequestMessage(HttpMethod.Get, targetDomain);
            var cts = new CancellationTokenSource(TimeSpan.FromSeconds(Timeout));
            var wires = new GlobalWires();
            ITorController tor = new TorRotate(host:Host, port:Port, password:Password);
            var _jitterMinValue = new Random().Next(Delay, Delay * 5);
            try
            {
                sw.Start();
                var result = await client.SendAsync(request, cts.Token);
                sw.Stop();
                if (wires.IsDetected((int)result.StatusCode))
                {
                    await tor.RotateAsync(cts.Token);
                    await Task.Delay(_jitterMinValue);
                    HeaderDisguise.Apply(request);
                }
                tlsScanModel.Target = targetDomain;
                tlsScanModel.LatencyMS = sw.ElapsedMilliseconds;
                tlsScanModel.Message = result.ReasonPhrase ?? string.Empty;
                tlsScanModel.TlsVersion = result.Version.ToString();
            }
            catch (Exception ex)
            {
                tlsScanModel.Message = ex.Message;
                tlsScanModel.LatencyMS = sw.ElapsedMilliseconds;
                tlsScanModel.Target = targetDomain;
            }
            finally
            {
                if(sw.IsRunning)
                {
                    sw.Stop();
                }
            }
            return tlsScanModel;
        }
    }
}