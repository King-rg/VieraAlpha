#pragma once

#include <iostream>
#include <vector>

#include "encoder.h"

using namespace std;

struct config
{
    int colAmount = 20;
    int nodeAmount = 10;


    int proxAmount = 150;

    int proxActivationThreshold = 5;
    int distalActivationThreshold = 5;
    int proxTerminationThreshold = 20;
    int distalTerminationThreshold = 20;
};

class conn
{
public:
    config conf;

    string id;

    vector<int> coord;
    int strength; // 1-100

    conn(string type, SDR input);
};

class procnode
{
public:
    config conf;

    string id;
    bool predictive = false;
    bool active = false;
    int predictionThreshold = conf.distalActivationThreshold;

    vector<conn> distconns;

    procnode(SDR input);

};

class proccol
{
public:
    config conf;

    string id;
    int connamount = conf.proxAmount;
    int childamount = conf.nodeAmount;
    int activationThreshold = conf.proxActivationThreshold;

    vector<vector<bool>> prevmap;
    vector<procnode> nodes;
    vector<conn> proxconns;

    proccol(SDR input);
    vector<bool> process_proximal(SDR input);
    void process_distal(vector<vector<bool>> map, SDR input);
};

class procunit
{
public:
    config conf;

    string id;
    int childamount = conf.colAmount;
    SDR internalInput; 

    vector<proccol> columns;

    procunit(SDR input);
    void cycle(SDR input);
};
