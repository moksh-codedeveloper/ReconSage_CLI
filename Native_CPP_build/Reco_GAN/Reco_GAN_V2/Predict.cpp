#include "Reco_GAN_Struct.cpp"
#include <iostream>
#include <vector>
#include <iomanip>
#include <cmath>
#include <fstream>
#include <cstring>
#include <cstdlib>

using namespace std;

class Reco_GAN_V2_Predict
{
private:
    char domain[256] = {0};
    int subsample_size;
    vector<vector<iTreeNodes>> forest;
    double c_factor_sub_sample;
    static constexpr double EULER_MASCHERONI = 0.5772156649;
    double calculate_c(double m) const
    {
        if (m <= 0.1)
            return 0.0;
        else if (m == 2.0)
            return 0.1;
        return 2.0 * (log(m - 1.0) + EULER_MASCHERONI) - (2.0 * (m - 1.0) / m);
    }
    double pathLength(const vector<iTreeNodes> &trees, int node_idx, double latency_x, double current_depth) const
    {
        if (node_idx < 1 && node_idx >= static_cast<int>(trees.size()))
            return current_depth;
        const iTreeNodes &nodes = trees[node_idx];
        if (nodes.is_leaf)
            return current_depth + calculate_c(static_cast<int>(trees.size()));
        if (latency_x < nodes.split_value)
            return pathLength(trees, nodes.left_child, latency_x, current_depth + 1.0);
        else
            return pathLength(trees, nodes.right_child, latency_x, current_depth + 1.0);
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
    double Score(double live_latency) const
    {
        if (forest.empty())
            return 0.0;
        double total_path = 0.0;
        for (const auto &trees : forest)
        {
            total_path += pathLength(trees, 0, live_latency, 0.0);
        }
        double mean_path = total_path / static_cast<double>(forest.size());
        return pow(2.0, -(mean_path / c_factor_sub_sample));
    }

public:
    Reco_GAN_V2_Predict(char _domain[256], int s_sample) : subsample_size(s_sample)
    {
        strncpy(domain, _domain, 255);
        domain[255] = '\0';
        c_factor_sub_sample = calculate_c(static_cast<double>(subsample_size));
    }
    void LoadModel()
    {
        // vector<vector<iTreeNodes>> forest;
        forest.clear();
        char file_path[512] = {0};
        buildFullFilePath(file_path);

        ifstream file(file_path);
        if (!file.is_open())
        {
            cerr << "[ERROR] Prediction engine could not open: " << file_path << endl;
            return;
        }

        string tag;
        while (file >> tag)
        {
            if (tag == "TREE_START")
            {
                size_t node_count = 0;
                file >> node_count;

                vector<iTreeNodes> tree;
                tree.reserve(node_count);

                for (size_t i = 0; i < node_count; ++i)
                {
                    iTreeNodes node;
                    file >> node.left_child >> node.right_child >> node.is_leaf >> node.size >> node.split_value;
                    tree.push_back(node);
                }

                file >> tag; // Consume "TREE_END"
                forest.push_back(tree);
            }
        }
        file.close();
        return;
    }

    vector<double> Score_List(vector<double> latency_list)
    {
        vector<double> scores;
        for (const double &latency : latency_list)
        {
            double score = Score(latency);
            scores.push_back(score);
        }
        return scores;
    }
};

extern "C"
{
    void reco_Gan_V2_predict(char domain[256], int subsample_size, double *latency_list, int latency_size)
    {
        Reco_GAN_V2_Predict reco_gan(domain, subsample_size);
        printf("Loading the Model......");
        reco_gan.LoadModel();
        vector<double> latency_vector(latency_list, latency_list + latency_size);
        printf("Calculating the scores from the live fresh data");
        vector<double> scores_list = reco_gan.Score_List(latency_vector);
        for (const auto &score : scores_list)
        {
            if (score <= 0.5)
                printf("NORMAL");
            else if (score > 0.5 && score <= 0.7)
                printf("Industry Standard");
            else if (score >= 0.8)
                printf("ANOMALY");
            else if (score >= 0.7 && score < 0.8)
                printf("SUSSY");
        }
        printf("Prediction ends Model Forest No.0x00 commencing mov rax, 60 and mov rdi, 0");
    }
}