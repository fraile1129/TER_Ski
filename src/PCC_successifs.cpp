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

void graphCM::reset_graph()
    {
        for (size_t i=0; i<Graphe.size(); i++) {
            
            auto it = Graphe[i].begin();
            while (it != Graphe[i].end()){
                int voisin = it->first;
                Arc arc = it->second;
                Arc a = {arc.capacity, 0, arc.capacity, arc.cost, arc.cost};
                it = Graphe[i].erase(it);
                Graphe[i][voisin] = a;
            }
        }
    }

void graphCM::symmetrize(){      // Symétrise le graphe
    
    for (size_t i=0; i<supply.size(); i++){    // Pour chaque sommet
        for (auto &[voisin, arc] : Graphe[i]){     // On prend tous ses voisins
            
            if (arc.capacity > 1e-6){       // Pour ne pas rajouter l'inverse des arcs inverse
                Arc a = {0, 0, 0, -1*arc.cost, -1*arc.cost};
                Graphe[voisin][i] = a;
                //add_Arc (G, voisin, i, 0, -1*arc.cost);
            }
        
        }
    }
}


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

/*void graphCM::print_graph(){     // Affiche le graphe G
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
}*/

int graphCM::indCoutMin(unordered_set<int> &noeuds, vector<double> &distances){     // Renvoie l'indice du noeud temporaire dont la valeur dans le vecteur de distances est minimum

    double min = std::numeric_limits<double>::infinity();
    int imin = -1;
    for (int i: noeuds){
        if (distances[i] < min){
            imin = i;
            min = distances[i];
        }
    }

    return imin;
}

bool graphCM::Dijsktra (int s, int t, vector <double> &d, vector <int> &pred, const vector<int> &Topo){

    int size = d.size() ;
    int a = Topo[s] + size/2;
    int b = Topo[t];
    
    unordered_set <int> Temp;
    for (int i=s+1; i<t+1; i++){
        Temp.insert(Topo[i]);
    }

    d[a] = 0;
    
    for (auto &[voisin, arc] : Graphe[a]){
        if (arc.residual > 1e-6){   // Si l'arc a une capa résiduelle non nulle
            //cout << "voisin de a : " << voisin << endl;
            d[voisin] = arc.costPi;
            d[voisin + size/2] = arc.costPi;
            pred[voisin] = a;
            pred[voisin + size/2] = voisin;
        }
    }

    //cout << "chemin de " << Topo[s] << " à " << Topo[t] << endl;
    while (!Temp.empty()){
    
        /*cout << "distances :" << endl;
        for (int i=0; i<size; i++){
            cout << i << " : " << d[i] << endl;
        }
        cout << "Temp : " << endl;
        for (auto &som : Temp){
            cout << som << " ";
        }
        cout << endl;*/
        int imin = indCoutMin(Temp,d);
        //cout << "imin : " << imin << endl;
        if (imin == -1){
            return false;
        }
        Temp.erase(imin);

        if (d[imin] > 1e8){    // On vérifie que le noeud soit accessible depuis k
            return false;
        }

        if (imin == b){              // On arrête l'algo si b n'est plus temporaire
            for (int i : Temp){     // On met le degré des noeuds encore temporaires au degré du noeud en déficit trouvé
                d[i] = d[imin];
            }
            return true;
        }

        for (auto &[voisin, arc] : Graphe[imin + size/2]){    // Pour tous les voisins du sommet imin

            if (Temp.count(voisin)){    // Si ce voisin a une étiquette temporaire
                // On prend l'arc de coût réduit minimum et de capacité résiduelle non nulle allant vers ce sommet
                //auto it = arcs.begin();
                if (arc.residual > 1e-6){   // S'il y a bien un arc de capacité résiduelle non nulle
                    double Cij = arc.costPi;
                    if (d[voisin] > d[imin] + Cij){
                        d[voisin] = d[imin] + Cij;
                        d[voisin + size/2] = d[voisin];
                        pred[voisin] = imin + size/2;
                        pred[voisin + size/2] = voisin;
                    }
                }
            }
        }

    }

    cerr << "Erreur pour Dijsktra : soit l'algo a un problème, soit il n'y a plus de noeuds en déficit alors qu'il reste un noeud en excès : Σ(b(i)) ≠ 0?" << endl;
    abort();
}

