using IParser;
using ResoModel;
using System.Runtime.InteropServices;

namespace ResoParser
{
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    internal struct CppRsoParserConfig
    {
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
        public string target;
        public int timeout;  // timeout first
        public int delay;    // delay second
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 800)]
        public string wordlist_path;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 800)]
        public string json_file_name;
    }
    public class RsoParser : IFileParser<RModel>
    {
        [DllImport("parser_cpp_module.so", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr parse_config(string filename);

        [DllImport("parser_cpp_module.so", CallingConvention = CallingConvention.Cdecl)]
        private static extern void free_module(IntPtr config);
        public string RsoFilePath { set; get; } = string.Empty;
        public RsoParser(string filepath)
        {
            RsoFilePath = filepath;
        }
        private CppRsoParserConfig ParseViaModuleCpp()
        {
            IntPtr ptr = parse_config(RsoFilePath);
            if (ptr == IntPtr.Zero)
                throw new Exception("C++ parser failed to parse the .rfo file");

            CppRsoParserConfig config = Marshal.PtrToStructure<CppRsoParserConfig>(ptr);
            free_module(ptr);
            return config;
        }

        public RModel ParseDictToModel()
        {
            CppRsoParserConfig data = ParseViaModuleCpp();
            return new RModel
            {
                Target = data.target,
                Timeout = data.timeout,
                JsonFilePath = data.json_file_name,
                WordlistPath = data.wordlist_path,
                Delay = data.delay,
            };
        }
    }
}