namespace ScanOutputModel
{
    public class ScanOutput
    {
        public string Target { set; get; } = string.Empty;
        public double LatencyMS { set; get; }
        public string Message { set; get; } = string.Empty;
        public Dictionary<string, string> Headers { set; get; } = new Dictionary<string, string>();
        public int StatusCode { set; get; }
    }

    public class MainScanOutput
    {
        public List<ScanOutput> Result { set; get; } = new();
    }
    public class TlsScanResult
    {
        public string Target { get; set; } = string.Empty;
        public int StatusCode { get; set; }
        public double LatencyMS { get; set; }
        public string TlsVersion { get; set; } = string.Empty;
        public string CipherSuite{set;get;} = string.Empty;
        public string Protocol { get; set; } = string.Empty;
        public string Message{set;get;} = string.Empty;
    }

    public class MainTorScan
    {
        public  List<TlsScanResult> Results{set;get;} = new();
    }
}