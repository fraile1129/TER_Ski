#include "PCC_successifs.hpp"



/*graph make_empty_graph(int n){      // Crée un graphe vide de taille n
    graph G;
    vector<double> supply(n,0);
    vector<unordered_map<int, set<Arc,compare>>> graphe (n);
    G.first = supply;
    G.second = graphe;
    return G;
}

bool is_valid_index(graph &G, int i){       // Vérifie que le sommet i appartient au graphe G
    return ((i >= 0) && (i < int(G.first.size())));
}

void add_Arc(graph &G, int begin, int end, double capacity, double cost){       // Ajoute un arc au graphe G
    
    if ((!is_valid_index(G, begin)) || (!is_valid_index(G, end))){     // On vérifie que les sommets de l'arc appartiennent au graphe
        cerr << "Invalid Index " << begin << " or " << end << " for a graph of size " << G.first.size() << endl;
        abort();
    }
        
    Arc a = {capacity, cost, cost, 0, capacity};
    G.second[begin][end].insert(a);

}*/


/*void symmetrize(graph &G){      // Symétrise le graphe
    
    for (int i=0; i<int(G.first.size()); i++){    // Pour chaque sommet
        for (auto &[voisin, arcs] : G.second[i]){     // On prend tous ses voisins
            for (auto &arc : arcs){     // On prend tous les arcs sortant de ce voisin
                if (arc.capa > 1e-6){       // Pour ne pas rajouter l'inverse des arcs inverse
                    add_Arc (G, voisin, i, 0, -1*arc.cost);
                }
            }
        }
    }
}*/


/*graph read_graph(string filename){      // Renvoie un graphe à partir d'un fichier
    
    ifstream file(filename);
    if(!file){
        cerr << "Could not open " << filename << endl;
        abort();
    }
    

    char indic;
    string lineComm;
    file >> indic;

    while (indic != 'p'){   // On cherche la ligne de définition du problème
        getline(file, lineComm);
        file >> indic;
    }

    // On récupère le nombre de noeuds et d'arcs
    file >> lineComm;
    int nodes, density;
    file >> nodes >> density;
    
    graph G = make_empty_graph (nodes);

    file >> indic;

    while (!file.eof()){

        switch (indic){

            // Ligne commentée
            case 'c' : {
                getline(file, lineComm);
                break;}

            // Ligne noeud
            case 'n' : {
                int ID;
                double supply;
                file >> ID >> supply;
                G.first[ID-1] = supply;
                break;}

            // Ligne arête
            case 'a' : {
                int from, to;
                double mincap, maxcap, cost;
                file >> from >> to >> mincap >> maxcap >> cost;
                add_Arc (G, from-1, to-1, maxcap, cost);
                break;}

            default : {
                cerr << "Erreur dans l'écriture / la lecture du fichier : Vérifier la première lettre de la ligne" << endl;
                abort();}

        }

        file >> indic;

    }
    symmetrize (G);
    return G;
}*/

void graphCM::print_graph(){     // Affiche le graphe G
    for (size_t i=0; i<supply.size(); i++){    // Pour chaque sommet
        cout << endl << "Voisins du sommet " << i << " (supply = " << supply[i] <<  ") :" << endl;
        for (auto &[voisin, arcs] : Graphe[i]){      // Pour chaque voisin
        
            cout << "   sommet " << voisin << " :" << endl;
            for (auto &arc : arcs){     // Pour chaque arc allant vers ce voisin
                cout << "       capacité : " << arc.capacity << ", résidu : " << arc.residual << ", coût : " << arc.cost  << ", coût réduit : " << arc.costPi << ", flot : " << arc.flow << endl;
            }
            cout << endl;
        }
    }
    cout << endl << endl << endl;
}

int graphCM::indCoutMin(unordered_set<int> &noeuds, vector<double> &distances){     // Renvoie l'indice du noeud temporaire dont la valeur dans le vecteur de distances est minimum

    double min = 1e10;
    int imin;     // !!! attention  
    for (int i: noeuds){
        if (distances[i] < min){
            imin = i;
            min = distances[i];
        }
    }

    return imin;
}

int graphCM::Dijsktra (int k, vector <double> &d, vector <int> &pred, Set &D){     // Calcule les PCC de k aux autres sommets selon leur coût réduit, renvoie un noeud en déficit accessible, -1 sinon


    unordered_set <int> Temp;
    for (int i=0; i<int(d.size()); i++){
        Temp.insert(i);
    }
    d[k] = 0;

    while (!Temp.empty()){

        int imin = indCoutMin(Temp, d);
        Temp.erase(imin);

        if (d[imin] > 1e8){    // On vérifie que le noeud soit accessible depuis k
            return -1;
        }

        if (D.contains(imin)){      // On arrête l'algo si on a un noeud en déficit avec une étiquette permanente
            for (int i : Temp){     // On met le degré des noeuds encore temporaires au degré du noeud en déficit trouvé
                d[i] = d[imin];
            }
            return imin;
        }

        for (auto &[voisin, arcs] : Graphe[imin]){    // Pour tous les voisins du sommet imin

            if (Temp.count(voisin)){    // Si ce voisin a une étiquette temporaire
                // On prend l'arc de coût réduit minimum et de capacité résiduelle non nulle allant vers ce sommet
                auto it = arcs.begin();
                if (it->residual > 1e-6){   // S'il y a bien un arc de capacité résiduelle non nulle
                    double Cij = it->costPi;
                    if (d[voisin] > d[imin] + Cij){
                        d[voisin] = d[imin] + Cij;
                        pred[voisin] = imin;
                    }
                }
            }
        }
    }

    cerr << "Erreur pour Dijsktra : soit l'algo a un problème, soit il n'y a plus de noeuds en déficit alors qu'il reste un noeud en excès : Σ(b(i)) ≠ 0?" << endl;
    abort();
}

