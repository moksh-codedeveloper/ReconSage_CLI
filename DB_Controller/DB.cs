using DBModel;
using Microsoft.Data.Sqlite;
using ReconSageLogger;
using System.IO;

namespace DB
{
    public class DBModule
    {
        private string BaseDataDirectory; // Central folder for all databases
        public string GetLinuxHomeDirectory()
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
        private string DBPath = string.Empty;
        private string DBPassword = string.Empty;
        private string ConnectionString = string.Empty;
        private string CurrentTargetDomain = string.Empty;
        private SqliteConnection? connection;

        public DBModule(string targetDomain, string pass)
        {
            DBPassword = pass;
            BaseDataDirectory = Path.Combine(GetLinuxHomeDirectory(), "ReconSage_Data");
            CurrentTargetDomain = targetDomain.Replace("https://", "").Replace("http://", "").Trim('/');
            DBPath = Path.Combine(BaseDataDirectory, $"{CurrentTargetDomain}.sqlite");

            if (!Directory.Exists(BaseDataDirectory))
            {
                Directory.CreateDirectory(BaseDataDirectory);
            }

            // Builder handles special characters in passwords cleanly
            var builder = new SqliteConnectionStringBuilder
            {
                DataSource = DBPath,
                Mode = SqliteOpenMode.ReadWriteCreate
            };

            if (!string.IsNullOrWhiteSpace(DBPassword))
            {
                builder.Password = DBPassword;
            }

            ConnectionString = builder.ToString();
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

        public async Task InsertLogsTransactionAsync(IAsyncEnumerable<Model> logStream)
        {
            EnsureConnectionInitialized();
            if (connection!.State != System.Data.ConnectionState.Open)
                await connection.OpenAsync();

            // Begin single transaction for the entire fuzzing run
            await using var transaction = await connection.BeginTransactionAsync();

            string insertSql = @"
        INSERT INTO RequestLogs (Target, JsonFilePath, HeadersFile, WordlistsPath, ReasonPhrase, HtmlResponseFile, StatusCode, LatencyMs) 
        VALUES ($target, $jsonPath, $headers, $wordlist, $reason, $html, $status, $latency);";

            await using var command = new SqliteCommand(insertSql, connection, (SqliteTransaction)transaction);

            var pTarget = command.Parameters.Add("$target", SqliteType.Text);
            var pJsonPath = command.Parameters.Add("$jsonPath", SqliteType.Text);
            var pHeaders = command.Parameters.Add("$headers", SqliteType.Text);
            var pWordlist = command.Parameters.Add("$wordlist", SqliteType.Text);
            var pReason = command.Parameters.Add("$reason", SqliteType.Text);
            var pHtml = command.Parameters.Add("$html", SqliteType.Text);
            var pStatus = command.Parameters.Add("$status", SqliteType.Integer);
            var pLatency = command.Parameters.Add("$latency", SqliteType.Real);

            await foreach (var log in logStream)
            {
                pTarget.Value = log.Target ?? string.Empty;
                pJsonPath.Value = log.JsonFilePath ?? string.Empty;
                pHeaders.Value = log.HeadersFile ?? string.Empty;
                pWordlist.Value = log.WordlistsPath ?? string.Empty;
                pReason.Value = log.ReasonPhrase ?? string.Empty;
                pHtml.Value = log.HtmlFilePath ?? string.Empty;
                pStatus.Value = log.StatusCode;
                pLatency.Value = log.LatencyMs;

                await command.ExecuteNonQueryAsync();
            }

            // One single disk flush for all fuzzer results
            await transaction.CommitAsync();
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
        private string GetDatabaseFilePath()
        {
            return Path.GetFullPath(DBPath); // Returns absolute native Linux path
        }
        public async IAsyncEnumerable<CompilerDataModel> StatusCodeFilter(int statusCode, int chunkSize = 1000)
        {
            EnsureConnectionInitialized();
            if (connection!.State != System.Data.ConnectionState.Open)
                await connection.OpenAsync();

            string sqlCommand = @"
                SELECT StatusCode, LatencyMs, ReasonPhrase
                FROM RequestLogs 
                WHERE StatusCode = @statusCode;";

            using var command = new SqliteCommand(sqlCommand, connection);
            command.Parameters.AddWithValue("@statusCode", statusCode);

            using var reader = await command.ExecuteReaderAsync();

            var currentChunk = new CompilerDataModel { Domain = CurrentTargetDomain };

            while (await reader.ReadAsync())
            {
                currentChunk.StatusCodes.Add(reader.IsDBNull(0) ? 0 : reader.GetInt32(0));
                currentChunk.LatencyList.Add(reader.IsDBNull(1) ? 0.0 : reader.GetDouble(1));
                currentChunk.ReasonPhrase.Add(reader.IsDBNull(2) ? string.Empty : reader.GetString(2));

                // Once the current batch hits chunkSize, yield it and reset for the next batch!
                if (currentChunk.StatusCodes.Count >= chunkSize)
                {
                    currentChunk.TotalRecords = currentChunk.StatusCodes.Count;
                    yield return currentChunk;

                    // Reset for next mini-batch
                    currentChunk = new CompilerDataModel { Domain = CurrentTargetDomain };
                }
            }

            // Yield any remaining records in the last partial chunk
            if (currentChunk.StatusCodes.Count > 0)
            {
                currentChunk.TotalRecords = currentChunk.StatusCodes.Count;
                yield return currentChunk;
            }
        }
        public async IAsyncEnumerable<CompilerDataModel> LatencyFilter(double latencyMs, int chunkSize = 1000)
        {
            EnsureConnectionInitialized();
            if (connection!.State != System.Data.ConnectionState.Open)
                await connection.OpenAsync();

            string sqlCommand = @"
                SELECT StatusCode, LatencyMs, ReasonPhrase
                FROM RequestLogs
                WHERE LatencyMs = @latencyMs;";

            using var command = new SqliteCommand(sqlCommand, connection);
            command.Parameters.AddWithValue("@latencyMs", latencyMs);

            using var reader = await command.ExecuteReaderAsync();

            var currentChunk = new CompilerDataModel { Domain = CurrentTargetDomain };

            while (await reader.ReadAsync())
            {
                currentChunk.StatusCodes.Add(reader.IsDBNull(0) ? 0 : reader.GetInt32(0));
                currentChunk.LatencyList.Add(reader.IsDBNull(1) ? 0.0 : reader.GetDouble(1));
                currentChunk.ReasonPhrase.Add(reader.IsDBNull(2) ? string.Empty : reader.GetString(2));

                if (currentChunk.StatusCodes.Count >= chunkSize)
                {
                    currentChunk.TotalRecords = currentChunk.StatusCodes.Count;
                    yield return currentChunk;

                    currentChunk = new CompilerDataModel { Domain = CurrentTargetDomain };
                }
            }

            if (currentChunk.StatusCodes.Count > 0)
            {
                currentChunk.TotalRecords = currentChunk.StatusCodes.Count;
                yield return currentChunk;
            }
        }
    }
}