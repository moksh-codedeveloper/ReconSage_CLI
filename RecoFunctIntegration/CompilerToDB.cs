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
            var wires = new Wires();
            var wordlists = await new GlobalWires().ProcessWordlist(wordlistsPath);

            DBModule dB = new DBModule(target, password);
            await dB.InitializeDB();

            // 1. Stream items directly from JSON without keeping them in RAM
            async IAsyncEnumerable<Model> BuildFuzzerLogStream()
            {
                await foreach (var data in wires.StreamReadJson(jsonFilePath))
                {
                    foreach (var path in wordlists)
                    {
                        yield return new Model
                        {
                            Target = target,
                            WordlistsPath = path,
                            StatusCode = data.StatusCode,
                            LatencyMs = data.LatencyMS,
                            ReasonPhrase = data.Message,
                            HeadersFile = headersFilePath,
                            HtmlFilePath = htmlResponsePath,
                            JsonFilePath = jsonFilePath
                        };
                    }
                }
            }

            // 2. Insert all fuzzer logs in a single atomic transaction
            await dB.InsertLogsTransactionAsync(BuildFuzzerLogStream());

            Logger.Done("Insertion of Data from Json to DB file is complete and done");
        }
        public static async Task StatusCodeBasedFilterCompile(string domain, string password, int statusCode)
        {
            DBModule db = new DBModule(domain, password);
            await foreach (var record in db.StatusCodeFilter(statusCode))
            {
                new Reco_novich().CompileAndSave(record);
            }
        }

        public static async Task LatencyBasedFilterCompile(string domain, string password, double latency)
        {
            DBModule db = new DBModule(domain, password);
            await foreach (var record in db.LatencyFilter(latency))
            {
                new Reco_novich().CompileAndSave(record);
            }
        }
    }
}