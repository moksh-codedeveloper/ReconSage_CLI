using ReconSageLogger;
using ScanOutputModel;
using System.Net;
using System.Text.Json;
using System.Text.Json.Serialization;

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
        public async Task<MainScanOutput> ReadJson(string jsonFilePath)
        {
            try
            {
                string jsonString = await File.ReadAllTextAsync(jsonFilePath);
                MainScanOutput jsonDeserialised = JsonSerializer.Deserialize<MainScanOutput>(jsonString) ?? new MainScanOutput();
                return jsonDeserialised;
            }
            catch (FileNotFoundException ex)
            {
                Logger.Error($"Unexpected Error - {ex.Message}");
                return new MainScanOutput();
            }
            catch (JsonException ex)
            {
                Logger.Error($"Unexpected Error - {ex.Message}");
                return new MainScanOutput();
            }
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
        public string[] WordlistBatching(int sizeOfWordlistToAccess, string[] wordlistArr, int valueToSkip)
        {
            return wordlistArr.Skip(valueToSkip).Take(sizeOfWordlistToAccess).ToArray();
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
        public async Task WriteToJsonAsync<T>(T data, string filePath)
        {
            var options = new JsonSerializerOptions
            {
                WriteIndented = true,
                DefaultIgnoreCondition = JsonIgnoreCondition.WhenWritingNull
            };

            // Handle duplicate file names
            string directory = Path.GetDirectoryName(filePath) ?? string.Empty;
            string fileName = Path.GetFileNameWithoutExtension(filePath);
            string extension = Path.GetExtension(filePath);

            string newFilePath = filePath;
            int count = 1;

            while (File.Exists(newFilePath))
            {
                newFilePath = Path.Combine(directory, $"{fileName}({count}){extension}");
                count++;
            }

            var json = JsonSerializer.Serialize(data, options);

            await File.WriteAllTextAsync(newFilePath, json);
            Logger.Success($"JSON output written to: {newFilePath}");
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
    }
}