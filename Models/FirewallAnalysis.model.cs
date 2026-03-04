namespace FirewallAnalysis.Model
{
    public class WebFirewallAnalysisOutput
    {
        public List<string> ListOfTarget { set; get; } = new();
        public List<double> LatencyList { set; get; } = new();
        public List<double> SpikedLatency { set; get; } = new();
        public bool isLatencyInreasing { set; get; }
        public bool isLatencyDecreasing { set; get; }
        public List<int> StatusCodeList { set; get; } = new();
        public List<int> DetectedStatusCodes { set; get; } = new();
        public List<Dictionary<string, string>> Headers { set; get; } = new();
        public List<string> Message { set; get; } = new();
    }
}