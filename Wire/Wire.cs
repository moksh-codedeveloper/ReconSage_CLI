using ScanOutputModel;
using System.Text.Json;

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
            catch (FileNotFoundException)
            {
                Console.WriteLine($"Error: The file '{jsonFilePath}' was not found.");
                return new MainScanOutput();
            }
            catch (JsonException ex)
            {
                Console.WriteLine($"Error decoding JSON: {ex.Message}");
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
            // \r moves the cursor to the start of the line
            // The $ allows us to put variables directly into the string
            Console.Write($"\r[SCANNING] Target: {target,-20} | Progress: ({current}/{total}) [");

            // Optional: Add a small visual bar
            int barWidth = 20;
            int progress = (int)((double)current / total * barWidth);

            for (int i = 0; i < barWidth; i++)
            {
                if (i < progress) Console.Write("=");
                else Console.Write(" ");
            }

            Console.Write("]");
        }
        /*
        how to use the above function easily 
        int total = wordlist.Count;
for (int i = 0; i < total; i++)
{
    string currentTarget = wordlist[i];
    
    // Update the animation
    ShowProgress(i + 1, total, currentTarget);
    
    // Perform the actual stealth scan
    await engine.ScanTarget(currentTarget);
}

// Move to a new line when finished
Console.WriteLine("\n[DONE] Batch Processing Complete.");

Retro style
public static void TypeWriter(string message)
{
    foreach (char c in message)
    {
        Console.Write(c);
        Thread.Sleep(30); // 30ms feels like an old mechanical printer
    }
    Console.WriteLine();
}
        */
    }
}