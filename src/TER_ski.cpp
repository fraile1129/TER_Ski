#include "TER_ski.hpp"
#include "Ford_Fulkerson.hpp"
#include "Callback.hpp"

// typedef vector<unordered_map<int, Arc>> graph;

TER_ski::TER_ski(string filename)
{
  Graphe.resize(0);
  ifstream inf(filename);
  if (!inf)
  {
    cerr << "Erreur : Impossible d'ouvrir le fichier " << filename << endl;
    size = -1;
    return;
  }
  /*
  int numNodes, numEdges;
  inf >> numNodes >> numEdges;
  Graphe.resize(numNodes);
  int node1, node2;
  while(inf >> node1 >> node2){
    if(Graphe[node1].find(node2) != Graphe[node1].end()){
      doublons.emplace_back(node1, node2);
    }else{
      Arc arc;
      arc.capacity = 1.0;
      arc.flow = 0.0;
      arc.residual = arc.capacity;
      arc.cost = 0.0;
      arc.costPi = 0.0;
      Graphe[node1][node2] = arc;
    }
  }
  */
  
  
  string temp = "a";
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
      double capacity;
      while (inf >> node1 >> node2 >> capacity)
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
          arc.cost = 0.0;
          arc.costPi = 0.0;
          Graphe[node1][node2] = arc;
        }
      }
    }
  }
    
  
  size = numNodes;
  inf.close();
  triTopologique();
  restreindre_graphe_CM();
}


void TER_ski::restreindre_graphe_FF(const vector<vector<double>> &Xij) // Modifie GFord pour avoir son graphe restreint aux xij = 0; avec les sommets dédoublés
{

  int size = Graphe.size();
  GFord.resize(0);
  GFord.resize(2 * size);
  double capacity = 1.;

  for (int i = 0; i < size; i++)
  {
    Arc a = {capacity, 0., capacity, 0., 0.};
    GFord[i][i + size] = a;
    for (auto &pair : Graphe[i])
    {
      if (Xij[i][pair.first] < 0.5)
      {
        GFord[size + i][pair.first] = pair.second;
      }
    }
  }
  symmetrize(GFord);
}

void TER_ski::restreindre_graphe_CM()
{
  int size = Graphe.size();
  GPCC.supply.resize(0);
  GPCC.supply.resize(2 * size, 0.);
  GPCC.Graphe.resize(0);
  GPCC.Graphe.resize(2 * size);
  double capacity = 1.;

  for (int i = 0; i < size; i++)
  {
    Arc a = {capacity, 0., capacity, 0., 0.};
    GPCC.Graphe[i][i + size] = a;
    for (auto &[voisin, arc] : Graphe[i])
    {
      GPCC.Graphe[size + i][voisin] = arc;
    }
  }

  GPCC.symmetrize();
}

