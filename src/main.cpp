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

    print_graph_cout(graphe.GPCC.Graphe);

    graphe.Resolution(version);


    return 0;
}