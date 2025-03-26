#include "Callback.hpp"
#include "Ford_Fulkerson.hpp"

SeparationCallback::SeparationCallback(vector<vector<GRBVar>> & x, int size,TER_ski *graphe, int version) : _x(x), _size(size), _graphe(graphe), _version(version){}

void SeparationCallback::callback () {
    try {
        if ( where == GRB_CB_MIPSOL ){        // Solution entière => Lazy Cuts
        int s = 0;
        bool fin = false;
        GRBLinExpr somme = 0;

            // On récupère la solution
            vector<vector<double>> xij(_size);
            for (int i=0 ; i<_size ; i++) {
                xij[i].resize(_size);
                for (int j=0 ; j<_size ; j++) {
                    xij[i][j] = getSolution(_x[i][j]);
                }
            }
            _graphe->restreindre_graphe(xij);
            print_graph(_graphe->GFord);
            
        while (s<_size-1 && !fin){
            int t = s+1;
            double flow = 0.;
            while (t<_size && !fin){
                cout << "Calcul flot entre " << s << " et " << t << " :" << endl; 

                double augment = flow;
                flow = fordfulkerson(_graphe->GFord, _graphe->ordreTopologique[s] + _size, _graphe->ordreTopologique[t], _version);
                cout << "Flot : " << flow << endl;
                if (flow > 1.5){
                    fin = true;

                    if (_version==2){
                        if (flow > augment + 0.5){
                            fin = false;
                        } else {
                            continue;
                        }
                    }

                    for (int i=0; i<_size; i++){
                        for (auto &pair : _graphe->GFord[i+_size]){
                            if (pair.second.flow > 0.5){
                                somme += _x[i][pair.first];
                            }
                        }
                    }

                    addLazy(somme>=flow-1);
                    
                } else {
                    reset_graph(_graphe->GFord);
                    flow = 0.;
                    t++;
                }

            }
            s++;
        }
        }
        
    } catch (GRBException e) {
        cout << "Error number: " << e.getErrorCode() << endl;
        cout << e.getMessage() << endl;
    } catch (...) { cout << "Error during callback" << endl; }
}
