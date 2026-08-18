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
using CompilerModel;
using DBModel;

namespace Reco_novich_compiler
{
    public class Reco_novich
    {
        [DllImport("reco_novich_compiler_cpp_module.so", CallingConvention = CallingConvention.Cdecl)]
        private static extern void compile_telemetry_save_file(CompilerDataPacket data_packet);
        public void CompileAndSave(CompilerDataModel model)
        {
            CompilerDataPacket packet = new CompilerDataPacket();

            // 1. Marshal Domain (C# String -> Native Unmanaged ANSI char*)
            IntPtr nativeDomainPtr = Marshal.StringToHGlobalAnsi(model.Domain);

            // 2. Marshal Primitive Lists (Convert to Array and Pin in Memory for zero-copy access)
            int[] statusCodeArray = model.StatusCodes.ToArray();
            double[] latencyArray = model.LatencyList.ToArray();

            GCHandle statusHandle = GCHandle.Alloc(statusCodeArray, GCHandleType.Pinned);
            GCHandle latencyHandle = GCHandle.Alloc(latencyArray, GCHandleType.Pinned);

            // 3. Marshal List<string> Reason_phrase -> Native Unmanaged Array of char* (char**)
            IntPtr[] reasonPhrasePtrArray = new IntPtr[model.TotalRecords];
            for (int i = 0; i < model.TotalRecords; i++)
            {
                reasonPhrasePtrArray[i] = Marshal.StringToHGlobalAnsi(model.ReasonPhrase[i]);
            }

            GCHandle reasonHandle = GCHandle.Alloc(reasonPhrasePtrArray, GCHandleType.Pinned);

            try
            {
                // 4. Fill the Packet Struct Memory Addresses
                packet.domain = nativeDomainPtr;
                packet.status_code = statusHandle.AddrOfPinnedObject();
                packet.latencies = latencyHandle.AddrOfPinnedObject();
                packet.reason_phrase = reasonHandle.AddrOfPinnedObject();
                packet.total_records = model.TotalRecords;

                // 5. Fire off to C++ Execution Pipeline!
                compile_telemetry_save_file(packet);
            }
            finally
            {
                // 6. Memory Cleanup Block (Prevents Unmanaged RAM Leaks!)
                Marshal.FreeHGlobal(nativeDomainPtr);

                for (int i = 0; i < model.TotalRecords; i++)
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