using ScanOutputModel;
using ReconSageLogger;
using System.Text.Json;
using System.Text.Json.Serialization;
using System.Text.RegularExpressions;
using System.Text;
using ResponseBodyStruct;

namespace AllFilesWires
{
    public class Wires
    {
        private static readonly JsonSerializerOptions JsonOptions = new()
        {
            PropertyNameCaseInsensitive = true
        };
        public async IAsyncEnumerable<ScanOutput> StreamReadJson(string jsonFilePath)
        {
            if (!File.Exists(jsonFilePath))
            {
                Logger.Error($"[Core 2] File not found: {jsonFilePath}");
                yield break;
            }

            FileStream? fileStream = null;

            try
            {
                fileStream = File.OpenRead(jsonFilePath);
            }
            catch (Exception ex)
            {
                Logger.Error($"[Core 2] Failed to open file - {ex.Message}");
                yield break;
            }

            try
            {
                // Parse stream into document tree without deserializing all object instances
                using var doc = await JsonDocument.ParseAsync(fileStream);

                // Grab the 'Result' or 'result' array property from MainScanOutput
                if (doc.RootElement.ValueKind == JsonValueKind.Object)
                {
                    if (doc.RootElement.TryGetProperty("Result", out var resultElement) ||
                        doc.RootElement.TryGetProperty("result", out resultElement))
                    {
                        foreach (var element in resultElement.EnumerateArray())
                        {
                            var scan = element.Deserialize<ScanOutput>(JsonOptions);
                            if (scan != null)
                            {
                                scan.Message = CleanReasonPhrase(scan.Message);
                                yield return scan;
                            }
                        }
                    }
                }
            }
            finally
            {
                if (fileStream != null)
                {
                    await fileStream.DisposeAsync();
                }
            }
        }
        public async Task<string[]> ProcessWordlist(string wordlistPath)
        {
            return await File.ReadAllLinesAsync(wordlistPath);
        }
        private string CleanReasonPhrase(string rawMessage)
        {
            if (string.IsNullOrWhiteSpace(rawMessage))
                return string.Empty;
            string cleaned = Regex.Replace(rawMessage, @"^(?:HTTP/\d(?:\.\d)?\s+)?(?:\d{3}\s+)?", "", RegexOptions.IgnoreCase);

            return cleaned.Trim();
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
        public void WriteTextResBody(MainScanResponseBodyModel mainModel, string outputFilePath)
        {
            try
            {
                // Determine the final safe path to use
                string finalFilePath = outputFilePath;

                // Check if the file already exists on the disk space
                if (File.Exists(outputFilePath))
                {
                    // Isolate directory paths, raw filename, and the target extension (e.g., ".txt")
                    string directory = Path.GetDirectoryName(outputFilePath) ?? string.Empty;
                    string fileNameWithoutExt = Path.GetFileNameWithoutExtension(outputFilePath);
                    string extension = Path.GetExtension(outputFilePath);
                    // Generate a highly precise timestamp layout down to the second
                    string timestamp = DateTime.Now.ToString("yyyyMMdd_HHmmss");

                    // Construct a completely unique new name: filename_20260707_145000.txt
                    string newFileName = $"{fileNameWithoutExt}_{timestamp}{extension}";

                    // Reassemble the full absolute or relative path
                    finalFilePath = Path.Combine(directory, newFileName);
                }
                // Using a StringBuilder to batch file I/O operations efficiently
                StringBuilder sb = new StringBuilder();

                foreach (var scan in mainModel.Result)
                {
                    sb.AppendLine($"{scan.target}");
                    sb.AppendLine("---");
                    sb.AppendLine($"{scan.statusCode}");
                    sb.AppendLine("---");
                    if (!string.IsNullOrEmpty(scan.bodyResponse))
                    {
                        sb.AppendLine(scan.bodyResponse);
                    }
                    else
                    {
                        sb.AppendLine("[WARN] Response body was empty or unreachable.");
                    }

                    sb.AppendLine(); // Buffer spacing between iterations
                }
                sb.AppendLine();
                // Write or append the text block natively to the disk filesystem
                // If you want to append across completely separate command sessions, use File.AppendAllText instead
                File.WriteAllText(outputFilePath, sb.ToString());

                Console.WriteLine($"[SUCCESS] Pure text report successfully written to: {outputFilePath}");
            }
            catch (Exception ex)
            {
                Console.WriteLine($"[ERROR] Failed to write text report to disk: {ex.Message}");
            }
        }
        public async Task<string> HeaderTextParser(string headersFile)
        {
            if (!File.Exists(headersFile))
            {
                Logger.Error("Your headers file doesn't exist");
                return string.Empty;
            }
            StringBuilder cleanHeaders = new StringBuilder();
            await foreach (string line in File.ReadLinesAsync(headersFile))
            {
                if (string.IsNullOrWhiteSpace(line))
                {
                    continue;
                }

                int colonIndex = line.IndexOf(':');
                if (colonIndex > 0)
                {
                    string key = line.Substring(0, colonIndex).Trim();
                    string value = line.Substring(colonIndex + 1).Trim();
                    cleanHeaders.AppendLine($"{key}: {value}");
                }
            }
            string parsedBuilderHeaders = cleanHeaders.ToString();
            return parsedBuilderHeaders;
        }
    }
}