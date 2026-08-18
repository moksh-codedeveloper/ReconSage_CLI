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
namespace RateLimitDetector.Model
{
    public class RateLimitDetectionOutputModel
    {
        public List<string> Target { set; get; } = new();
        public List<double> LatencyMS { set; get; } = new();
        public List<int> StatusCode { set; get; } = new();
        public List<int> DetectedStatusCodeList { set; get; } = new();
        public List<double> SpikedLatencyMS { set; get; } = new();
        public bool isIncreasingLat{set;get;}
        public bool isDecreasingLat{set;get;}
    }
}

