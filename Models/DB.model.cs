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
namespace DBModel
{
    public class Model
    {
        public int Id{set;get;}
        public string Target{set;get;} = string.Empty;
        public string JsonFilePath{set;get;} = string.Empty;
        public string HeadersFile{set;get;} = string.Empty;
        public string WordlistsPath{set;get;} = string.Empty;
        public string HtmlFilePath{set;get;} = string.Empty;
        public string ReasonPhrase{set;get;} = string.Empty;
        public int StatusCode{set;get;}
        public double LatencyMs{set;get;}
    }

    public class CompilerDataModel
    {
        public List<string> ReasonPhrase{set;get;} = new();
        public string Domain{set;get;} = string.Empty;
        public List<double> LatencyList{set;get;} = new();
        public List<int> StatusCodes{set;get;} = new();
        public int TotalRecords{set;get;}
    }
    public class MainModel
    {
        public List<Model> packet{set;get;} = new();
    }
}