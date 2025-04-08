#include "Callback.hpp"
#include "Ford_Fulkerson.hpp"

SeparationCallback::SeparationCallback(vector<vector<GRBVar>> & x, int size,TER_ski *graphe, int version) : _x(x), _size(size), _graphe(graphe), _version(version){}

void SeparationCallback::callback () {
    try {
        if ( where == GRB_CB_MIPSOL ){        // Solution entière => Lazy Cuts
            int s = 0;
            bool fin = false;

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
                        GRBLinExpr somme = 0;

                        if (_version%3==2){     // Versions 2, 5 ou 8
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
                                    //cout << "x[" << i << "][" << voisin << "] + ";
                                }
                            }
                        }

                        addLazy(somme>=flow-1);
                        //cout << " >= " << flow-1 << endl;
                        
                    } else {
                        reset_graph(_graphe->GFord);
                        flow = 0.;
                        t++;
                    }

                }
                s++;
            }
        } else if (_version > 3 && (where == GRB_CB_MIPNODE) && (getIntInfo(GRB_CB_MIPNODE_STATUS) == GRB_OPTIMAL)) {       // Solution RL => User Cuts

            int s = 0;
            int ajout = 0;
            bool fin = false;
            
            // On récupère la solution
            vector<vector<double>> xij(_size);
            for (int i=0 ; i<_size ; i++) {
                xij[i].resize(_size);
                for (int j=0 ; j<_size ; j++) {
                    if (_graphe->Graphe[i].contains(j)){
                        xij[i][j] = getNodeRel(_x[i][j]);
                    }
                }
            }

            // Affichage de la solution
            for (int i=0; i<_size; i++){
                for (int j=0; j<_size; j++){
                    cout << xij[i][j] << " ";
                }
                cout << endl;
            }
            
            _graphe->GPCC.ajouter_couts(xij);
            // Doubler le graphe pour noeud disjoints => GraphPCC

            double demande = 2;

            while (s<_size-1 && ajout < 20){
                int a = _graphe->ordreTopologique[s] + _size;
                int t = s+1;

                while (t<5 && ajout < 20){
                    int b = _graphe->ordreTopologique[t];

                    if (demande==2){
                        _graphe->GPCC.supply[a] = demande;
                        _graphe->GPCC.supply[b] = -1*demande;
                    }
                    
                    double cost = _graphe->GPCC.PCC_successifs(s, t, _graphe->ordreTopologique);
                    cout << "chemin de " << a-_size << " à " << b << ", " << "demande : " << demande << ", coût : " << cost << endl;
                    if (cost < 0){  // Si flot non réalisable, donc pas [demande] chemins noeuds-disjoint
                        
                        t++;
                        demande = 2;
                        _graphe->GPCC.reset_graph();
                        
                        
                    } else if (cost > demande - 1.0001){     // Si assez de capteurs
                        cout << "assez de capteurs" << endl;
                        demande ++;
                        _graphe->GPCC.supply[a] = 1;
                        _graphe->GPCC.supply[b] = 1;

                    } else {   // Si contrainte non respectée

                        if (_version > 6){
                            cout << "test 2" << endl;
                            demande ++;
                            _graphe->GPCC.supply[a] = 1;
                            _graphe->GPCC.supply[b] = 1;
                        }
                        
                        GRBLinExpr somme = 0;

                        cout << "Ajout User" << endl;
                        for (int i=0; i<_size; i++){
                            for (auto &[voisin, arc] : _graphe->GPCC.Graphe[i + _size]){
                                if (arc.flow > 0.5){
                                    somme += _x[i][voisin];
                                    cout << "x[" << i << "][" << voisin << "] + ";
                                }
                            }
                        }
                        cout << " >= " << demande - 1 << endl;

                        addCut(somme >= demande - 1);
                        ajout++;

                        if (_version < 7){
                            t++;
                            demande = 2;
                            _graphe->GPCC.reset_graph(a,b);
                        }
                        
                    }
                }
                s++;
            }
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
