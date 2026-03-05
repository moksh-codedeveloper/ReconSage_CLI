using Wire;

namespace WarmUpScan
{
    public class Scan
    {
        private readonly string Target;
        private readonly int Timeout;
        private readonly string JsonFilePath;
        private readonly string WordlistPath;
        public Scan(string target, int timeout, string jsonFilePath, string wordlistPath)
        {
            Target = target;
            Timeout = timeout;
            JsonFilePath = jsonFilePath;
            WordlistPath = wordlistPath;
        }
    }
}