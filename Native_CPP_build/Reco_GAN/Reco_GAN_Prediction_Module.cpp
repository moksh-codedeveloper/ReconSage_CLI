#include <cstring>
#include <sstream>
#include <vector>
#include "Reco_GAN_Struct.cpp"
#include <iostream>
#include <fstream>
#include <cmath>
using namespace std;

class Reco_GAN_Prediction_Module
{
private:
    char domain[256];
    double k_factor;

public:
    Reco_GAN_Prediction_Module(char _domain[256], double _k_factor)
    {
        strncpy(domain, _domain, 256);
        k_factor = _k_factor;
    }
};