int graphCM::Dijsktra (int k, vector <double> &d, vector <int> &pred, Set &D){     // Calcule les PCC de k aux autres sommets selon leur coût réduit, renvoie un noeud en déficit accessible, -1 sinon


    unordered_set <int> Temp;
    for (int i=0; i<int(d.size()); i++){
        Temp.insert(i);
    }
    d[k] = 0;

    while (!Temp.empty()){

        int imin = indCoutMin(Temp, d);
        if (imin == -1){
            cerr << "pas normal ça" << endl;
            abort();
        }
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

        for (auto &[voisin, arc] : Graphe[imin]){    // Pour tous les voisins du sommet imin

            if (Temp.count(voisin)){    // Si ce voisin a une étiquette temporaire
                // On prend l'arc de coût réduit minimum et de capacité résiduelle non nulle allant vers ce sommet
                //auto it = arcs.begin();
                if (arc.residual > 1e-6){   // S'il y a bien un arc de capacité résiduelle non nulle
                    double Cij = arc.costPi;
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

        double flotInv = Graphe[l][i].flow;   

        // Arc normal
        Arc arcModif = Graphe[i][l];
        Graphe[i].erase(l);
        if (flotInv < 1e-6){        // Si l'arc inverse a un flot nul
            arcModif.flow += delta;
        }
        arcModif.residual -= delta;
        Graphe[i][l] = arcModif;

        // Arc inverse
        arcModif = Graphe[l][i];
        Graphe[l].erase(i);
        if (flotInv > 1e-6){        // Si l'arc inverse a un flot non nul
            arcModif.flow -= delta;
        }
        arcModif.residual += delta;
        Graphe[l][i] = arcModif;

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

    // Mise à jour des coûts réduits
    for (size_t i=0; i<supply.size(); i++){     // Pour chaque sommet
        for (auto &[voisin, arc] : Graphe[i]){      // Pour chaque voisin

            Arc arcModif = arc;
            arcModif.costPi = arc.cost - pi[i] + pi[voisin];
            Graphe[i][voisin] = arcModif;

        }
    }
}
void graphCM::miseAJour(int s, int t, vector<double> &exces, vector <double> &pi, double delta, const vector<int> &Topo){    // Met à jour le graphe suite à l'augmentation de flot

    int demi_size = Topo.size();
    exces[Topo[s] + demi_size] -= delta;
    exces[Topo[t]] += delta;

    // Mise à jour des coûts réduits
    /*for (size_t i=s; i<t+1; i++){     // Pour chaque sommet
        for (auto &[voisin, arc] : Graphe[Topo[i]+demi_size]){      // Pour chaque voisin

            Arc arcModif = arc;
            arcModif.costPi = arc.cost - pi[Topo[i]] + pi[voisin];
            Graphe[Topo[i]+demi_size][voisin] = arcModif;

        }
    }*/
}


double graphCM::PCC_successifs (){      // Applique l'algorithme des plus courts chemins successifs sur le graphe

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
        int l = Dijsktra (k, d, pred, D);

        if (l==-1){
            //cout << "Il n'y a pas de chemin allant du sommet " << k << " vers un noeud en déficit : le problème est irréalisable" << endl;
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
        augmenter_flot(k,l,delta,pred);

        // Tout mettre à jour
        miseAJour(k, l, E, D, exces, potentiel, delta);


    }

    double coutTotal = 0;

    for (int i=0; i<size; i++){     // Pour chaque sommet
        for (auto &[voisin, arc] : Graphe[i]){      // Pour chaque voisin

            if (arc.flow > arc.capacity){       // Vérification contrainte de capacité de l'arc
                cout << "Erreur : l'arc " << i << "->" << voisin << " a un flot de " << arc.flow << " pour une capacité de " << arc.capacity << ": le flot n'est pas réalisable" << endl;
                return -1;
            }


            // Pour vérifier les contraintes de supply
            if (arc.flow > 1e-6){   // Si le flot est non nul
                coutTotal += arc.flow*arc.cost;
            }

        }
    }

    return coutTotal;
}





double graphCM::PCC_successifs (int s, int t, const vector<int> &Topo){      // Applique l'algorithme des plus courts chemins successifs sur le graphe

    int size = Graphe.size();
    int a = Topo[s] + size/2;
    int b = Topo[t];

    vector <double> potentiel (size/2, 0.);
    vector <double> exces = supply;

    while (exces[a] > 1e-8){

        vector <double> d(size, std::numeric_limits<double>::infinity());
        vector <int> pred (size, -1);

        if (Dijsktra (s, t, d, pred, Topo)){

            // π = π - d
            for (int i=s; i<t+1; i++){
                potentiel[Topo[i]] -= d[Topo[i]];
            }

            double delta = 1.;

            // Augmentation du flot
            augmenter_flot(a,b,delta,pred);

            // Tout mettre à jour
            miseAJour(s, t, exces, potentiel, delta, Topo);

            //cout << "exces de " << a-size/2 << " : " << exces[a] << endl;

        } else {
            //cout << "Il n'y a pas de chemin allant du sommet " << k << " vers un noeud en déficit : le problème est irréalisable" << endl;
            return -1;
        }

        

    }

    double coutTotal = 0;
    //cout << "flot réalisable" << endl;

    for (int i=0; i<size; i++){     // Pour chaque sommet
        for (auto &[voisin, arc] : Graphe[i]){      // Pour chaque voisin

            /*if (arc.flow > arc.capacity){       // Vérification contrainte de capacité de l'arc
                cout << "Erreur : l'arc " << i << "->" << voisin << " a un flot de " << arc.flow << " pour une capacité de " << arc.capacity << ": le flot n'est pas réalisable" << endl;
                return -1;
            }*/


            if (arc.flow > 1e-6){   // Si le flot est non nul
                coutTotal += arc.flow*arc.cost;
            }

        }
    }

    return coutTotal;

}
