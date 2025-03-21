#ifndef TER_SKI_HPP
#define TER_SKI_HPP 

#pragma once

#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <iostream>
#include <utility>
#include <sstream>
#include "gurobi_c++.h" 


using namespace std;

struct Arc
{
    double capacity=1;
    double flow;
    double residual;
};

typedef vector<unordered_map<int, Arc>> graph;

#include "Ford_Fulkerson.hpp"


class TER_ski
{
public:
    int size;
    graph Graphe;
    vector<pair<int, int>> doublons;
    graph GFord;
    vector<int> ordreTopologique;

    TER_ski(string filename);

    void triTopologique();

    // Modifie GFord pour avoir son graphe restreint aux xij = 0; avec les sommets dédoublés
    void restreindre_graphe(const vector<vector<double>> &Xij);

    void Resolution();


    
};
#endif 
