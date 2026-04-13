namespace ScanOutputModel
{
    public class ScanOutput
    {
        public string Target { set; get; } = string.Empty;
        public double LatencyMS { set; get; }
        public string Message { set; get; } = string.Empty;
        public Dictionary<string, string> Headers { set; get; } = new Dictionary<string, string>();
        public int StatusCode { set; get; }

        public string ResponseBody{set;get;} = string.Empty;
    }

    public class MainScanOutput
    {
        public List<ScanOutput> Result { set; get; } = new();
    }
    public class TlsScanResult
    {
        // Connection Metadata
        public string Target { get; set; } = string.Empty;
        public int StatusCode { get; set; }
        public double LatencyMS { get; set; }
        public string Message { get; set; } = string.Empty;

        // Modern TLS Data
        public string TlsVersion { get; set; } = string.Empty;   // e.g., "Tls13"
        public string CipherSuite { get; set; } = string.Empty; // e.g., "TlsAes256GcmSha384"
        public string Protocol { get; set; } = string.Empty;    // e.g., "Http/1.1" or "H2"

        // --- Modern Certificate Extraction ---

        public string CertSubject { get; set; } = string.Empty;      // Main entity
        public string CertIssuer { get; set; } = string.Empty;       // Who signed it (CA)
        public string CertThumbprint { get; set; } = string.Empty;   // Unique SHA1 fingerprint
        public string CertSerialNumber { get; set; } = string.Empty; // CA serial number
        public DateTime? CertNotBefore { get; set; }                 // Activation date
        public DateTime? CertNotAfter { get; set; }                  // Expiration date

        // Crucial for Subdomain Discovery
        public List<string> SubjectAlternativeNames { get; set; } = new();

        // The "Proof": Full cert encoded in Base64
        public string RawCertificateBase64 { get; set; } = string.Empty;
    }

    public class MainTorScan
    {
        public List<TlsScanResult> Results { set; get; } = new();
    }
}