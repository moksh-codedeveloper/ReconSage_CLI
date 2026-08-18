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

namespace Struct
{
    // Remove Pack = 1 to allow natural 8-byte alignment match
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    internal struct CppScanOutput
    {
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 3072)]
        public string domain;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 65536)]
        public string response_headers;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
        public string reason_phrase;

        public int status_code;
        public double latency_ms; // Will now sit at the correct byte offset position
    }
}