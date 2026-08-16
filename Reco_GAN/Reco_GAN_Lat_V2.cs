using System.Runtime.InteropServices;
using System;
namespace Reco_GAN_Latency_V2
{
    public class Latency_Module
    {
        [DllImport("reco_gan_lat_v2_ml_module_cpp_module.so", CallingConvention = CallingConvention.Cdecl)]
        private static extern void reco_Gan_V2();
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
    }
}