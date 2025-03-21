#include "TER_ski.hpp"


TER_ski::TER_ski(string filename)
    {
        ifstream inf(filename);
        if (!inf)
        {
            size = -1;
            return;
        }

        string temp;
        int numNodes = -1;
        while (inf >> temp)
        {
            if (temp == "NODES")
            {
                inf >> numNodes;
                Graphe.resize(numNodes);
            }
            else if (temp == "LINKS")
            {
                int node1, node2;
                double capacity = capacity;
                while (inf >> node1 >> node2)
                {
                    if (Graphe[node1].find(node2) != Graphe[node1].end())
                    {
                        doublons.emplace_back(node1, node2);
                    }
                    else
                    {
                        Arc arc;
                        arc.capacity = capacity;
                        arc.flow = 0.0;
                        arc.residual = capacity;
                        Graphe[node1][node2] = arc;
                    }
                }
            }
        }
        size = numNodes;
        inf.close();
    }



void TER_ski::restreindre_graphe(const vector<vector<double>> &Xij)        // Modifie GFord pour avoir son graphe restreint aux xij = 0; avec les sommets dédoublés
    {
        int size = Graphe.size();
        GFord.resize(0);
        GFord.resize(2*size);
        double capacity = 1.;
        
        for (int i=0; i<size; i++){
            Arc a = {capacity, 0., capacity};
            GFord[i][i+size] = a;
            for (auto &[s, arc] : Graphe[i]){
                if (Xij[i][s] < 0.5){
                    GFord[size + i][s] = arc;
                }
            }
        }
        symmetrize(GFord);
    }

vector<int> TER_ski::triTopologique() {
    vector<int> degreEntrant(size, 0); // nb d'arc entrant pour chaque noeud
    queue<int> file; // FIFO
    vector<int> ordreTopologique;

    // calcul du degre entrant pour chaque noeud
    for (int u = 0; u < size; ++u) {
        for (const auto& [v, arc] : graph[u]) {
            degreEntrant[v]++;
        }
    }

    // si degreEntrant[u] == 0, alors u est ajouté à la file
    for (int i = 0; i < size; ++i) {
        if (degreEntrant[i] == 0) {
            file.push(i);
        }
    }
    /*
        tant que la file n'est pas vide
        on retire un noeud u de la file
        on ajoute u à l'ordre topologique
        on retire u de la liste des successeurs de chaque noeud v
        si degreEntrant[v] == 0, alors on ajoute v à la file
        si l'ordre topologique ne contient pas tous les noeuds, alors il y a un cycle
    */
    while (!file.empty()) {
        int u = file.front();
        file.pop();
        ordreTopologique.push_back(u);

        for (const auto& [v, arc] : graph[u]) {
            if (--degreEntrant[v] == 0) {
                file.push(v);
            }
        }
    }
    if (ordreTopologique.size() != size) {
        cerr << "Graphe contient un cycle." << endl;
        return {};
    }

    return ordreTopologique;
}



int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        cerr << "Specifier un fichier en argument." << endl;
        return 1;
    }

    string filename = argv[1];
    filename = "../data/" + filename;

    TER_ski ter(filename);

    if (ter.size == -1)
    {
        cerr << "Erreur : impossible d'ouvrir le fichier " << filename << endl;
        return 1;
    }

    vector<int> topoOrder = ter.triTopologique();

    if (topoOrder.empty())
    {
        return 1;
    }

    cout << "Tri topologique : ";
    for (int i = 0; i < ter.size; ++i)
    {
        cout << topoOrder[i] << " ";
    }
    cout << endl;

    return 0;
}
