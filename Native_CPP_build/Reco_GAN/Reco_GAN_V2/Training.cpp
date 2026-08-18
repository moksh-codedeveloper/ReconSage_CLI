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
#include "Reco_GAN_Struct.cpp"
#include <vector>
#include <algorithm>
#include <iomanip>
#include <random>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include <iostream>

using namespace std;

class TrainingModule
{
private:
    mt19937 rng;
    int num_trees;
    int max_depth;
    int subsample_size;
    char domain[256] = {0};

    int buildTreeRecursive(vector<iTreeNodes> &tree, vector<double> &current_bucket, int depth)
    {
        int n_samples = static_cast<int>(current_bucket.size());

        // 1. Allocate node index on the flat array
        int current_node_idx = static_cast<int>(tree.size());
        tree.push_back(iTreeNodes{}); // Place placeholder in vector

        // 2. Stop condition check
        if (depth >= max_depth || n_samples <= 1)
        {
            tree[current_node_idx].is_leaf = true;
            tree[current_node_idx].size = n_samples;
            tree[current_node_idx].left_child = -1;
            tree[current_node_idx].right_child = -1;
            return current_node_idx;
        }

        auto minmax = minmax_element(current_bucket.begin(), current_bucket.end());
        double min_val = *minmax.first;
        double max_val = *minmax.second;

        if (max_val == min_val)
        {
            tree[current_node_idx].is_leaf = true;
            tree[current_node_idx].size = n_samples;
            tree[current_node_idx].left_child = -1;
            tree[current_node_idx].right_child = -1;
            return current_node_idx;
        }

        // 3. Sample random cut point (Floating-point safe)
        uniform_real_distribution<double> dist(min_val, max_val);
        double split_val = dist(rng);
        tree[current_node_idx].split_value = split_val;

        // 4. Partition bucket
        vector<double> left_child_bucket;
        vector<double> right_child_bucket;
        left_child_bucket.reserve(n_samples);
        right_child_bucket.reserve(n_samples);

        for (const double &data : current_bucket)
        {
            if (data < split_val)
            {
                left_child_bucket.push_back(data);
            }
            else
            {
                right_child_bucket.push_back(data);
            }
        }

        // 5. Recurse with depth + 1 (Prevents infinite recursion)
        int left_idx = buildTreeRecursive(tree, left_child_bucket, depth + 1);
        int right_idx = buildTreeRecursive(tree, right_child_bucket, depth + 1);

        // 6. Connect child node indices
        tree[current_node_idx].left_child = left_idx;
        tree[current_node_idx].right_child = right_idx;
        tree[current_node_idx].is_leaf = false;

        return current_node_idx;
    }

    void buildFullFilePath(char out_path[512])
    {
        const char *user_name = getenv("USER");
        if (!user_name)
            user_name = "root";

        char sanitized_domain[256] = {0};
        for (int i = 0; i < 255 && domain[i] != '\0'; ++i)
        {
            char c = domain[i];
            if (c == '.' || c == '/' || c == ':' || c == '\\')
                sanitized_domain[i] = '_';
            else
                sanitized_domain[i] = c;
        }

        snprintf(out_path, 512, "/home/%s/Reco_GAN_Data/%s_trees_data.txt", user_name, sanitized_domain);
    }

public:
    // num_trees = 100, sub_sample_size = 256 but didn't included coz let the user pass it instead of assuming it ourselves
    TrainingModule(const char *_domain, int s_sample, int n_trees)
        : num_trees(n_trees), subsample_size(max(2, s_sample)), rng(random_device{}())
    {
        max_depth = static_cast<int>(ceil(log2(max(2, subsample_size))));
        strncpy(domain, _domain, 255);
        domain[255] = '\0';
    }

    vector<vector<iTreeNodes>> Train(vector<double> &latency_dataset)
    {
        vector<vector<iTreeNodes>> forests;
        if (latency_dataset.empty())
            return forests;

        for (int i = 0; i < num_trees; i++)
        {
            // directly implemented shuffle on the latency dataset because it would be easy coz i am eventually gonna be adding the size limit in the extern functions given below
            shuffle(latency_dataset.begin(), latency_dataset.end(), rng);
            size_t sample_len = min(static_cast<size_t>(subsample_size), latency_dataset.size());
            vector<double> sample_bucket(latency_dataset.begin(), latency_dataset.begin() + sample_len);
            vector<iTreeNodes> tree;
            buildTreeRecursive(tree, sample_bucket, 0);
            forests.push_back(tree);
        }
        return forests;
    }

    void writeTreeTrainingData(vector<vector<iTreeNodes>> forests)
    {
        if (forests.empty())
            return;

        char absolute_filename[512] = {0};
        buildFullFilePath(absolute_filename);

        ofstream file(absolute_filename, ios::trunc);
        if (!file.is_open())
        {
            cerr << "[ERROR] Cannot open file for writing: " << absolute_filename << endl;
            return;
        }

        file << fixed << setprecision(4);

        // Write each tree with a clean header for easy deserialization
        for (size_t t = 0; t < forests.size(); ++t)
        {
            file << "TREE_START " << forests[t].size() << "\n";
            for (const auto &node : forests[t])
            {
                file << node.left_child << " "
                     << node.right_child << " "
                     << node.is_leaf << " "
                     << node.size << " "
                     << node.split_value << "\n";
            }
            file << "TREE_END\n";
        }
        file.close();
    }
};

extern "C"
{
    struct reco_gan_v2
    {
        char *domain;
        double *latency_data;
        int total_count;
        int sub_sample_size;
        int num_trees;
    };
    void reco_Gan_V2(reco_gan_v2 packet)
    {
        // 1. PRINT EVERYTHING IMMEDIATELY BEFORE DOING ANY MATH OR VECTORS
        cout << "--- C++ RECEIVED ---" << endl;
        cout << "Total Count: " << packet.total_count << endl;
        cout << "Sub Sample: " << packet.sub_sample_size << endl;
        cout << "Num Trees: " << packet.num_trees << endl;
        vector<double> incomingLiveLatencyDataset(packet.latency_data, packet.latency_data + packet.total_count);
        TrainingModule module(packet.domain, packet.sub_sample_size, packet.num_trees);
        cout << "[TRAINING_START C++] Firing the training module of C++" << endl;
        vector<vector<iTreeNodes>> forests = module.Train(incomingLiveLatencyDataset);
        cout << "[TRAINING_END C++] Training is done forests has been created(hopefully) now storing everything in a txt file :)" << endl;
        module.writeTreeTrainingData(forests);
        cout << "Model written to disk." << endl;
    }
}