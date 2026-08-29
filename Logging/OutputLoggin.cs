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
namespace ReconSageLogger
{
    public static class Logger
    {
        public static void Info(string message)
        {
            Console.ForegroundColor = ConsoleColor.Blue;
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
            Console.ForegroundColor = ConsoleColor.DarkMagenta;
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