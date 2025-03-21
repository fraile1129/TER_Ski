#include<string>
#include<sstream>
#include<vector>
#include<iostream>
#include<tuple>
#include <algorithm>
#include "ATSP_Data.hpp"
#include "hi_pr.hpp"
#include "TER_ski.hpp" 

#include "gurobi_c++.h"

using namespace std;

class SeparationCallback: public GRBCallback {
  public:
    GRBVar** & _x;
    int _size; // Largeur de la matrice d'adjacence
    SeparationCallback(GRBVar** & x, int size) : _x(x), _size(size) { }
  protected:
    void callback () {
      try {
        if ( where == GRB_CB_MIPSOL ){
          vector<vector<int>> matAdj(_size,vector<int>(_size,0));
          for (int i = 0; i < _size; i++) {
            for (int j = 0; j < _size; j++) {
                double solVal = getSolution(_x[i][j]);
                if (solVal > 0.5) {  
                  matAdj[i][j] = 1;
                } else {
                  matAdj[i][j] = 0;
                }
            }
          } 
        }
        
      } catch (GRBException e) {
        cout << "Error number: " << e.getErrorCode() << endl;
        cout << e.getMessage() << endl;
      } catch (...) { cout << "Error during callback" << endl; }
    }
};
int main(int argc, char** argv){
  if (argc < 2) {
    cout << "usage : " << argv[0] << " ATSPFilename" << endl;
    return 0;
  }

  cout << "Size : " << size << endl;

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
  
      SeparationCallback * myCallback = new SeparationCallback(x,size);
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
    return 0;
  }
  