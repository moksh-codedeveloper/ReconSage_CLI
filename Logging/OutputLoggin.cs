namespace ReconSageLogger
{
    public static class Logger
    {
        public static void Info(string message)
        {
            Console.ForegroundColor = ConsoleColor.Cyan;
            Console.Write("[INFO] ");
            Console.ResetColor();
            Console.WriteLine(message);
        }

        public static void Warn(string message)
        {
            Console.ForegroundColor = ConsoleColor.Yellow;
            Console.Write("[WARN] ");
            Console.ResetColor();
            Console.WriteLine(message);
        }

        public static void Error(string message)
        {
            Console.ForegroundColor = ConsoleColor.Red;
            Console.Write("[ERROR] ");
            Console.ResetColor();
            Console.WriteLine(message);
        }

        public static void Success(string message)
        {
            Console.ForegroundColor = ConsoleColor.Green;
            Console.Write("[SUCCESS] ");
            Console.ResetColor();
            Console.WriteLine(message);
        }

        public static void Scan(string message)
        {
            Console.ForegroundColor = ConsoleColor.Magenta;
            Console.Write("[SCAN] ");
            Console.ResetColor();
            Console.WriteLine(message);
        }

        public static void Rotate(string message)
        {
            Console.ForegroundColor = ConsoleColor.DarkYellow;
            Console.Write("[ROTATE] ");
            Console.ResetColor();
            Console.WriteLine(message);
        }

        public static void Done(string message)
        {
            Console.ForegroundColor = ConsoleColor.Green;
            Console.Write("[DONE] ");
            Console.ResetColor();
            Console.WriteLine(message);
        }
    }
}