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
                    if (_graphe->Graphe[i].contains(j)){
                        xij[i][j] = getSolution(_x[i][j]);
                    }
                }
            }
            _graphe->restreindre_graphe_FF(xij);
                
            while (s<_size-1 && !fin){
                int t = s+1;
                double flow = 0.;
                while (t<_size && !fin){
                    //cout << "Calcul flot entre " << s << " et " << t << " :" << endl; 

                    double augment = flow;
                    flow = fordfulkerson(_graphe->GFord, _graphe->ordreTopologique[s] + _size, _graphe->ordreTopologique[t], _version);
                    //cout << "Flot : " << flow << endl;
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
                            for (auto &[voisin, arc] : _graphe->GFord[i+_size]){
                                if (arc.flow > 0.5){
                                    somme += _x[i][voisin];
                                    cout << "x[" << i << "][" << voisin << "] + ";
                                }
                            }
                        }

                        addLazy(somme>=flow-1);
                        cout << " >= " << flow-1 << endl;
                        
                    } else {
                        reset_graph(_graphe->GFord);
                        flow = 0.;
                        t++;
                    }

                }
                s++;
            }
        } else if ((where == GRB_CB_MIPNODE) && (getIntInfo(GRB_CB_MIPNODE_STATUS) == GRB_OPTIMAL)) {       // Solution RL => User Cuts
            // Récupérer la solution courante
            // xij deviennent les coûts (réduits?)

            // Doubler le graphe pour noeud disjoints => GraphPCC

            // Pour chaque s<t :
                // Mettre la demande = 2 pour s et t
                // Calcul FCM
                // Si flot real 
                    // si cout > demande
                        // On augmente la demande de 1 et on refait
                    // sinon 
                        // fin = true
                        // v2 : fin = true + continue si pas d'augment, sinon true = false
                        // ajout contrainte sur le flot? (somme des xij contenant le flot >= demande - 1)
                        // est-ce qu'on arrête là ou on continue sur ces sommets jusqu'à non réalis?
                        

                // Sinon
                    // t++
                    // reset graphePCC?

        }
        
    } catch (GRBException e) {
        cout << "Error number: " << e.getErrorCode() << endl;
        cout << e.getMessage() << endl;
    } catch (...) { cout << "Error during callback" << endl; }
}
