#pragma once

#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <iostream>
#include <utility>

using namespace std;

struct Arc
{
    double capacity;
    double flow;
    double residual;
};

typedef vector<unordered_map<int, Arc>> graph;

#include "ff.hpp"

class TER_ski
{
public:
    int size;
    graph Graphe;
    vector<pair<int, int>> doublons;
    graph GFord;

    TER_ski(string filename);

    vector<int> triTopo();

    void restreindre_graphe(const vector<vector<double>> &Xij);        // Modifie GFord pour avoir son graphe restreint aux xij = 0; avec les sommets dédoublés

    double Resolution();


    
};
