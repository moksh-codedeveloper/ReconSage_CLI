using DBModel;
using Reco_novich_compiler;
using AllFilesWires;
using ScanOutputModel;
using Wire;
using DB;
using ReconSageLogger;

namespace CompilerToDB
{
    public static class JsonToDB
    {
        public static async Task JsonFileToDB(string jsonFilePath, string wordlistsPath, string htmlResponsePath, string headersFilePath, string target, string password)
        {
            List<Model> packet = new();
            MainScanOutput jsonReadOutput = await new Wires().ReadJson(jsonFilePath);
            var wordlists = await new GlobalWires().ProcessWordlist(wordlistsPath);
            foreach(var data in jsonReadOutput.Result)
            {
                foreach(var paths in wordlists)
                {
                    var model = new Model
                    {
                        Target = data.Target,
                        WordlistsPath = paths,
                        StatusCode = data.StatusCode,
                        LatencyMs = data.LatencyMS,
                        ReasonPhrase = data.Message,
                        HeadersFile = headersFilePath,
                        HtmlFilePath = htmlResponsePath,
                        JsonFilePath = jsonFilePath
                    };
                    packet.Add(model);
                }
            }
            DBModule dB = new DBModule(target, password);
            await dB.InitializeDB();
            foreach(var data in packet)
            {
                await dB.InsertLogs(data);
            }
            Logger.Done("Insertion of Data from Json to DB file is complete and done");
        }
    }
}