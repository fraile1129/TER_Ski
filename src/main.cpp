#include "TER_ski.hpp"

int main(){


    string filename = "test.txt";
    TER_ski graphe(filename);

    vector<int> som_tri = graphe.triTopo();






    vector<vector<double>> xij;
    graphe.restreindre_graphe(xij);

    int s = 0;
    bool fin = false;
    while (s<graphe.size-1 && !fin){
        int t = s+1;
        while (t<graphe.size && !fin){
            double flow = fordfulkerson(graphe.GFord, s, t);
            if (flow > 1.5){
                fin = true;
                // ajout de contraintes
            } else {
                reset_graph(graphe.GFord);
                t++;
            }
        }
        s++;
    }






    return 0;
}
