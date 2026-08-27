/*
 * ReconSage_Cli - Advanced Network & Telemetry Reconnaissance Framework
 * Copyright (C) 2026 ReconSage_Cli Authors
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
using System.Runtime.InteropServices;
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
        [DllImport("reco_gan_lat_v2_ml_module_cpp_module.so", CallingConvention = CallingConvention.Cdecl)]
        private static extern void reco_Gan_V2_predict(string domain, int subsample_size, IntPtr latency_list, int latency_size);
        private List<double> latencyDataSet = new();
        private string domain = string.Empty;
        private int totalCount;
        public Latency_Module(string _domain, List<double> _latencyDataSet)
        {
            latencyDataSet = _latencyDataSet;
            domain = _domain;
            totalCount = _latencyDataSet.Count();
        }
        public void Train(int numTrees, int subSampleSize)
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
        public void Predict(int subSampleSize)
        {
            double[] latencyArray = latencyDataSet.ToArray();
            GCHandle latencyHandle = GCHandle.Alloc(latencyArray, GCHandleType.Pinned);
            try
            {
                reco_Gan_V2_predict(domain, subSampleSize, latencyHandle.AddrOfPinnedObject(), totalCount);
            }
            finally
            {
                if (latencyHandle.IsAllocated) latencyHandle.Free();
            }
        }
    }
}