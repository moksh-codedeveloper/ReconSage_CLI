using System.Diagnostics;
using System.Net.Security;
using System.Security.Cryptography.X509Certificates;
using Interface.Network;
using MihaZupan;
using ReconSageLogger;
using ScanOutputModel;
using StealthStack;
using Wire;

namespace NormalTorScan
{
    public class TorScan : INetwork, ITlsScan
    {
        private string Target = string.Empty;
        private int Timeout;
        private int TorPort;
        private string TorIP = string.Empty;
        private string Host = string.Empty;
        private string Password = string.Empty;
        private int CPPort;
        private readonly int Delay;
        public TorScan(string target, int timeout, int port, int tor_port, string password, string host, string tor_ip, int delay)
        {
            Target = target;
            Timeout = timeout;
            CPPort = port;
            Host = host;
            Password = password;
            TorIP = tor_ip;
            TorPort = tor_port;
            Delay = delay;
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
            var wires = new GlobalWires();
            var cts = new CancellationTokenSource(TimeSpan.FromSeconds(Timeout));
            Random  random = new();
            var _jitterValue = random.Next(Delay, Delay * 20);
            try
            {
                sw.Start();
                var result = await client.SendAsync(request, cts.Token);
                sw.Stop();
                if (wires.IsDetected((int)result.StatusCode))
                {
                    Logger.Warn($"Got detected at {Domain}");
                    HeaderDisguise.Apply(request);
                    Logger.Success("Headers changed successfully");
                    await Task.Delay(_jitterValue);
                }
                scan.Target = targetedDomain;
                scan.StatusCode = (int)result.StatusCode;
                scan.Headers = result.Headers.ToDictionary(h => h.Key, h => string.Join(",", h.Value));
                scan.LatencyMS = sw.ElapsedMilliseconds;
            }
            catch (HttpRequestException ex)
            {
                scan.Message = ex.Message;
                scan.LatencyMS = sw.ElapsedMilliseconds;
            }
            catch (Exception ex)
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
        public async Task<TlsScanResult> TlsScan(string Domain)
        {
            var tlsScan = new TlsScanResult();
            var handler = new SocketsHttpHandler
            {
                Proxy = new HttpToSocks5Proxy(TorIP, TorPort),
                UseProxy = true,
                SslOptions = new SslClientAuthenticationOptions
                {
                    RemoteCertificateValidationCallback = (sender, cert, chain, errors) =>
                    {
                        if(sender is SslStream sslStream)
                        {
                            tlsScan.TlsVersion = sslStream.SslProtocol.ToString();
                            tlsScan.CipherSuite = sslStream.NegotiatedCipherSuite.ToString();
                        }
                        if (cert is X509Certificate2 cert2)
                        {
                            tlsScan.CertSubject = cert2.Subject;
                            tlsScan.CertIssuer = cert2.Issuer;
                            tlsScan.CertThumbprint = cert2.Thumbprint;
                            tlsScan.CertSerialNumber = cert2.SerialNumber;
                            tlsScan.CertNotBefore = cert2.NotBefore;
                            tlsScan.CertNotAfter = cert2.NotAfter;
                            tlsScan.RawCertificateBase64 = Convert.ToBase64String(cert2.RawData);

                            var sanExtension = cert2.Extensions["2.5.29.17"];
                            if (sanExtension != null)
                            {
                                tlsScan.SubjectAlternativeNames = sanExtension.Format(false)
                                    .Split(new[] { ',', ';' }, StringSplitOptions.RemoveEmptyEntries)
                                    .Select(s => s.Trim().Replace("DNS Name=", ""))
                                    .ToList();
                            }
                        }
                        return true;
                    }
                }
            };
            var targetDomain = Target + Domain;
            var client = new HttpClient(handler);
            var request = new HttpRequestMessage(HttpMethod.Get, targetDomain);
            var cts = new CancellationTokenSource(TimeSpan.FromSeconds(Timeout));
            var sw = new Stopwatch();
            var wires = new GlobalWires();
            var _jitterValue = new Random().Next(Delay, Delay * 10);
            try
            {
                sw.Start();
                var result = await client.SendAsync(request);
                sw.Stop();
                if (wires.IsDetected((int)result.StatusCode))
                {
                    Logger.Warn($"Got detected at {Domain}");
                    HeaderDisguise.Apply(request);
                    Logger.Success("Headers changed successfully");
                    await Task.Delay(_jitterValue);
                }
                tlsScan.Target = targetDomain;
                tlsScan.StatusCode = (int)result.StatusCode;
                tlsScan.Message = result.ReasonPhrase ?? string.Empty;
                tlsScan.LatencyMS = sw.ElapsedMilliseconds;
            } catch(HttpRequestException ex)
            {
                tlsScan.Message = ex.Message;
                tlsScan.Target = targetDomain;
                tlsScan.LatencyMS = sw.ElapsedMilliseconds;
            } catch(Exception ex)
            {
                tlsScan.Message = ex.Message;
                tlsScan.Target = targetDomain;
                tlsScan.LatencyMS = sw.ElapsedMilliseconds;
            }
            finally
            {
                if(sw.IsRunning) sw.Stop();
            }
            return tlsScan;
        }
    }
}