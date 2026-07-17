using ScanOutputModel;
using ReconSageLogger;
using System.Text.Json;
using Reco_novich_compiler;
using DB;
using System.Text.Json.Serialization;
using DBModel;

namespace AllFilesWires
{
    public class Wires
    {
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
        public async Task JsonToCompilerTxtFile(string jsonFilePath)
        {
            MainScanOutput JsonData = await ReadJson(jsonFilePath);
            string target = JsonData.Result.Select(x => x.Target).ToString() ?? "";
            List<int> status_code = JsonData.Result.Select(x => x.StatusCode).ToList<int>() ?? new();
            List<double> latency_list = JsonData.Result.Select(x => x.LatencyMS).ToList<double>() ?? new();
            List<string> reason_phrase = JsonData.Result.Select(x => x.Message).ToList<string>() ?? new();
            int total_records = status_code.Count;
            Reco_novich compiler = new Reco_novich(target, status_code, latency_list, reason_phrase, total_records);
            compiler.CompileAndSave();
        }
        public async Task<CompilerDataModel> DBToCompiler(string domain, string password)
        {
            CompilerDataModel model;
            DBModule db = new DBModule(domain, password);
            model = await db.CompilerDataQuery();
            return model;
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
    }
}