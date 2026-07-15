using System.Runtime.InteropServices;
using CompilerModel;

namespace Reco_novich_compiler
{
    public class Reco_novich
    {
        [DllImport("reco_novich_compiler_cpp_module.so", CallingConvention = CallingConvention.Cdecl)]
        private static extern void compile_telemetry_save_file(CompilerDataPacket data_packet);

        private List<int> StatusCode;
        private List<double> LatencyList;
        private string Domain;
        private List<string> Reason_phrase;
        private int TotalRecords;

        public Reco_novich(string domain, List<int> status_code, List<double> latency_list, List<string> reason_phrase, int total_records)
        {
            Domain = domain;
            StatusCode = status_code;
            LatencyList = latency_list;
            Reason_phrase = reason_phrase;
            TotalRecords = total_records;
        }

        public void CompileAndSave()
        {
            CompilerDataPacket packet = new CompilerDataPacket();

            // 1. Marshal Domain (C# String -> Native Unmanaged ANSI char*)
            IntPtr nativeDomainPtr = Marshal.StringToHGlobalAnsi(Domain);

            // 2. Marshal Primitive Lists (Convert to Array and Pin in Memory for zero-copy access)
            int[] statusCodeArray = StatusCode.ToArray();
            double[] latencyArray = LatencyList.ToArray();

            GCHandle statusHandle = GCHandle.Alloc(statusCodeArray, GCHandleType.Pinned);
            GCHandle latencyHandle = GCHandle.Alloc(latencyArray, GCHandleType.Pinned);

            // 3. Marshal List<string> Reason_phrase -> Native Unmanaged Array of char* (char**)
            IntPtr[] reasonPhrasePtrArray = new IntPtr[TotalRecords];
            for (int i = 0; i < TotalRecords; i++)
            {
                reasonPhrasePtrArray[i] = Marshal.StringToHGlobalAnsi(Reason_phrase[i]);
            }

            GCHandle reasonHandle = GCHandle.Alloc(reasonPhrasePtrArray, GCHandleType.Pinned);

            try
            {
                // 4. Fill the Packet Struct Memory Addresses
                packet.domain = nativeDomainPtr;
                packet.status_code = statusHandle.AddrOfPinnedObject();
                packet.latencies = latencyHandle.AddrOfPinnedObject();
                packet.reason_phrase = reasonHandle.AddrOfPinnedObject();
                packet.total_records = TotalRecords;

                // 5. Fire off to C++ Execution Pipeline!
                compile_telemetry_save_file(packet);
            }
            finally
            {
                // 6. Memory Cleanup Block (Prevents Unmanaged RAM Leaks!)
                Marshal.FreeHGlobal(nativeDomainPtr);

                for (int i = 0; i < TotalRecords; i++)
                {
                    Marshal.FreeHGlobal(reasonPhrasePtrArray[i]);
                }

                if (reasonHandle.IsAllocated) reasonHandle.Free();
                if (statusHandle.IsAllocated) statusHandle.Free();
                if (latencyHandle.IsAllocated) latencyHandle.Free();
            }
        }
    }
}