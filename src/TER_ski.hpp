#pragma once

//#include <fstream>
//#include <vector>
//#include <string>
#include <unordered_map>
//#include <iostream>
#include <utility>
#include <sstream>
#include "gurobi_c++.h" 

#include "PCC_successifs.hpp"



/*struct Arc
{
    double capacity=1;
    double flow;
    double residual;
    double cost;
    double costPi;
};*/

typedef vector<unordered_map<int, Arc>> graph;



class TER_ski
{
public:
    int size;
    graph Graphe;
    vector<pair<int, int>> doublons;
    graph GFord;
    vector<int> ordreTopologique;
    graphCM GPCC;

    TER_ski(string filename);

    void triTopologique();

    // Modifie GFord pour avoir son graphe restreint aux xij = 0; avec les sommets dédoublés
    void restreindre_graphe_FF(const vector<vector<double>> &Xij);

    void Resolution(int version);


    
};
