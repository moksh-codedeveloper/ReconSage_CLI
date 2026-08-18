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
namespace RfoModel
{
    public class RfoParsedModel
    {
        public string Target { set; get; } = string.Empty;
        public int Port { set; get; }
        public string Password { set; get; } = string.Empty;
        public string tor_ip { set; get; } = string.Empty;
        public string Proto_port { set; get; } = string.Empty;
        public int tor_port { set; get; }
        public string dns_server{set; get;} = string.Empty;
    }
}