#pragma once

#include <fstream>
//#include <vector>
//#include <string>
#include <unordered_map>
//#include <iostream>
#include <utility>
#include <sstream>
#include "gurobi_c++.h" 
#include <functional>
#include <chrono>

#include "PCC_successifs.hpp"

typedef unordered_map<int, unordered_set<int>> arcs_supprimes;

class TER_ski
{
public:
    string fichier;
    int size;
    graph Graphe;
    vector<pair<int, int>> doublons;
    graph GFord;
    vector<int> ordreTopologique;
    graphCM GPCC;
    vector<pair<int,int>> F;

    TER_ski(string filename);

    void triTopologique();

    // Modifie GFord pour avoir son graphe restreint aux xij = 0; avec les sommets dédoublés
    void restreindre_graphe_FF(const vector<vector<double>> &Xij);

    // Modifie GPCC 
    void restreindre_graphe_CM();

    vector<pair<int,int>> Resolution(int version, string filename);

    void Detection_Flot();

    vector<pair<int,int>> Resolution_compact(int version, string filename);

    bool checker(vector<pair<int,int>> capteurs);

    vector<pair<int, int>> find_solution_realisable();   


    vector<int> trouverSources();

    void dfs(int u, graph &G, vector<bool>& vu, arcs_supprimes& ignores, vector<pair<int,int>>&Capteurs, int source);

    vector<pair<int,int>> Init_Sol();

};
