#ifndef TER_SKI_HPP
#define TER_SKI_HPP 

#pragma once

#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <iostream>
#include <utility> 
#include "gurobi_c++.h" 

#include "Ford_Fulkerson.hpp"

using namespace std;

struct Arc
{
    double capacity=1;
    double flow;
    double residual;
};

typedef vector<unordered_map<int, Arc>> graph;

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

    void Resolution();


    
};
#endif 
