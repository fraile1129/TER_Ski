#include "TER_ski.hpp"
#include "Ford_Fulkerson.hpp"

typedef vector<unordered_map<int, Arc>> graph;

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
                double capacity = 1.0;
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
        triTopologique();
    }


class SeparationCallback: public GRBCallback {
        public:
            GRBVar** & _x;
            int _size; // Nombre de sommets
            TER_ski* _graphe;
            SeparationCallback(GRBVar** & x, int size,TER_ski *graphe) : _x(x), _size(size), _graphe(graphe) { }
        protected:
          void callback () {
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
                  
                while (s<_size-1 && !fin){
                    int t = s+1;
                    while (t<_size && !fin){
                        double flow = fordfulkerson(_graphe->GFord, ordreTopologique[s], ordreTopologique[t]);
                        if (flow > 1.5){
                            fin = true;

                            for (int i=0; i<_size; i++){
                                for (auto &[voisin, arc] : GFord[i+_size]){
                                    if (arc.flow > 0.5){
                                        somme += _x[i][voisin];
                                    }
                                }
                            }

                            addLazy(somme>=flow-1);
                            
                        } else {
                            reset_graph(_graphe->GFord);
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
      };


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


void TER_ski::Resolution(){
    GRBVar** x;
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
      x = new GRBVar*[size];
      for(size_t i=0;i<size;++i){
        x[i] = new GRBVar[size];
        for(size_t j=0;j<size;++j){
          stringstream ss;
          ss << "x(" << i<< ","<<j<<")" << endl;
          x[i][j]= model.addVar(0.0, 1.0, 0.0, GRB_BINARY, ss.str());
        }
      }
  
  
      // --- Creation of the objective function ---
      cout<<"--> Creating the objective function"<<endl;
      GRBLinExpr obj = 0;
      for(size_t i=0;i<size;++i){
        for(size_t j=0;j<size;++j){
          if(i!=j){
            obj+=x[i][j];
          }
        }
      }
      model.setObjective(obj, GRB_MINIMIZE);
  
      SeparationCallback * myCallback = new SeparationCallback(x,size,this);
      model.setCallback(myCallback); // adding the callback to the model
      // Optimize model
      // --- Solver configuration ---
      cout<<"--> Configuring the solver"<<endl;
      model.set(GRB_DoubleParam_TimeLimit, 600.0); //< sets the time limit (in seconds)
      model.set(GRB_IntParam_Threads,1); //< limits the solver to single thread usage
  
  
      // --- Solver launch ---
      cout<<"--> Running the solver"<<endl;
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
        cout << "Objective value = "<< model.get(GRB_DoubleAttr_ObjVal)  << endl; //<gets the value of the objective function for the best computed solution (optimal if no time limit)
        for(size_t i=0;i<size;++i){
          for(size_t j=0;j<size;++j){
            if(x[i][j].get(GRB_DoubleAttr_X)>=1e-4){
              cout << "On pose un capteur sur l'arc (" << i << ", " << j << ")" << endl;
            }
          }
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
    for(size_t j=0;j<size;++j){
      delete[] x[j];
    }
    delete[] x;
}  


void TER_ski::triTopologique() {
    vector<int> degreEntrant(size, 0); // nb d'arc entrant pour chaque noeud
    queue<int> file; // FIFO
    //vector<int> ordreTopologique;

    // calcul du degre entrant pour chaque noeud
    for (int u = 0; u < size; ++u) {
        for (const auto& [v, arc] : Graphe[u]) {
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

        for (const auto& [v, arc] : Graphe[u]) {
            if (--degreEntrant[v] == 0) {
                file.push(v);
            }
        }
    }
    if (ordreTopologique.size() != size) {
        cerr << "Graphe contient un cycle." << endl;
        return {};
    }

    //return ordreTopologique;
}
