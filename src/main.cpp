#include "TER_ski.hpp"
#include "Ford_Fulkerson.hpp" 

int main(int argc, char *argv[]){

    if (argc==1){

        

    }

    if (argc < 3){
        cerr << "Passez un fichier et une version en argument!" << endl;
        return 1;
    }
    string filename = argv[1];
    int version = stoi(argv[2]);
    filename = "data/" + filename;
    TER_ski graphe(filename);
    graphe.Detection_Flot();
    
    //print_graph_cout(graphe.GPCC.Graphe);

    /*graphCM Gtest;
    Gtest.Graphe = graphe.GPCC.Graphe;
    Gtest.supply.resize(18,0);
    Gtest.supply[9] = 2;
    Gtest.supply[4] = -2;
    
    vector<vector<double>> x(9, vector<double> (9));
    vector<int> topo(9);
    for (int i=0; i<9; i++){
        topo[i] = i;
    }
    x[1][3] = 0.5;
    x[2][3] = 0.5;
    x[3][4] = 0.5;
    x[3][5] = 0.5;
    x[5][7] = 0.5;
    x[6][7] = 0.5;
    x[5][8] = 0.5;
    x[6][8] = 0.5;
    
    Gtest.ajouter_couts(x);
    print_graph(Gtest.Graphe);
    print_graph_cout(Gtest.Graphe);
    double cost = Gtest.PCC_successifs(0,4,topo);
    cout << cost << endl;*/

    //vector<pair<int, int>> capteurs = graphe.Resolution(version);
    //bool checkGraph = graphe.checker(capteurs);
    //cout << (checkGraph ? "Le graphe passe le checker!" : "Le graphe ne passe pas le checker!" ) << endl;

    return 0;
}