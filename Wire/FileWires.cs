using ScanOutputModel;
using ReconSageLogger;
using System.Text.Json;
using Reco_novich_compiler;
using DB;

namespace AllFilesWires
{
    public class DBPacket
    {
        public string Target{set;get;} = string.Empty;
        public string JsonFilePath {set;get;} = string.Empty;
        public string HtmlFilePath {set;get;} = string.Empty;
        public string HeadersFile{set;get;} = string.Empty;
        public string WordlistsPath{set;get;} = string.Empty;
        public string ReasonPhrase {set;get;} = string.Empty;
        public int StatusCode{set;get;}
        public double LatencyList{set;get;}
    }
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

        public async Task JsonToDBFile()
        {
        }
    }
}