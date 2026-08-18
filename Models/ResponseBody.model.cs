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

namespace ResponseBodyStruct
{
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    internal struct BodyStruct
    {
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 4096)]
        public string captured_body;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 3072)]
        public string domain;
        public int statusCode;
    }

    public class ScanResponseBodyModel
    {
        public string target{set;get;} = string.Empty;
        public string bodyResponse{set;get;} = string.Empty;
        public int statusCode{set;get;}
    }

    public class MainScanResponseBodyModel
    {
        public List<ScanResponseBodyModel> Result{get; set;} = new();
    }
}