namespace StealthStack
{
    public static class HeaderDisguise
    {
        private static readonly Random _rnd = new Random();

        private static readonly string[] UserAgents = {
        "Mozilla/5.0 (X11; Arch Linux; Linux x86_64; rv:124.0) Gecko/20100101 Firefox/124.0",
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/122.0.0.0 Safari/537.36",
        "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/122.0.0.0 Safari/537.36",
        "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/121.0.0.0 Safari/537.36"
    };

        public static void Apply(HttpRequestMessage request)
        {
            // Set the User-Agent
            request.Headers.UserAgent.ParseAdd(UserAgents[_rnd.Next(UserAgents.Length)]);

            // Add the "Sec-Ch-Ua" headers (Modern Browser Fingerprinting)
            request.Headers.Add("sec-ch-ua", "\"Chromium\";v=\"122\", \"Not(A:Brand\";v=\"24\", \"Google Chrome\";v=\"122\"");
            request.Headers.Add("sec-ch-ua-mobile", "?0");
            request.Headers.Add("sec-ch-ua-platform", "\"Linux\"");

            // Essential for bypassing basic bot filters
            request.Headers.Add("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8");
            request.Headers.Add("Accept-Language", "en-US,en;q=0.5");
            request.Headers.Add("Sec-Fetch-Dest", "document");
            request.Headers.Add("Sec-Fetch-Mode", "navigate");
            request.Headers.Add("Sec-Fetch-Site", "none");
        }
    }
}