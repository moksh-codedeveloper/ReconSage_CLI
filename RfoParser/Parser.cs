using System.Runtime.InteropServices;
using IParser;
using RfoModel;

namespace TorConfigParser
{
    // Mirror of C++ struct — field order must match EXACTLY
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    internal struct CppParserConfig
    {
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
        public string target;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 800)]
        public string json_file_name;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 800)]
        public string wordlist_path;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
        public string host;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
        public string password;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
        public string tor_ip;
        public ushort port;      // uint16_t
        public ushort tor_port;  // uint16_t ← pehle
        public byte timeout;     // uint8_t
        public byte delay;       // uint8_t ← baad mein
    }

    public class RfoParser : IFileParser<RfoParsedModel>
    {
        // P/Invoke bridge
        [DllImport("parser_cpp_module.so", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr parse_rfo(string filename);

        [DllImport("parser_cpp_module.so", CallingConvention = CallingConvention.Cdecl)]
        private static extern void free_parser(IntPtr config);

        public string _filepath { set; get; } = string.Empty;

        public RfoParser(string filepath)
        {
            _filepath = filepath;
        }

        private CppParserConfig ParseViaCpp()
        {
            IntPtr ptr = parse_rfo(_filepath);
            if (ptr == IntPtr.Zero)
                throw new Exception("C++ parser failed to parse the .rfo file");

            CppParserConfig config = Marshal.PtrToStructure<CppParserConfig>(ptr);
            free_parser(ptr);
            return config;
        }

        public RfoParsedModel ParseDictToModel()
        {
            CppParserConfig parsed = ParseViaCpp();

            RfoParsedModel parsedModel = new RfoParsedModel();
            parsedModel.Target = parsed.target;
            parsedModel.Timeout = parsed.timeout;
            parsedModel.JsonFilePath = parsed.json_file_name;
            parsedModel.WordlistPath = parsed.wordlist_path;
            parsedModel.Port = parsed.port;
            parsedModel.host = parsed.host;
            parsedModel.Password = parsed.password;
            parsedModel.tor_ip = parsed.tor_ip;
            parsedModel.tor_port = parsed.tor_port;
            parsedModel.delay = parsed.delay;

            return parsedModel;
        }
    }
}