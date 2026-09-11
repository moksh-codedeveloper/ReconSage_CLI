#pragma once

#include "Reco_GAN_Struct.cpp"
#include <vector>

class Reco_GAN_V2_Predict
{
private:
    char domain[256] = {0};
    int subsample_size;
    std::vector<std::vector<iTreeNodes>> forest;
    double c_factor_sub_sample;
    static constexpr double EULER_MASCHERONI = 0.5772156649;

    double calculate_c(double m) const;
    double pathLength(const std::vector<iTreeNodes> &trees, int node_idx, double latency_x, double current_depth) const;
    void buildFullFilePath(char out_path[512]);
    double Score(double live_latency) const;

public:
    Reco_GAN_V2_Predict(const char *_domain, int s_sample);

    bool LoadModel();
    std::vector<double> Score_List(const std::vector<double> &latency_list) const;
};