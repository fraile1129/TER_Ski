#include "TER_ski.hpp"
#include "Ford_Fulkerson.hpp" 

int main(int argc, char *argv[]){

    if (argc < 3){
        cerr << "Passez un fichier et une version en argument!" << endl;
        return 1;
    }
    string filename = argv[1];
    int version = stoi(argv[2]);
    filename = "data/" + filename;
    TER_ski graphe(filename);
    
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

    vector<pair<int, int>> capteurs;

    
    string fres = "test.txt";

    
    if (version < 1){
        capteurs = graphe.Resolution_compact(version,fres);
    } else {
        capteurs = graphe.Resolution(version,fres);
    }
    bool checkGraph = graphe.checker(capteurs);
    cout << (checkGraph ? "Le graphe passe le checker!" : "Le graphe ne passe pas le checker!" ) << endl;
    
    
    /*capteurs = graphe.Init_Sol();
    // add the doublons
    for (const auto& doublon : graphe.doublons) {
        capteurs.push_back(doublon);
        cout << "Doublon trouvé et ajouté : (" << doublon.first << ", " << doublon.second << ")" << endl;
    }
    cout << "Capteurs installés sur les arcs suivants :" << endl;
    for (const auto& [u, v] : capteurs) {
        cout << "(" << u << ", " << v << ")" << endl;
    }*/
    

    if (graphe.checker(capteurs)) {
        cout << "Solution réalisable trouvée avec " << capteurs.size() + graphe.doublons.size() << " capteurs." << endl;
    } else {
        cout << "Échec de la validation de la solution." << endl;
    }
    return 0;
}