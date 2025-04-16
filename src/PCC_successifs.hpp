#pragma once


#include <iostream>
#include <cmath>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <unordered_set>
#include <set>
#include <stack>
#include <algorithm>

using namespace std;

typedef unordered_set<int> Set;

struct Arc {
    double capacity;
    double flow;
    double residual;
    double cost;
    double costPi;
};

typedef vector<unordered_map<int, Arc>> graph;



class graphCM
{
    public:

        // Attributs

        graph Graphe;
        vector<double> supply;


        // Méthodes

        //graph make_empty_graph(int n);      // Crée un graphe vide de taille n

        //bool is_valid_index(graph &G, int i);       // Vérifie que le sommet i appartient au graphe G

        //void add_Arc(graph &G, int begin, int end, double capacity, double cost);       // Ajoute un arc au graphe G

        void symmetrize();      // Symmétrise le graphe

        //graph read_graph(string filename);      // Renvoie un graphe à partir d'un fichier
            
        void print_graph();         // Affiche le graphe G

        void reset_graph(int a, int b);
        void reset_graph();

        void ajouter_couts(const vector<vector<double>> &Xij);

        int indCoutMin(unordered_set<int> &noeuds, vector<double> &distances);     // Renvoie l'indice du noeud temporaire dont la valeur dans le vecteur de distances est minimum

        int Dijsktra (int k, vector <double> &d, vector <int> &pred, Set &D);     // Calcule les PCC de k aux autres sommets selon leur coût réduit, renvoie un noeud en déficit accessible, -1 sinon

        void augmenter_flot(int k, int l, double delta, vector<int> &pred);   // Augmente le flot x de delta unités le long du chemin indiqué par pred

        void miseAJour(int k, int l, stack<int> &E, Set &D,vector<double> &exces, vector <double> &pi, double delta);    // Met à jour le graphe suite à l'augmentation de flot

        void miseAJour(int s, int t, vector<double> &exces, vector <double> &pi, double delta, const vector<int> &Topo);    // Met à jour le graphe suite à l'augmentation de flot

        double PCC_successifs ();      // Applique l'algorithme des plus courts chemins successifs sur le graphe

        bool Dijsktra (int s, int t, vector <double> &d, vector <int> &pred, const vector<int> &Topo); // PCC de a à b

        bool Bellman (int s, int t, vector<double> &d, vector<int> &pred, const vector<int> &Topo);

        double PCC_successifs (int s, int t, const vector<int> &Topo);      // Applique l'algorithme des plus courts chemins successifs sur le graphe


};
