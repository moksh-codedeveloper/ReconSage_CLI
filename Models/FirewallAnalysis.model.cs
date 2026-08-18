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
namespace FirewallAnalysis.Model
{
    public class WebFirewallAnalysisOutput
    {
        public List<string> ListOfTarget { set; get; } = new();
        public List<double> LatencyList { set; get; } = new();
        public List<double> SpikedLatency { set; get; } = new();
        public bool isLatencyInreasing { set; get; }
        public bool isLatencyDecreasing { set; get; }
        public List<int> StatusCodeList { set; get; } = new();
        public List<int> DetectedStatusCodes { set; get; } = new();
        public List<Dictionary<string, string>> Headers { set; get; } = new();
        public List<string> Message { set; get; } = new();
    }
}