void TER_ski::triTopologique()
{
  vector<int> degreEntrant(size, 0); // nb d'arc entrant pour chaque noeud
  queue<int> file;                   // FIFO
  // vector<int> ordreTopologique;

  // calcul du degre entrant pour chaque noeud
  for (int u = 0; u < size; ++u)
  {
    for (const auto &pair : Graphe[u])
    {
      degreEntrant[pair.first]++;
    }
  }

  // si degreEntrant[u] == 0, alors u est ajouté à la file
  for (int i = 0; i < size; ++i)
  {
    if (degreEntrant[i] == 0)
    {
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
  while (!file.empty())
  {
    int u = file.front();
    file.pop();
    ordreTopologique.push_back(u);

    for (const auto &pair : Graphe[u])
    {
      if (--degreEntrant[pair.first] == 0)
      {
        file.push(pair.first);
      }
    }
  }
  if (ordreTopologique.size() != size)
  {
    cerr << "Graphe contient un cycle." << endl;
    abort();
  }
}

vector<pair<int, int>> TER_ski::Resolution(int version)
{
  vector<pair<int, int>> res;

  vector<vector<GRBVar>> x(size);
  // try{
  //  --- Creation of the Gurobi environment ---
  cout << "--> Creating the Gurobi environment" << endl;
  GRBEnv env = GRBEnv(true);
  // env.set("LogFile", "mip1.log"); ///< prints the log in a file
  env.start();

  // --- Creation of the Gurobi model ---
  cout << "--> Creating the Gurobi model" << endl;
  GRBModel model = GRBModel(env);
  model.set(GRB_IntParam_LazyConstraints, 1); // MANDATORY FOR LAZY CONSTRAINTS!

  // --- Creation of the variables ---
  cout << "--> Creating the variables" << endl;

  for (size_t i = 0; i < size; ++i)
  {
    x[i].resize(size);
    for (size_t j = 0; j < size; ++j)
    {
      if (Graphe[i].contains(j))
      {
        stringstream ss;
        ss << "x(" << i << "," << j << ")" << endl;
        x[i][j] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, ss.str());
      }
    }
  }

  // --- Creation of the objective function ---
  cout << "--> Creating the objective function" << endl;
  GRBLinExpr obj = 0;
  for (size_t i = 0; i < size; ++i)
  {
    for (size_t j = 0; j < size; ++j)
    {
      if (Graphe[i].contains(j))
      {
        obj += x[i][j];
      }
    }
  }
  model.setObjective(obj, GRB_MINIMIZE);

  SeparationCallback *myCallback = new SeparationCallback(x, size, this, version);
  model.setCallback(myCallback); // adding the callback to the model
  // Optimize model
  // --- Solver configuration ---
  cout << "--> Configuring the solver" << endl;
  model.set(GRB_DoubleParam_TimeLimit, 600.0); //< sets the time limit (in seconds)
  model.set(GRB_IntParam_Threads, 1);          //< limits the solver to single thread usage

  // --- Solver launch ---
  cout << "--> Running the solver" << endl;
  model.optimize();
  model.write("model.lp"); //< Writes the model in a file

  // --- Solver results retrieval ---
  cout << "--> Retrieving solver results " << endl;

  int status = model.get(GRB_IntAttr_Status);
  if (status == GRB_OPTIMAL || (status == GRB_TIME_LIMIT && model.get(GRB_IntAttr_SolCount) > 0))
  {
    // the solver has computed the optimal solution or a feasible solution (when the time limit is reached before proving optimality)
    cout << "Succes! (Status: " << status << ")" << endl; //< prints the solver status (see the gurobi documentation)
    cout << "Runtime : " << model.get(GRB_DoubleAttr_Runtime) << " seconds" << endl;

    cout << "--> Printing results " << endl;
    // model.write("solution.sol"); //< Writes the solution in a file
    cout << "Objective value = " << model.get(GRB_DoubleAttr_ObjVal) + doublons.size() << endl; //<gets the value of the objective function for the best computed solution (optimal if no time limit)
    for (size_t i = 0; i < size; ++i)
    {
      for (size_t j = 0; j < size; ++j)
      {
        if (Graphe[i].contains(j))
        {
          if (x[i][j].get(GRB_DoubleAttr_X) >= 1e-4)
          {
            cout << "On pose un capteur sur l'arc (" << i << ", " << j << ")" << endl;
            res.emplace_back(i, j);
          }
        }
      }
    }

    for (const auto &[i, j] : doublons)
    {
      cout << "On pose un capteur sur l'arc (" << i << ", " << j << "), doublon" << endl;
    }
  }

  else
  {
    // the model is infeasible (maybe wrong) or the solver has reached the time limit without finding a feasible solution
    cerr << "Fail! (Status: " << status << ")" << endl; //< see status page in the Gurobi documentation
  }
  delete myCallback;
  /*} catch(GRBException e) {
    cout << "Error code = " << e.getErrorCode() << endl;
    cout << e.getMessage() << endl;
  } catch(...) {
    cout << "Exception during optimization" << endl;
  }*/
  return res;
}

void TER_ski::Detection_Flot(){

  vector<vector<double>> xij(size, vector<double> (size,0));

  restreindre_graphe_FF(xij);

  for (int i=0; i<size-1; i++){
    for (int j=i+1; j<size; j++){
      int s = ordreTopologique[i] + size;
      int t = ordreTopologique[j];
      double flot = fordfulkerson(GFord, s, t, 1);
      if (flot > 1.5){
        F.emplace_back(s-size,t);
      }
      reset_graph(GFord);
    }
  }
}

vector<pair<int, int>> TER_ski::Resolution_compact()
{
  Detection_Flot();  // Déterminer l'ensemble F

  //vector<vector<GRBVar>> alpha(size, vector<GRBVar> (size));
  //vector<vector<GRBVar>> beta(size, vector<GRBVar> (size));
  vector<vector<vector<GRBVar>>> pi(size, vector<vector<GRBVar>> (size, vector<GRBVar> (size)));
  vector<vector<vector<vector<GRBVar>>>> gamma(size, vector<vector<vector<GRBVar>>> (size, vector<vector<GRBVar>> (size, vector<GRBVar> (size))));
  vector<vector<GRBVar>> x(size, vector<GRBVar> (size));

  vector<pair<int, int>> res; // vecteur à retourner

  //  --- Creation of the Gurobi environment ---
  cout << "--> Creating the Gurobi environment" << endl;
  GRBEnv env = GRBEnv(true);
  // env.set("LogFile", "mip1.log"); ///< prints the log in a file
  env.start();

  // --- Creation of the Gurobi model ---
  cout << "--> Creating the Gurobi model" << endl;
  GRBModel model = GRBModel(env);

  // --- Creation of the variables ---
  cout << "--> Creating the variables" << endl;
  int s, t, i, j;

  // x
  for (size_t i = 0; i < size; ++i)
  {
    //x[i].resize(size);
    for (size_t j = 0; j < size; ++j)
    {
      if (Graphe[i].contains(j))
      {
        stringstream ss;
        ss << "x(" << i << "," << j << ")" << endl;
        x[i][j] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, ss.str());
      }
    }
  }

  // --- Variables alpha, beta, pi, gamma ---
  for (size_t f = 0; f < F.size(); ++f)
  {
    int s = F[f].first;
    int t = F[f].second;
    
    // stringstream sa, sb;
    // sa << "alpha(" << s << "," << t << ")";
    // sb << "beta(" << s << "," << t << ")";
    // alpha[s][t] = model.addVar(-GRB_INFINITY, GRB_INFINITY, 0.0, GRB_CONTINUOUS, sa.str());
    // beta[s][t] = model.addVar(-GRB_INFINITY, GRB_INFINITY, 0.0, GRB_CONTINUOUS, sb.str());
    
    for (int i = 0; i < size; ++i)
    {
      stringstream spi;
      spi << "pi(" << s << "," << t << "," << i << ")";
      pi[s][t][i] = model.addVar(-GRB_INFINITY, GRB_INFINITY, 0.0, GRB_CONTINUOUS, spi.str());

      for (auto &[j,arc] : Graphe[i])
      {
        if (i != t && j != s)
        {
          stringstream sg;
          sg << "gamma(" << s << "," << t << "," << i << "," << j << ")";
          gamma[s][t][i][j] = model.addVar(0.0, GRB_INFINITY, 0.0, GRB_CONTINUOUS, sg.str());
        }
      }
    }
  }

  // --- Creation of the objective function ---
  cout << "--> Creating the objective function" << endl;
  GRBLinExpr obj = 0;
  for (size_t i = 0; i < size; ++i)
  {
    for (size_t j = 0; j < size; ++j)
    {
      if (Graphe[i].contains(j))
      {
        obj += x[i][j];
      }
    }
  }
  model.setObjective(obj, GRB_MINIMIZE);

  // --- Contraintes ---

  for (auto &[s,t] : F)
  {
    // s = F[f].first;
    // t = F[f].second;
    GRBLinExpr contrainte = 2 * pi[s][t][s] - 2 * pi[s][t][t];
    for (int i = 0; i < size; ++i)
    {
      for (auto &[j,arc] : Graphe[i])
      {
        if (i != t && j != s)
        {
          //int idx = i * size + j;
          contrainte -= gamma[s][t][i][j];
          model.addConstr(pi[s][t][i] - pi[s][t][j] - gamma[s][t][i][j] - x[i][j] <= 0);
        }
      }
    }
    model.addConstr(contrainte >= 1);
  }

  // Optimize model
  // --- Solver configuration ---
  cout << "--> Configuring the solver" << endl;
  model.set(GRB_DoubleParam_TimeLimit, 600.0); //< sets the time limit (in seconds)
  model.set(GRB_IntParam_Threads, 1);          //< limits the solver to single thread usage

  // --- Solver launch ---
  cout << "--> Running the solver" << endl;
  model.optimize();
  // model.write("model.lp"); //< Writes the model in a file

  // --- Solver results retrieval ---
  cout << "--> Retrieving solver results " << endl;

  int status = model.get(GRB_IntAttr_Status);
  if (status == GRB_OPTIMAL || (status == GRB_TIME_LIMIT && model.get(GRB_IntAttr_SolCount) > 0))
  {
    // the solver has computed the optimal solution or a feasible solution (when the time limit is reached before proving optimality)
    cout << "Succes! (Status: " << status << ")" << endl; //< prints the solver status (see the gurobi documentation)
    cout << "Runtime : " << model.get(GRB_DoubleAttr_Runtime) << " seconds" << endl;

    cout << "--> Printing results " << endl;
    // model.write("solution.sol"); //< Writes the solution in a file
    cout << "Objective value = " << model.get(GRB_DoubleAttr_ObjVal) << endl; //<gets the value of the objective function for the best computed solution (optimal if no time limit)
    for (size_t i = 0; i < size; ++i)
    {
      for (size_t j = 0; j < size; ++j)
      {
        if (Graphe[i].contains(j))
        {
          if (x[i][j].get(GRB_DoubleAttr_X) >= 1e-4)
          {
            cout << "On pose un capteur sur l'arc (" << i << ", " << j << ")" << endl;
            res.emplace_back(i, j);
          }
        }
      }
    }

    for (const auto &[i, j] : doublons)
    {
      cout << "On pose un capteur sur l'arc (" << i << ", " << j << "), doublon" << endl;
    }
  }

  else
  {
    // the model is infeasible (maybe wrong) or the solver has reached the time limit without finding a feasible solution
    cerr << "Fail! (Status: " << status << ")" << endl; //< see status page in the Gurobi documentation
  }
  return res;
}


