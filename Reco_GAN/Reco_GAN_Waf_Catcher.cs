using System.Runtime.InteropServices;

namespace Reco_GAN_Waf_Catcher
{
    public class Waf_Catcher
    {
        private List<double> LiveLatencyList = new();
        private string responseBodyFilePath = string.Empty;
        private int liveLatencySize;
        private int subSampleSize;
        private string Domain = string.Empty;
        [DllImport("reco_gan_waf_catcher_cpp_module.so", CallingConvention = CallingConvention.Cdecl)]
        private extern static void mainWafCatchingEngine(IntPtr liveLatencyPtr, int liveLatencySize, string responseBodyFilePath, int s_sample_size, string domain);
        public Waf_Catcher(string domain, string _responseBodyFilePath, List<double> liveLatencyList, int s_sampleSize)
        {
            responseBodyFilePath = _responseBodyFilePath;
            Domain = domain;
            subSampleSize = s_sampleSize;
            liveLatencySize = liveLatencyList.Count();
            LiveLatencyList = liveLatencyList;
        }
        public void CatchWaf()
        {
            double[] liveLatencyArray = LiveLatencyList.ToArray();
            GCHandle liveLatencyArrayHandle = GCHandle.Alloc(liveLatencyArray, GCHandleType.Pinned);
            try
            {
                mainWafCatchingEngine(liveLatencyArrayHandle.AddrOfPinnedObject(), liveLatencySize, responseBodyFilePath, subSampleSize, Domain);
            }
            finally
            {
                if (liveLatencyArrayHandle.IsAllocated) { liveLatencyArrayHandle.Free(); }
            }
        }
    }
}