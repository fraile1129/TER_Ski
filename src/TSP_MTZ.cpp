#include <string>
#include <vector>
#include <iostream>
#include "ATSP_Data.hpp"
#include "gurobi_c++.h"

using namespace std;

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    cout << "usage : " << argv[0] << " ATSPFilename" << endl;
    return 0;
  }

  string filename = "data/" + string(argv[1]);
  ATSPDataC data(filename);

  cout << "Size : " << data.size << endl;
  cout << "Distances : " << endl;
  for (size_t i = 0; i < data.size; ++i)
  {
    for (size_t j = 0; j < data.size; ++j)
    {
      cout << data.distances[i][j] << " ";
    }
    cout << endl;
  }

  const int nbVilles = data.size;

  // Distances entre chaque ville
  // data.distances[i][j]

  // --- Creation of the Gurobi environment ---
  cout << "--> Creating l'environnement Gurobi" << endl;
  GRBEnv env = GRBEnv(true);
  // env.set("LogFile", "mip1.log"); ///< prints the log in a file
  env.start();

  // --- Creation of the Gurobi model ---
  cout << "--> Creating le modèle Gurobi" << endl;
  GRBModel model = GRBModel(env);

  // --- Creation of the variables ---
  cout << "--> Creating les variables" << endl;

  vector<vector<GRBVar>> x(nbVilles, vector<GRBVar>(nbVilles)); // TODO
  vector<GRBVar> u(nbVilles);

  for (size_t i = 0; i < nbVilles; ++i)
  {
    for (size_t j = 0; j < nbVilles; ++j)
    {
      if (i != j)
      {
        // cout << "x(" << i << ", " << j << ")\n";
        x[i][j] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, "x");
      }
    }
    u[i] = model.addVar(0, nbVilles - 1, 0.0, GRB_INTEGER, "u");
  }

  // --- Creating the fonction objectif ---
  cout << "--> Creating la fonction objectif" << endl;

  GRBLinExpr obj = 0;
  for (size_t i = 0; i < nbVilles; ++i)
  {
    for (size_t j = 0; j < nbVilles; ++j)
    {
      if (i != j)
      {
        obj += data.distances[i][j] * x[i][j];
      }
    }
  }
  model.setObjective(obj, GRB_MINIMIZE);

  // --- Creating les contraintes ---
  cout << "--> Creating les contraintes" << endl;

  // Contrainte (1)
  for (size_t i = 0; i < nbVilles; ++i)
  {
    GRBLinExpr c1 = 0;
    for (size_t j = 0; j < nbVilles; ++j)
    {
      if (i != j)
      {
        // utilisation des opérateurs (+ ou -) pour ajouter les variables à la contrainte
        c1 += x[i][j];
      }
    }
    // cout << "Sum xij(" << i << ")";
    // Ajout de la contrainte au modèle (contrainte, nom)
    model.addConstr(c1 == 1, "c1");
  }

  // Contrainte (2)
  for (size_t i = 0; i < nbVilles; ++i)
  {
    GRBLinExpr c2 = 0;
    for (size_t j = 0; j < nbVilles; ++j)
    {
      if (i != j)
      {
        c2 += x[j][i];
      }
    }
    // cout << "Sum xji(" << i << ")";
    model.addConstr(c2 == 1, "c1");
  }

  // Contrainte (3)
  model.addConstr(u[0] == 0, "c3");

  // Contrainte (4)
  for (size_t i = 0; i < nbVilles; ++i)
  {
    for (size_t j = 0; j < nbVilles; ++j)
    {
      if (i != j && j != 0)
      {
        GRBLinExpr c4 = 0;
        c4 = u[i] + 1 - (nbVilles - 1) * (1 - x[i][j]) - u[j];
        model.addConstr(c4 <= 0, "c4");
      }
    }
  }

  // Contraintes (5) et (6)
  for (size_t i = 1; i < nbVilles; ++i)
  {
    GRBLinExpr c5, c6 = u[i];
    model.addConstr(c5 <= nbVilles - 1, "c5");
    model.addConstr(c6 >= 1, "c6");
  }

  // Configurer Solveur
  model.set(GRB_DoubleParam_TimeLimit, 600.0); //< définition du temps limite (en secondes)
  model.set(GRB_IntParam_Threads, 1);          //< définition du nombre de threads pouvant être utilisé
  model.write("model.lp");                     //< écriture du modèle PLNE dans le fichier donné en paramètre (optionnel)

  model.optimize();

}