void graphCM::augmenter_flot(int k, int l, double delta, vector<int> &pred){   // Augmente le flot x de delta unités le long du chemin indiqué par pred

    while (l!=k){   // On parcourt à l'envers le chemin de k à l
        int i = pred[l];

        // Arc normal
        Arc arcModif = *Graphe[i][l].begin();
        Graphe[i][l].erase(Graphe[i][l].begin());
        arcModif.flow += delta;
        arcModif.residual -= delta;
        Graphe[i][l].insert(arcModif);

        // Arc inverse
        double coutInv = -1*arcModif.cost;
        auto it = find_if(Graphe[l][i].begin(), Graphe[l][i].end(), [coutInv](const Arc& arc) { return (abs(arc.cost - coutInv) < 1e-6); });  // Pour obtenir l'iterateur correspondant à l'arc inverse
        Arc arcInv = *it;
        Graphe[l][i].erase(it);
        arcInv.residual += delta;
        Graphe[l][i].insert(arcInv);

        l = i;
    }

}

void graphCM::miseAJour(int k, int l, stack<int> &E, Set &D, vector<double> &exces, vector <double> &pi, double delta){    // Met à jour le graphe suite à l'augmentation de flot

    exces[k] -= delta;
    exces[l] += delta;

    if (abs(exces[k]) < 1e-6){
        E.pop();
    }
    if (abs(exces[l]) < 1e-6){
        D.erase(l);
    }

    // Mise à jour des coùts réduits
    for (size_t i=0; i<supply.size(); i++){     // Pour chaque sommet
        for (auto &[voisin, arcs] : Graphe[i]){      // Pour chaque voisin

            set<Arc,compare> arcsModif;
            for (auto &arc : arcs){     // Pour chaque arc allant vers ce voisin
                Arc arcModif = arc;
                arcModif.costPi = arc.cost - pi[i] + pi[voisin];
                arcsModif.insert(arcModif);
            }
            Graphe[i][voisin] = arcsModif;

        }
    }

    // Vérifier qu'il n'y a pas de flot dans les 2 sens
    for (size_t i=0; i<supply.size(); i++){     // Pour chaque sommet
        for (auto &[voisin, arcs] : Graphe[i]){      // Pour chaque voisin
            for (auto it = arcs.begin(); it != arcs.end(); ++it){       // Pour chaque arc allant vers ce voisin

                Arc arcNorm = *it;
                if (arcNorm.flow > 1e-6){   // Si le flot est non nul

                    double coutInv = -1*arcNorm.cost;
                    auto itInv = find_if(Graphe[voisin][i].begin(), Graphe[voisin][i].end(), [coutInv](const Arc& arc) { return (abs(arc.cost - coutInv) < 1e-6); });  // Pour obtenir l'iterateur correspondant à l'arc inverse
                    Arc arcInv = *itInv;

                    if (arcInv.flow > 1e-6){    // Si le flot inverse est aussi non nul

                        // On prend le min des 2 flots
                        double flotMin = arcNorm.flow;
                        if (arcNorm.flow > arcInv.flow){
                            flotMin = arcInv.flow;
                        }

                        // On modifie le flot de l'arc norm
                        arcNorm.flow -= flotMin;
                        arcs.erase(it);
                        arcs.insert(arcNorm);

                        // On modifie le flot de l'arc inverse
                        arcInv.flow -= flotMin;
                        Graphe[voisin][i].erase(itInv);
                        Graphe[voisin][i].insert(arcInv);

                    }
                }
            }
        }
    }
}

int graphCM::PCC_successifs (){      // Applique l'algorithme des plus courts chemins successifs sur le graphe

    // x=0, π = 0
    int size = Graphe.size();
    vector <double> potentiel (size, 0.);
    
    // e(i) = b(i) pour tout i ∈ N
    vector <double> exces = supply;
    
    // Saturer les arcs de coût négatif est inutile, les instances n'ont que des arcs de coût positif ou nul
    
    // E = {i ∈ N : e(i) > 0} et D = {i ∈ N : e(i) < 0}   
    stack <int> E;
    Set D;
    for (int i=0; i<size; i++){
        if (supply[i] > 1e-6){
            E.push(i);
        } else if (supply[i] < -1*1e-6){
            D.insert(i);
        }
    }

    while (!E.empty()){

        // Choisir un noeud k ∈ E
        int k = E.top();

        // Calculer les plus courts chemins du noeud k vers tous les autres sommets de G selon les coûts réduits
        vector <double> d(size, 1e9);
        vector <int> pred (size, -1);
        int l = Dijsktra (G, k, d, pred, D);

        if (l==-1){
            cout << "Il n'y a pas de chemin allant du sommet " << k << " vers un noeud en déficit : le problème est irréalisable" << endl;
            return -1;
        }

        // π = π - d
        for (int i=0; i<size; i++){
            potentiel[i] -= d[i];
        }

        // Calcul de δ
        double delta = 1.;
        //double delta = Delta(G, k, l, pred, exces);

        // Augmentation du flot
        augmenter_flot(G,k,l,delta,pred);

        // Tout mettre à jour
        miseAJour(G, k, l, E, D, exces, potentiel, delta);


    }

    return 1;
}