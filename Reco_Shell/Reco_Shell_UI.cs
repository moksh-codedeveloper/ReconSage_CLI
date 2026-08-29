using Commands.RecoShell;
using SessionData_RecoShell;

namespace ReconSageShell.UI
{
    public static class ShellUi
    {
        public static void RenderBanner(SessionData sessionData)
        {
            Console.Clear();
            Console.ForegroundColor = ConsoleColor.Cyan;
            Console.WriteLine(@"
    __________  ________________  _____  ___ __________ 
    ___  __ \_  / __  __ \_  __ \__  / / /_  ____/_  _ \
    __  /_/ /  /  _  / / /  / / /_  /_/ /_  / __ _  / / /
    _  _, _// /___/ /_/ // /_/ /_  __  / / /_/ / / /_/ / 
    /_/ |_|/_____/\____/ /____/ /_/ /_/  \____/  \____/  
                                         v2.0 [ARCH-LINUX]");

            Console.ResetColor();
            Console.WriteLine(new string('-', 62));
            Console.ForegroundColor = ConsoleColor.DarkGray;
            Console.WriteLine("=[ " + ConsoleColor.Yellow + "ReconSage | Advanced Telemetry Framework" + ConsoleColor.DarkGray + " ]=");
            Console.WriteLine("+ -- --=[ Status: " + FormatPill(sessionData.isRsoLoaded, "RSO") + " | " +
                              FormatPill(sessionData.isRfoLoaded, "RFO") + " | " +
                              FormatPill(sessionData.isRxoLoaded, "RXO") + " ]=");
            Console.ResetColor();
            Console.WriteLine(new string('-', 62) + "\n");
        }

        public static void RenderPrompt(SessionData sessionData)
        {
            Console.ForegroundColor = ConsoleColor.Blue;
            Console.Write("reconsage");

            if (sessionData.isRfoLoaded && sessionData.rfoModel != null)
            {
                Console.ForegroundColor = ConsoleColor.White;
                Console.Write(" (");
                Console.ForegroundColor = ConsoleColor.Red;
                Console.Write(sessionData.rfoModel.Target);
                Console.ForegroundColor = ConsoleColor.White;
                Console.Write(")");
            }

            Console.ForegroundColor = ConsoleColor.DarkGray;
            Console.Write(" > ");
            Console.ResetColor();
        }

        public static void RenderHelp(IEnumerable<ShellCommand> commands)
        {
            Console.ForegroundColor = ConsoleColor.Yellow;
            Console.WriteLine("\nAvailable Commands");
            Console.WriteLine("==================\n");
            Console.ResetColor();

            Console.WriteLine($"  {"Command",-42} {"Description"}");
            Console.WriteLine($"  {new string('-', 40),-42} {new string('-', 35)}");

            string currentCategory = "";
            foreach (var cmd in commands)
            {
                if (cmd.Category != currentCategory)
                {
                    currentCategory = cmd.Category;
                    Console.ForegroundColor = ConsoleColor.Cyan;
                    Console.WriteLine($"\n[{currentCategory}]");
                    Console.ResetColor();
                }
                Console.WriteLine($"  {cmd.Name,-42} {cmd.Description}");
            }
            Console.WriteLine();
        }

        private static string FormatPill(bool isLoaded, string label) =>
            isLoaded ? $"\u001b[32m{label}:LOADED\u001b[0m" : $"\u001b[31m{label}:NONE\u001b[0m";
    }
}