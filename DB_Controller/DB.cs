using DBModel;
using Microsoft.Data.Sqlite;
using ReconSageLogger;
using System.IO;

namespace DB
{
    public class DBModule
    {
        private static string GetLinuxHomeDirectory()
        {
            // 1. Try standard .NET API (checks env, then /etc/passwd)
            string home = Environment.GetFolderPath(Environment.SpecialFolder.UserProfile);

            // 2. Fallback to OS environment variable
            if (string.IsNullOrEmpty(home))
            {
                home = Environment.GetEnvironmentVariable("HOME") ?? "";
            }

            // 3. Fallback for specific minimal root environments
            if (string.IsNullOrEmpty(home) && Environment.UserName == "root")
            {
                home = "/root";
            }

            // 4. Ultimate fallback if everything else fails
            if (string.IsNullOrEmpty(home))
            {
                throw new InvalidOperationException("Could not determine the home directory.");
            }

            return home;
        }
        private string BaseDataDirectory = $"{GetLinuxHomeDirectory}/ReconSage_Data"; // Central folder for all databases
        private string DBPath = string.Empty;
        private string DBPassword = string.Empty;
        private string ConnectionString = string.Empty;
        private string CurrentTargetDomain = string.Empty;
        private Model log;
        private SqliteConnection? connection;

        public DBModule(string targetDomain, string pass, Model modelLog)
        {
            DBPassword = pass;
            log = modelLog;

            // Clean the domain string to make it a safe filename (e.g., "google.com")
            CurrentTargetDomain = targetDomain.Replace("https://", "").Replace("http://", "").Trim('/');

            // Step 1: Ensure our storage base directory exists on Arch/Linux filesystem
            if (!Directory.Exists(BaseDataDirectory))
            {
                Directory.CreateDirectory(BaseDataDirectory);
            }

            // Step 2: Solder the dynamic path "ReconSage_Data/google.com.sqlite"
            DBPath = Path.Combine(BaseDataDirectory, $"{CurrentTargetDomain}.sqlite");

            // Step 3: Establish the final encrypted or standard connection string
            ConnectionString = $"Data Source={DBPath}; Password={DBPassword}";
        }

        private void EnsureConnectionInitialized()
        {
            if (connection == null)
            {
                connection = new SqliteConnection(ConnectionString);
            }
        }

        public async Task InitializeDB()
        {
            EnsureConnectionInitialized();
            if (connection!.State != System.Data.ConnectionState.Open) { await connection.OpenAsync(); }

            // Strict flat schema matching our C++ vector aggregation model
            string createDBModel = @"
                CREATE TABLE IF NOT EXISTS RequestLogs (
                    Id INTEGER PRIMARY KEY AUTOINCREMENT,
                    Target TEXT,
                    JsonFilePath TEXT,
                    HeadersFile TEXT,
                    WordlistsPath TEXT,
                    ReasonPhrase TEXT,
                    HtmlResponseFile TEXT,
                    StatusCode INTEGER,
                    LatencyMs REAL
                );";

            using var command = new SqliteCommand(createDBModel, connection);
            await command.ExecuteNonQueryAsync();
            Logger.Info($"[+] Isolated Database Matrix Ready for Target: {CurrentTargetDomain}");
        }

        public async Task InsertLogs()
        {
            EnsureConnectionInitialized();
            if (connection!.State != System.Data.ConnectionState.Open) { await connection.OpenAsync(); }

            string insertSql = @"
                INSERT INTO RequestLogs (Target, JsonFilePath, HeadersFile, WordlistsPath, ReasonPhrase, HtmlResponseFile, StatusCode, LatencyMs) 
                VALUES ($target, $jsonPath, $headers, $wordlist, $reason, $html, $status, $latency);";

            using var command = new SqliteCommand(insertSql, connection);
            command.Parameters.AddWithValue("$target", log.Target);
            command.Parameters.AddWithValue("$jsonPath", log.JsonFilePath);
            command.Parameters.AddWithValue("$headers", log.HeadersFile);
            command.Parameters.AddWithValue("$wordlist", log.WordlistsPath);
            command.Parameters.AddWithValue("$reason", log.ReasonPhrase);
            command.Parameters.AddWithValue("$html", log.HtmlFilePath);
            command.Parameters.AddWithValue("$status", log.StatusCode);
            command.Parameters.AddWithValue("$latency", log.LatencyMs);

            await command.ExecuteNonQueryAsync();
        }

        public async Task<List<Model>> ReadLogs()
        {
            EnsureConnectionInitialized();
            if (connection!.State != System.Data.ConnectionState.Open) { await connection.OpenAsync(); }

            string selectSql = "SELECT Id, Target, JsonFilePath, HeadersFile, WordlistsPath, ReasonPhrase, HtmlResponseFile, StatusCode, LatencyMs FROM RequestLogs;";
            var logList = new List<Model>();

            using var command = new SqliteCommand(selectSql, connection);
            using var render = await command.ExecuteReaderAsync();

            while (await render.ReadAsync())
            {
                var rowLog = new Model
                {
                    Id = render.GetInt32(0),
                    Target = render.IsDBNull(1) ? string.Empty : render.GetString(1),
                    JsonFilePath = render.IsDBNull(2) ? string.Empty : render.GetString(2),
                    HeadersFile = render.IsDBNull(3) ? string.Empty : render.GetString(3),
                    WordlistsPath = render.IsDBNull(4) ? string.Empty : render.GetString(4),
                    ReasonPhrase = render.IsDBNull(5) ? string.Empty : render.GetString(5),
                    HtmlFilePath = render.IsDBNull(6) ? string.Empty : render.GetString(6),
                    StatusCode = render.IsDBNull(7) ? 0 : render.GetInt32(7),
                    LatencyMs = render.IsDBNull(8) ? 0.0 : render.GetDouble(8)
                };
                logList.Add(rowLog);
            }
            return logList;
        }

        public async Task ClearLogs()
        {
            EnsureConnectionInitialized();
            if (connection!.State != System.Data.ConnectionState.Open) await connection.OpenAsync();

            string deleteQuery = "DELETE FROM RequestLogs; DELETE FROM sqlite_sequence WHERE name='RequestLogs';";

            using var command = new SqliteCommand(deleteQuery, connection);
            await command.ExecuteNonQueryAsync();
            Logger.Info($"[-] System Wipe Complete: Logs cleared for {CurrentTargetDomain}");
        }

        // Getter function so your Orchestrator can easily pass the exact path to C++ .so module
        public string GetDatabaseFilePath()
        {
            return Path.GetFullPath(DBPath); // Returns absolute native Linux path
        }
        public async Task<CompilerDataModel> CompilerDataQuery()
        {
            EnsureConnectionInitialized();
            if (connection!.State != System.Data.ConnectionState.Open) await connection.OpenAsync();
            CompilerDataModel dataModel = new CompilerDataModel();
            dataModel.Domain = CurrentTargetDomain;
            string sqlCommand = "SELECT ReasonPhrase, StatusCode, LatencyMs from RequestLogs;";
            using var command = new SqliteCommand(sqlCommand, connection);
            using var render = await command.ExecuteReaderAsync();
            while (await render.ReadAsync())
            {
                dataModel.ReasonPhrase.Add(render.IsDBNull(0) ? string.Empty : render.GetString(0));
                dataModel.StatusCodes.Add(render.IsDBNull(1) ? 0 : render.GetInt32(1));
                dataModel.LatencyList.Add(render.IsDBNull(2) ? 0.0 : render.GetDouble(2));
            }
            return dataModel;
        }
    }

}