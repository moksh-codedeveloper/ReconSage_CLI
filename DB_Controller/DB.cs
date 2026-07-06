using System;
using DBModel;
using Microsoft.Data.Sqlite;
using ReconSageLogger;

namespace DB
{
    public class DBModule
    {
        private string DBPath = string.Empty;
        private string DBPassword = string.Empty;
        private string ConnectionString;
        private Model log;
        private SqliteConnection connection;
        public DBModule(string dbPath, string pass, Model modelLog)
        {
            DBPath = dbPath;
            DBPassword = pass;
            ConnectionString = $"Data Source={DBPath}; Password={DBPassword}";
            log = modelLog;
            connection = new SqliteConnection(ConnectionString);
        }
        public async Task InitializeDB()
        {
            if (connection.State != System.Data.ConnectionState.Open) { await connection.OpenAsync(); }
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
            );
            ";
            using var command = new SqliteCommand(createDBModel, connection);
            await command.ExecuteNonQueryAsync();
        }

        public async Task InsertLogs()
        {
            if (connection.State != System.Data.ConnectionState.Open) { await connection.OpenAsync(); }
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
            Logger.Info("Log entries done successfully");
        }

        public async Task<List<Model>> ReadLogs()
        {
            if (connection.State != System.Data.ConnectionState.Open) { await connection.OpenAsync(); }
            string selectSql = "SELECT Id, Target, JsonFilePath, HeadersFile, WordlistsPath, ReasonPhrase, HtmlResponseFile, StatusCode, LatencyMs FROM RequestLogs;";
            var logList = new List<Model>();
            using var command = new SqliteCommand(selectSql, connection);
            using var render = await command.ExecuteReaderAsync();
            while (await render.ReadAsync())
            {
                var log = new Model
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

                // Add it to the list
                logList.Add(log);
            }
            return logList;
        }

        public async Task ClearLogs()
        {
            if (connection.State != System.Data.ConnectionState.Open) await connection.OpenAsync();

            // SQLite uses DELETE FROM instead of TRUNCATE
            string deleteQuery = "DELETE FROM RequestLogs; DELETE FROM sqlite_sequence WHERE name='RequestLogs';";

            using var command = new SqliteCommand(deleteQuery, connection);
            await command.ExecuteNonQueryAsync();
            Logger.Info("Logs cleared from your db file....");
        }
    }
}