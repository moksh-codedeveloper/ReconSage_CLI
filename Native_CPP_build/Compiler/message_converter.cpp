#include <vector>
#include <string>
#include <cstring>
#include <fstream> // Added strictly for raw file saving
#include <sstream>
#include <iostream>
using namespace std;

class MessageToTensar
{
private:
    // Fixed the landmine: We use a vector of strings for input batches
    vector<string> reason_phrases;
    const int TENSOR_SIZE = 12; // Enforcing fixed size for Reco_GAN

public:
    // Constructor taking a batch of phrases
    MessageToTensar(vector<string> _reason_phrases)
    {
        reason_phrases = _reason_phrases;
    }

    vector<vector<double>> CompileBatch()
    {
        vector<vector<double>> multidimensional_stash;
        vector<double> compiled_vector;

        for (const string &phrase : reason_phrases)
        {
            int phrase_len = phrase.length();

            for (int it = 0; it < phrase_len; it++)
            {
                int val = static_cast<int>(phrase[it]);
                compiled_vector.push_back( val / 127.0);
            }

            // Push this 1D row into our 2D multi-dimensional tensor matrix
            multidimensional_stash.push_back(compiled_vector);
        }

        return multidimensional_stash;
    }

    // void SaveToFile(const string &filename, const vector<vector<double>> &tensor_matrix)
    // {
    //     ofstream stash_file(filename);

    //     if (stash_file.is_open())
    //     {
    //         for (const auto &row : tensor_matrix)
    //         {
    //             for (int i = 0; i < TENSOR_SIZE; i++)
    //             {
    //                 stash_file << row[i];
    //                 if (i < TENSOR_SIZE - 1)
    //                     stash_file << " ";
    //             }
    //             stash_file << "\n"; // New line for the next log row
    //         }
    //         stash_file.close();
    //     }
    // }

    // vector<vector<double>> LoadDataset(const string &filename)
    // {
    //     vector<vector<double>> matrix_stash;
    //     ifstream stash_file(filename);

    //     if (!stash_file.is_open())
    //     {
    //         cerr << "[Reco_GAN ERROR] :: Failed to open tensor stash file: " << filename << endl;
    //         return matrix_stash;
    //     }

    //     string line;
    //     // Read line-by-line (Row by Row) to preserve low-power performance
    //     while (getline(stash_file, line))
    //     {
    //         // Skip empty lines safely
    //         if (line.empty())
    //             continue;

    //         vector<double> row_vector;
    //         stringstream row_stream(line);
    //         double feature_val;

    //         // Stream each space-separated float out of the current line
    //         while (row_stream >> feature_val)
    //         {
    //             row_vector.push_back(feature_val);
    //         }

    //         // Integrity Check: Ensure the row matches your exact expected vector size
    //         if (row_vector.size() == TENSOR_SIZE)
    //         {
    //             matrix_stash.push_back(row_vector);
    //         }
    //     }

    //     stash_file.close();
    //     return matrix_stash;
    // }
};