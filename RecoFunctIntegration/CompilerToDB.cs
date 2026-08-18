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
using DBModel;
using Reco_novich_compiler;
using AllFilesWires;
using ScanOutputModel;
using Wire;
using DB;
using ReconSageLogger;
using Reco_GAN_Latency_V2;

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
        public static async Task LatTrain(string domain, int numTrees, int subSampleSize, string pass)
        {
            DBModule dBModule = new DBModule(domain, pass);
            List<double> latencyList = await dBModule.GetAllLatency();
            Latency_Module latency_Module = new Latency_Module(domain, numTrees, subSampleSize, latencyList);
            latency_Module.Train();
        }
    }
}