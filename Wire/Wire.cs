/*
 * ReconSage_Cli - Advanced Network & Telemetry Reconnaissance Framework
 * Copyright (C) 2026 ReconSage_Cli Authors
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
using ReconSageLogger;
using System.Net;
using System.Net.Sockets;

namespace Wire
{
    public class GlobalWires
    {
        public List<int> DetectRLStatusCodes(List<int> statusCode)
        {
            List<int> DetectedCodes = new();
            List<int> RLCodes = new() { 429, 403, 402, 420, 430, 503, 507, 509 };
            foreach (var codes in statusCode)
            {
                if (RLCodes.Contains(codes))
                {
                    DetectedCodes.Add(codes);
                }
            }
            return DetectedCodes;
        }

        public List<double> SpikedLatency(List<double> latency)
        {
            if (latency == null || latency.Count < 10) return new List<double>();

            double average = latency.Average();
            double sumOfSquare = latency.Select(val => Math.Pow(val - average, 2)).Sum();

            // Use N-1 (latency.Count - 1) for better accuracy on smaller lists (Bessel's correction)
            double stDeviation = Math.Sqrt(sumOfSquare / (latency.Count - 1));
            double thresholds = average + (3 * stDeviation);

            return latency.Where(l => l > thresholds).ToList();
        }
        public bool isIncreasingLatency(List<double> LatencyList)
        {
            for (int i = 0; i < LatencyList.Count; i++)
            {
                if (LatencyList[i] < LatencyList[i++])
                {
                    return true;
                }
            }
            return false;
        }
        public Dictionary<string, string> ParseHeaders(string rawHeaders)
        {
            var headerDict = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);

            if (string.IsNullOrWhiteSpace(rawHeaders))
                return headerDict;

            // 1. Raw string ko lines mein tod do (\r\n standard hai HTTP ke liye)
            string[] lines = rawHeaders.Split(new[] { "\r\n", "\n" }, StringSplitOptions.RemoveEmptyEntries);

            foreach (var line in lines)
            {
                // Status line ko skip karo (e.g., HTTP/1.1 200 OK)
                if (line.StartsWith("HTTP/", StringComparison.OrdinalIgnoreCase))
                    continue;

                // 2. Pehla ':' dhoondo (Key aur Value ko alag karne ke liye)
                int separatorIndex = line.IndexOf(':');
                if (separatorIndex > 0)
                {
                    string key = line.Substring(0, separatorIndex).Trim();
                    string value = line.Substring(separatorIndex + 1).Trim();

                    // Duplicate keys handle karo (rare but possible)
                    if (!headerDict.ContainsKey(key))
                    {
                        headerDict.Add(key, value);
                    }
                }
            }

            return headerDict;
        }
        public string SanitizeTarget(string inputUrl)
        {
            try
            {
                if (!inputUrl.StartsWith("http://") && !inputUrl.StartsWith("https://"))
                {
                    return inputUrl;
                }
                else
                {
                    Uri uri = new Uri(inputUrl);
                    string host = uri.Host;
                    return host;
                }
            }
            catch (Exception)
            {
                return inputUrl;
            }
        }
        public bool isDecreasingLatency(List<double> LatencyList)
        {
            for (int i = 0; i < LatencyList.Count; i++)
            {
                if (LatencyList[i] > LatencyList[i++])
                {
                    return true;
                }
            }
            return false;
        }
        public async Task<string[]> ProcessWordlist(string wordlistPath)
        {
            return await File.ReadAllLinesAsync(wordlistPath);
        }
        public List<int> DetectWafStatusCodes(List<int> StatusCodes)
        {
            List<int> BlockedCodes = new List<int>()
            {
              403, 429, 406, 413, 401, 451, 400, 444, 520
            };
            List<int> DetectedCodes = new();
            foreach (var codes in StatusCodes)
            {
                if (BlockedCodes.Contains(codes))
                {
                    DetectedCodes.Add(codes);
                }
            }
            return DetectedCodes;
        }
        public bool IsDetected(int status_codes)
        {
            List<int> BlockedCodes = new List<int>()
            {
              403, 429, 406, 413, 401, 451, 400, 444, 520
            };
            if (BlockedCodes.Contains(status_codes))
            {
                return true;
            }
            return false;
        }
        public void ShowProgress(int current, int total, string target)
        {
            Console.Write($"\r");
            Console.ForegroundColor = ConsoleColor.Magenta;
            Console.Write("[SCANNING] ");
            Console.ResetColor();
            Console.Write($"Target: ");
            Console.ForegroundColor = ConsoleColor.Cyan;
            Console.Write($"{target,-20}");
            Console.ResetColor();
            Console.Write($" | Progress: ({current}/{total}) [");

            int barWidth = 20;
            int progress = (int)((double)current / total * barWidth);

            Console.ForegroundColor = ConsoleColor.Green;
            for (int i = 0; i < barWidth; i++)
            {
                if (i < progress) Console.Write("=");
                else
                {
                    Console.ResetColor();
                    Console.Write(" ");
                }
            }
            Console.ResetColor();
            Console.Write("]");
        }
        public HttpClient BuildClient(string proxyHost, int proxyPort)
        {
            var handler = new SocketsHttpHandler
            {
                Proxy = new WebProxy(proxyHost, proxyPort),
                UseProxy = true,
                PooledConnectionLifetime = TimeSpan.FromMinutes(3)
            };
            return new HttpClient(handler);
        }
        public async Task<bool> IsProxyOpen(string host, int port, int Timeout)
        {
            try
            {
                using (var tcp = new TcpClient())
                {
                    var connectTask = tcp.ConnectAsync(host, port);
                    var delayTask = Task.Delay(Timeout);
                    var completedTask = await Task.WhenAny(connectTask, delayTask);
                    return completedTask == connectTask && tcp.Connected;
                }
            }
            catch
            {
                return false;
            }
        }
        public async Task<bool> IsProxyWorking(string host, int port, int Timeout)
        {
            var isProxyOpen = await IsProxyOpen(host, port, Timeout);
            if (!isProxyOpen)
            {
                Logger.Error("[!]Proxy you have passed is not open so don't use it if you don't mind taking my advice");
                return false;
            }
            else
            {
                try
                {
                    using var cts = new CancellationTokenSource(TimeSpan.FromSeconds(Timeout));
                    using var handler = new SocketsHttpHandler
                    {
                        Proxy = new WebProxy(host, port),
                        UseProxy = true,
                        SslOptions = new System.Net.Security.SslClientAuthenticationOptions
                        {
                            RemoteCertificateValidationCallback = (sender, cert, chain, errors) => true
                        }
                    };
                    using var client = new HttpClient(handler);
                    var request = new HttpRequestMessage(HttpMethod.Get, "https://httpbin.org/ip");
                    var result = await client.SendAsync(request, cts.Token);
                    return result.IsSuccessStatusCode;
                }
                catch
                {
                    return false;
                }
            }
        }
    }
}