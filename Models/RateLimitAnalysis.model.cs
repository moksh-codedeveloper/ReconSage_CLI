namespace RateLimitDetector.Model
{
    public class RateLimitDetectionOutputModel
    {
        public List<string> Target { set; get; } = new();
        public List<double> LatencyMS { set; get; } = new();
        public List<int> StatusCode { set; get; } = new();
        public List<int> DetectedStatusCodeList { set; get; } = new();
        public List<double> SpikedLatencyMS { set; get; } = new();
        public List<double> increasingTrendCount { set; get; } = new();
        public List<double> decreasingTrendCount { set; get; } = new();
    }
}

