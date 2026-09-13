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
#include "message_converter.hpp"
MessageToTensar::MessageToTensar(vector<string> _reason_phrases)
{
    reason_phrases = _reason_phrases;
}

vector<vector<double>> MessageToTensar::CompileBatch()
{
    vector<vector<double>> multidimensional_stash;
    vector<double> compiled_vector;

    for (const string &phrase : reason_phrases)
    {
        int phrase_len = phrase.length();

        for (int it = 0; it < phrase_len; it++)
        {
            int val = static_cast<int>(phrase[it]);
            compiled_vector.push_back(val / 127.0);
        }

        // Push this 1D row into our 2D multi-dimensional tensor matrix
        multidimensional_stash.push_back(compiled_vector);
    }

    return multidimensional_stash;
}