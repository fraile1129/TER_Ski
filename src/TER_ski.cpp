#include "TER_ski.hpp"
#include "Ford_Fulkerson.hpp"
#include "Callback.hpp"

//typedef vector<unordered_map<int, Arc>> graph;

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

        string temp = "a";
        int numNodes = -1;
        cout << "test" << endl;
        while (inf >> temp)
        {
            if (temp == "NODES")
            {
                inf >> numNodes;
                cout << numNodes << endl;
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
                        Graphe[node1][node2] = arc;
                    }
                }
            }
        }
        size = numNodes;
        inf.close();
        triTopologique();
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
        cout << GFord[i][i+size].capacity << endl;
        for (auto &pair : Graphe[i]){
            if (Xij[i][pair.first] < 0.5){
              cout << "ajout arc " << i << " " << pair.first << endl;
                GFord[size + i][pair.first] = pair.second;
            }
        }
    }
    symmetrize(GFord);
  }

void TER_ski::triTopologique() 
  {
    vector<int> degreEntrant(size, 0); // nb d'arc entrant pour chaque noeud
    queue<int> file; // FIFO
    //vector<int> ordreTopologique;

    // calcul du degre entrant pour chaque noeud
    for (int u = 0; u < size; ++u) {
        for (const auto& pair : Graphe[u]) {
            degreEntrant[pair.first]++;
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

        for (const auto& pair : Graphe[u]) {
            if (--degreEntrant[pair.first] == 0) {
                file.push(pair.first);
            }
        }
    }
    if (ordreTopologique.size() != size) {
      cout << ordreTopologique.size();
        cerr << "Graphe contient un cycle." << endl;
        abort();
    }
  }


void TER_ski::Resolution(int version)
  {
    vector<vector<GRBVar>> x(size);
    try{
      // --- Creation of the Gurobi environment ---
      cout<<"--> Creating the Gurobi environment"<<endl;
      GRBEnv env = GRBEnv(true);
      //env.set("LogFile", "mip1.log"); ///< prints the log in a file
      env.start();
  
      // --- Creation of the Gurobi model ---
      cout<<"--> Creating the Gurobi model"<<endl;
      GRBModel model = GRBModel(env);
      model.set(GRB_IntParam_LazyConstraints , 1); // MANDATORY FOR LAZY CONSTRAINTS!


      // --- Creation of the variables ---
      cout<<"--> Creating the variables"<<endl;

      for(size_t i = 0; i < size; ++i){
        x[i].resize(size);
        for(size_t j = 0; j < size; ++j){
            stringstream ss;
            ss << "x(" << i << "," << j << ")" << endl;
            x[i][j] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, ss.str());
          
          
        }
      }
  
  
      // --- Creation of the objective function ---
      cout << "--> Creating the objective function" << endl;
      GRBLinExpr obj = 0;
      for(size_t i = 0; i < size; ++i){
        for(size_t j = 0; j < size; ++j){
          if(Graphe[i].count(j)){
            obj += x[i][j];
          }
        }
      }
      model.setObjective(obj, GRB_MINIMIZE);
  
      SeparationCallback * myCallback = new SeparationCallback(x,size,this,version);
      model.setCallback(myCallback); // adding the callback to the model
      // Optimize model
      // --- Solver configuration ---
      cout << "--> Configuring the solver" << endl;
      model.set(GRB_DoubleParam_TimeLimit, 600.0); //< sets the time limit (in seconds)
      model.set(GRB_IntParam_Threads, 1); //< limits the solver to single thread usage
  
  
      // --- Solver launch ---
      cout <<"--> Running the solver"<<endl;
      model.optimize();
      model.write("model.lp"); //< Writes the model in a file
  
  
      // --- Solver results retrieval ---
      cout<<"--> Retrieving solver results "<<endl;
  
      int status = model.get(GRB_IntAttr_Status);
      if (status == GRB_OPTIMAL || (status== GRB_TIME_LIMIT && model.get(GRB_IntAttr_SolCount)>0))
      {
        //the solver has computed the optimal solution or a feasible solution (when the time limit is reached before proving optimality)
        cout << "Succes! (Status: " << status << ")" << endl; //< prints the solver status (see the gurobi documentation)
        cout << "Runtime : " << model.get(GRB_DoubleAttr_Runtime) << " seconds"<<endl;
  
        cout<<"--> Printing results "<<endl;
        //model.write("solution.sol"); //< Writes the solution in a file
        cout << "Objective value = "<< model.get(GRB_DoubleAttr_ObjVal) + doublons.size()  << endl; //<gets the value of the objective function for the best computed solution (optimal if no time limit)
        for(size_t i=0;i<size;++i){
          for(size_t j=0;j<size;++j){
            if(x[i][j].get(GRB_DoubleAttr_X)>=1e-4){
              cout << "On pose un capteur sur l'arc (" << i << ", " << j << ")" << endl;
            }
          }
        }

        for (const auto &[i,j] : doublons){
          cout << "On pose un capteur sur l'arc (" << i << ", " << j << "), doublon" << endl;
        }
      }
  
      else
      {
        // the model is infeasible (maybe wrong) or the solver has reached the time limit without finding a feasible solution
        cerr << "Fail! (Status: " << status << ")" << endl; //< see status page in the Gurobi documentation
      }
      delete myCallback;
    } catch(GRBException e) {
      cout << "Error code = " << e.getErrorCode() << endl;
      cout << e.getMessage() << endl;
    } catch(...) {
      cout << "Exception during optimization" << endl;
    }
    
  }  



