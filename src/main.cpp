#include "TER_ski.hpp"
#include "Ford_Fulkerson.hpp" 

int main(){


    string filename = "data/test.txt";
    TER_ski graphe(filename);

    /*int n = graphe.size;
    for (int i=0; i<n; i++){
        cout << "Voisins de " << i << " :" << endl;
        for (auto &pair : graphe.Graphe[i]){
            cout << "   " << pair.first << " : " << pair.second.capacity << endl;
        }
    }*/
    print_graph(graphe.Graphe);
    symmetrize(graphe.Graphe);
    print_graph(graphe.Graphe);


    /*cout << "Ordre Topologique : " << endl;
    for (int i=0; i<graphe.size; i++){
        cout << graphe.ordreTopologique[i] << "; ";
    }
    cout << endl;

    graphe.Resolution();

    */



    return 0;
}
