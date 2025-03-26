#pragma once
#include "TER_ski.hpp"

class SeparationCallback: public GRBCallback {
    public:
      vector<vector<GRBVar>> & _x;
        int _size; // Nombre de sommets
        TER_ski* _graphe;
        int _version;
        SeparationCallback(vector<vector<GRBVar>> & x, int size,TER_ski *graphe, int version);
    protected:
      void callback ();
  };