bool TER_ski::checker(vector<pair<int, int>> capteurs)
{
  // Supprimer les arcs avec capteurs
  for (auto &[i, j] : capteurs)
  {
    Graphe[i].erase(j);
  }

  // Création des listes sources et puits
  vector<int> source;
  vector<int> sink;
  for (int i = 0; i < size; ++i)
  {
    if (Graphe[i].empty())
    {
      sink.push_back(i);
    }
    else
    {
      source.push_back(i);
    }
  }

  // Fonction récursive pour trouver tous les chemins jusqu'à 2
  function<void(int, int, unordered_set<int> &, vector<int> &, vector<vector<int>> &)> find_paths;
  find_paths = [&](int current, int target, unordered_set<int> &visited, vector<int> &path, vector<vector<int>> &foundPaths)
  {
    if (foundPaths.size() >= 2) return;

    visited.insert(current);
    path.push_back(current);

    if (current == target)
    {
      foundPaths.push_back(path);
    }
    else
    {
      for (auto &[neighbor, arc] : Graphe[current])
      {
        if (visited.find(neighbor) == visited.end())
        {
          find_paths(neighbor, target, visited, path, foundPaths);
        }
      }
    }

    path.pop_back();
    visited.erase(current);
  };

  // Recherche des chemins
  for (int s : source)
  {
    for (int t : sink)
    {
      if (s == t)
        continue;

      unordered_set<int> visited;
      vector<int> path;
      vector<vector<int>> foundPaths;

      find_paths(s, t, visited, path, foundPaths);

      if (foundPaths.size() > 1)
      {
        cout << "Plusieurs chemins entre " << s << " et " << t << " :" << endl;
        for (const auto &p : foundPaths)
        {
          cout << "Chemin : ";
          for (size_t i = 0; i < p.size(); ++i)
          {
            cout << p[i];
            if (i < p.size() - 1) cout << " -> ";
          }
          cout << endl;
        }
        return false;
      }
    }
  }

  return true;
}


