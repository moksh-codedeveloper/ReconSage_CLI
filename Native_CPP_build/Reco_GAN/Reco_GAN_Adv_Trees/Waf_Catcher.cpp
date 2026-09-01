#include <iostream>
#include <vector>
#include <algorithm>
#include <cstring>
#include "Soft_404_Catcher.cpp"
#include "Reco_GAN_Struct.cpp"

using namespace std;

class WafCatcher
{
private:
    char responseBodyFilePath[512];
    vector<double> ScoresList;

    void ScoresSort(ScoresStruct &scoreObj)
    {
        for (const auto &dataPoint : ScoresList)
        {
            if (dataPoint <= 0.50)
                scoreObj.Normal.push_back(dataPoint);
            else if (dataPoint <= 0.65)
                scoreObj.Standard.push_back(dataPoint);
            else if (dataPoint < 0.80)
                scoreObj.Suspicious.push_back(dataPoint);
            else
                scoreObj.Anomaly.push_back(dataPoint); // Renamed to Anomaly for clarity
        }
    }

public:
    WafCatcher(const vector<double> &_scoresList, const char *_responseBodyFilePath)
        : ScoresList(_scoresList)
    {
        strncpy(responseBodyFilePath, _responseBodyFilePath, 511);
        responseBodyFilePath[511] = '\0';
    }

    void WafHit()
    {
        ScoresStruct scoreSort;
        ScoresSort(scoreSort);

        Soft_404_Catcher _404_engine(responseBodyFilePath);
        vector<bool> mainSoft404sList = _404_engine.mainSoft404();

        // 1. Tally actual positive hits instead of vector length
        size_t soft404Hits = count(mainSoft404sList.begin(), mainSoft404sList.end(), true);
        size_t anomalyLatencyHits = scoreSort.Anomaly.size();
        size_t suspiciousLatencyHits = scoreSort.Suspicious.size();

        // 2. Correlation Matrix
        if (anomalyLatencyHits >= 2 && soft404Hits >= 2)
        {
            cout << "[CRITICAL] WAF BLOCK CONFIRMED: High latency tarpits + structural challenge/soft-404 walls detected." << endl;
        }
        else if (anomalyLatencyHits >= 1 && soft404Hits >= 1)
        {
            cout << "[WARNING] Potential WAF tripwire hit on at least one probed route." << endl;
        }
        else if (anomalyLatencyHits >= 2 && soft404Hits == 0)
        {
            cout << "[NOTICE] Backend throttling / rate-limiting active (Latency spikes without structural changes)." << endl;
        }
        else if (anomalyLatencyHits == 0 && soft404Hits >= 2)
        {
            cout << "[NOTICE] Custom Soft-404 or SPA catch-all route detected (No latency penalty)." << endl;
        }
    }
};

extern "C"
{
    void mainWafCatchingEngine(double *scorePointer, int scoreListSize, const char *responseBodyFilePath)
    {
        vector<double> scoreList(scorePointer, scorePointer + scoreListSize);
        WafCatcher wafEngine(scoreList, responseBodyFilePath);
        wafEngine.WafHit();
    }
}