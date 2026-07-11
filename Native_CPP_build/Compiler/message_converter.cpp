#include <vector>
#include <string>
#include <cstring>
#include <fstream>   // Added strictly for raw file saving

using namespace std;

class MessageToTensar {
private: 
    // Fixed the landmine: We use a vector of strings for input batches
    vector<string> reason_phrases;
    const int TENSOR_SIZE = 12; // Enforcing fixed size for Reco_GAN

public:
    // Constructor taking a batch of phrases
    MessageToTensar(vector<string> _reason_phrases) {
        reason_phrases = _reason_phrases;
    }

    // 1. MULTI-DIMENSIONAL CONVERSION
    // Compiles the entire batch into a 2D Matrix Tensor: vector<vector<double>>
    vector<vector<double>> CompileBatch() {
        vector<vector<double>> multidimensional_stash;

        for (const string& phrase : reason_phrases) {
            // Pre-allocate a fixed-size 1D vector filled with 0.0 padding
            vector<double> compiled_vector(TENSOR_SIZE, 0.0);
            int phrase_len = phrase.length();

            for (int it = 0; it < phrase_len && it < TENSOR_SIZE; it++) {
                int val = static_cast<int>(phrase[it]);
                compiled_vector[it] = val / 127.0;
            }

            // Push this 1D row into our 2D multi-dimensional tensor matrix
            multidimensional_stash.push_back(compiled_vector);
        }

        return multidimensional_stash;
    }

    // 2. FILE SAVING SYSTEM
    // Dumps the multidimensional tensor into a lightweight text file stash
    void SaveToFile(const string& filename, const vector<vector<double>>& tensor_matrix) {
        ofstream stash_file(filename);
        
        if (stash_file.is_open()) {
            for (const auto& row : tensor_matrix) {
                for (int i = 0; i < TENSOR_SIZE; i++) {
                    stash_file << row[i];
                    // Separate numbers with a space, but skip space on the last element
                    if (i < TENSOR_SIZE - 1) stash_file << " ";
                }
                stash_file << "\n"; // New line for the next log row
            }
            stash_file.close();
        }
    }
};