vector<pair<int, int>> TER_ski::find_solution_realisable()
{
    graph G = Graphe;

    // arcs à retirer (capteurs)
    vector<pair<int, int>> capteurs;

    // Détection des sources et puits
    vector<int> sources, sinks;
    for (int i = 0; i < size; ++i)
    {
        if (G[i].empty())
        {
            sinks.push_back(i);
        }
        else
        {
            sources.push_back(i);
        }
    }

    // Fonction récursive pour compter les chemins entre s et t (max 2)
    function<int(int, int, unordered_set<int> &, vector<pair<int, int>> &)> count_paths;
    count_paths = [&](int current, int target, unordered_set<int> &visited, vector<pair<int, int>> &path) -> int
    {
        if (current == target)
            return 1;

        if (visited.count(current))
            return 0;

        visited.insert(current);
        int count = 0;

        for (const auto &neighbor : G[current])
        {
            if (visited.count(neighbor.first)) continue;

            path.emplace_back(current, neighbor.first);
            count += count_paths(neighbor.first, target, visited, path);
            if (count >= 2)
            {
                visited.erase(current);
                return count; // arrêt anticipé
            }
            path.pop_back(); // backtrack
        }

        visited.erase(current);
        return count;
    };

    // Boucle principale pour casser les multipaths
    bool changed = true;
    while (changed)
    {
        changed = false;

        for (int s : sources)
        {
            for (int t : sinks)
            {
                if (s == t)
                    continue;

                unordered_set<int> visited;
                vector<pair<int, int>> path;
                int paths = count_paths(s, t, visited, path);
                cout << "Nombre de chemins de " << s << " à " << t << " : " << paths << endl;

                if (paths >= 2 && !path.empty())
                {
                    // Couper un arc du milieu du chemin
                    pair<int, int> arc_to_cut = path[path.size() / 2];
                    if (G[arc_to_cut.first].count(arc_to_cut.second))
                    {
                        cout << "Suppression de l'arc : " << arc_to_cut.first << " -> " << arc_to_cut.second << endl;
                        G[arc_to_cut.first].erase(arc_to_cut.second);
                        capteurs.push_back(arc_to_cut);
                        changed = true;
                        break;
                    }
                }
            }
            if (changed)
                break;
        }
    }

    return capteurs;
}
