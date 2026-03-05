using System.Net;
using System.Net.Sockets;
using ITor;

namespace TorRotate
{
    public sealed class TorRotate : ITorController, IDisposable
    {
        private readonly string _password;
        private readonly IPAddress _host;
        private readonly int _port;

        private readonly SemaphoreSlim _rotateLock = new(1, 1);
        private DateTime _lastRotationUtc = DateTime.MinValue;

        private static readonly TimeSpan RotationCooldown = TimeSpan.FromSeconds(10);

        public TorRotate(string password, IPAddress host, int port)
        {
            _password = password ?? throw new ArgumentNullException(nameof(password));
            _host = host ?? throw new ArgumentNullException(nameof(host));
            _port = port;
        }

        public async Task RotateAsync(CancellationToken cancellationToken = default)
        {
            await _rotateLock.WaitAsync(cancellationToken);
            try
            {
                // Respect Tor cooldown
                var now = DateTime.UtcNow;
                var timeSinceLast = now - _lastRotationUtc;

                if (timeSinceLast < RotationCooldown)
                {
                    var delay = RotationCooldown - timeSinceLast;
                    await Task.Delay(delay, cancellationToken);
                }

                using TcpClient client = new();
                await client.ConnectAsync(_host, _port, cancellationToken);

                using NetworkStream stream = client.GetStream();
                using StreamReader reader = new(stream);
                using StreamWriter writer = new(stream)
                {
                    AutoFlush = true,
                    NewLine = "\r\n"
                };

                // AUTHENTICATE
                await writer.WriteLineAsync($"AUTHENTICATE \"{_password}\"")
                            .WaitAsync(cancellationToken);

                string? authResponse = await ReadTorResponseAsync(reader, cancellationToken);

                if (authResponse is null || !authResponse.StartsWith("250"))
                    throw new InvalidOperationException("Tor AUTH failed.");

                // SIGNAL NEWNYM
                await writer.WriteLineAsync("SIGNAL NEWNYM")
                            .WaitAsync(cancellationToken);

                string? nymResponse = await ReadTorResponseAsync(reader, cancellationToken);

                if (nymResponse is null || !nymResponse.StartsWith("250"))
                    throw new InvalidOperationException($"Tor NEWNYM failed: {nymResponse}");

                _lastRotationUtc = DateTime.UtcNow;

                Console.WriteLine("Tor circuit rotated successfully.");
            }
            finally
            {
                _rotateLock.Release();
            }
        }
        private static async Task<string> ReadTorResponseAsync(
    StreamReader reader,
    CancellationToken ct)
        {
            string? line;
            string? lastLine = null;

            while ((line = await reader.ReadLineAsync().WaitAsync(ct)) != null)
            {
                lastLine = line;

                // Final line ends with space after status code
                // Example: "250 OK"
                if (line.Length >= 4 && line[3] == ' ')
                    break;
            }

            return lastLine ?? string.Empty;
        }
        public void Dispose()
        {
            _rotateLock.Dispose();
        }
    }
}