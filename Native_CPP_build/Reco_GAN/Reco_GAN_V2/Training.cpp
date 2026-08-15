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
    vector<vector<iTreeNodes>> forests;
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
            user_name = "default";

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
    TrainingModule(const char *_domain, int s_sample = 256, int n_trees = 100)
        : num_trees(n_trees), subsample_size(s_sample), rng(random_device{}())
    {
        max_depth = static_cast<int>(ceil(log2(max(2, subsample_size))));
        strncpy(domain, _domain, 255);
        domain[255] = '\0';
    }

    void Train(vector<double> &latency_dataset)
    {
        forests.clear();
        if (latency_dataset.empty())
            return;

        for (int i = 0; i < num_trees; i++)
        {
            vector<iTreeNodes> tree;
            buildTreeRecursive(tree, latency_dataset, 0);
            forests.push_back(tree);
        }
    }

    void writeTreeTrainingData()
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