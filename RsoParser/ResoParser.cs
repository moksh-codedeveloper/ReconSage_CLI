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
        private static extern int parse_config(string filename, out CppRsoParserConfig outModel);

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
            int result = parse_config(RsoFilePath, out config);
            
            if (result == 0)
                throw new Exception("C++ parser failed to parse the .rso file");

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