#include "TER_ski.hpp"


TER_ski::TER_ski(string filename)
    {
        ifstream inf(filename);
        if (!inf)
        {
            size = -1;
            return;
        }

        string temp;
        int numNodes = -1;
        while (inf >> temp)
        {
            if (temp == "NODES")
            {
                inf >> numNodes;
                Graphe.resize(numNodes);
            }
            else if (temp == "LINKS")
            {
                int node1, node2;
                double capacity = 1.0;
                while (inf >> node1 >> node2)
                {
                    if (Graphe[node1].find(node2) != Graphe[node1].end())
                    {
                        doublons.emplace_back(node1, node2);
                    }
                    else
                    {
                        Arc arc;
                        arc.capacity = capacity;
                        arc.flow = 0.0;
                        arc.residual = capacity;
                        Graphe[node1][node2] = arc;
                    }
                }
            }
        }
        size = numNodes;
        inf.close();
    }



void TER_ski::restreindre_graphe(const vector<vector<double>> &Xij)        // Modifie GFord pour avoir son graphe restreint aux xij = 0; avec les sommets dédoublés
    {
        int size = Graphe.size();
        GFord.resize(0);
        GFord.resize(2*size);
        double capacity = 1.;
        
        for (int i=0; i<size; i++){
            Arc a = {capacity, 0., capacity};
            GFord[i][i+size] = a;
            for (auto &[s, arc] : Graphe[i]){
                if (Xij[i][s] < 0.5){
                    GFord[size + i][s] = arc;
                }
            }
        }
        symmetrize(GFord);
    }
