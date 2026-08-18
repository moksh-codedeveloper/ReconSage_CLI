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
using System;
using System.Runtime.InteropServices;
using IParser;
using ResoModel;

namespace ResoParser
{
    // Mirror of C++ struct — field order and boundary limits match EXACTLY
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    internal struct CppRsoParserConfig
    {
        public int timeout;
        public int delay;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 800)]
        public string wordlist_path;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 800)]
        public string json_file_name;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 800)]
        public string headers_file;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 800)]
        public string html_file;
    }

    public class RsoParser : IFileParser<RModel>
    {
        // P/Invoke bridge updated to handle native execution over the unmanaged stack frame
        [DllImport("parser_cpp_module.so", CallingConvention = CallingConvention.Cdecl)]
        private static extern CppRsoParserConfig parse_config(string filename);

        public string RsoFilePath { set; get; } = string.Empty;

        public RsoParser(string filepath)
        {
            RsoFilePath = filepath;
        }

        private CppRsoParserConfig ParseViaModuleCpp()
        {
            // Allocated directly on the C# execution stack
            CppRsoParserConfig config;

            // Invoking the hardened function; returns 1 on success, 0 on validation error
            config = parse_config(RsoFilePath);
            return config;
        }

        public RModel ParseDictToModel()
        {
            CppRsoParserConfig data = ParseViaModuleCpp();

            return new RModel
            {
                Timeout = data.timeout,
                JsonFilePath = data.json_file_name,
                WordlistPath = data.wordlist_path,
                Delay = data.delay,
                HeadersFile = data.headers_file,
                HtmlFile = data.html_file
            };
        }
    }
}