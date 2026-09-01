#include <iostream>
#include "Soft_404_Catcher.cpp"
#include "Reco_GAN_Struct.cpp"
#include <vector>
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
            {
                scoreObj.Normal.push_back(dataPoint);
            }
            else if (dataPoint <= 0.65)
            {
                scoreObj.Standard.push_back(dataPoint);
            }
            else if (dataPoint < 0.80)
            {
                scoreObj.Suspicious.push_back(dataPoint);
            }
            else
            {
                scoreObj.Anonimous.push_back(dataPoint);
            }
        }
    }

public:
    WafCatcher(vector<double> _scoresList, const char *_responseBodyFilePath)
    {
        strncpy(responseBodyFilePath, _responseBodyFilePath, 511);
        responseBodyFilePath[511] = '\0';
        ScoresList = _scoresList;
    }
    void WafHit()
    {
        ScoresStruct scoreSort;
        ScoresSort(scoreSort);
        Soft_404_Catcher _404_engine(responseBodyFilePath);
        vector<bool> mainSoft404sList = _404_engine.mainSoft404();
        if (scoreSort.Anonimous.size() == 1 && mainSoft404sList.size() == 1)
            cout << "Warning one of your paths has triggered a WAF at one point so be aware" << endl;
        else if (scoreSort.Anonimous.size() >= 2 && mainSoft404sList.size() >= 2)
            cout << "WAF HIT DETECTED BEAWARE the WAF is active......." << endl;
    }
};