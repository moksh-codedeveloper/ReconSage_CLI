using System.Net;

namespace StealthStack
{
    public class StealthEngine
    {
        private readonly string Target;
        private int Timeout;
        private readonly IPAddress host;
        private readonly int port;
        private readonly string password;
        private readonly string ReportFile ;
        private readonly string WordlistPath;
        public StealthEngine(string target, IPAddress Host, int Port, int timeout, string jsonFilePath, string wordlistPath, string Password)
        {
            Target = target;
            port = Port;
            password = Password;
            host = Host;
            Timeout = timeout;
            WordlistPath = wordlistPath;
            ReportFile = jsonFilePath;
        }
    }
}