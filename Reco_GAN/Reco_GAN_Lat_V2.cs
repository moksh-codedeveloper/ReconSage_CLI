using System.Runtime.InteropServices;
using System;
namespace Reco_GAN_Latency_V2
{
    public class Latency_Module
    {
        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
        internal struct reco_gan_v2
        {
            public IntPtr domain;
            public IntPtr latency_data;
            public int total_counts;
            public int sub_sample_size;
            public int num_trees;
        }
        [DllImport("reco_gan_lat_v2_ml_module_cpp_module.so", CallingConvention = CallingConvention.Cdecl)]
        private static extern void reco_Gan_V2(reco_gan_v2 packet);
        private List<double> latencyDataSet = new();
        private string domain = string.Empty;
        private int totalCount;
        private int subSampleSize;
        private int numTrees;
        public Latency_Module(string _domain, int _numTrees, int _subSampleSize, List<double> _latencyDataSet)
        {
            latencyDataSet = _latencyDataSet;
            domain = _domain;
            numTrees = _numTrees;
            subSampleSize = _subSampleSize;
            totalCount = _latencyDataSet.Count();
        }
        public void Train()
        {
            IntPtr domainPtr = Marshal.StringToHGlobalAnsi(domain);
            double[] latencyArray = latencyDataSet.ToArray();
            GCHandle latencyHandle = GCHandle.Alloc(latencyArray, GCHandleType.Pinned);
            reco_gan_v2 packet = new reco_gan_v2();
            try
            {
                packet.domain = domainPtr;
                packet.latency_data = latencyHandle.AddrOfPinnedObject();
                packet.num_trees = numTrees;
                packet.sub_sample_size = subSampleSize;
                packet.total_counts = totalCount;
                reco_Gan_V2(packet);
            }
            finally
            {
                // 6. Memory Cleanup Block (Prevents Unmanaged RAM Leaks!)
                Marshal.FreeHGlobal(domainPtr);
                if (latencyHandle.IsAllocated) latencyHandle.Free();
            }
        }
    }
}