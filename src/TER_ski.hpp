#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <iostream>
#include <utility>

using namespace std;

struct Arc
{
    double capacity;
    double flow;
    double residual;
};

class TER_ski
{
public:
    int size;
    vector<unordered_map<int, Arc>> graph;
    vector<pair<int, int>> doublons;

    TER_ski(string filename);

    vector<int> triTopo();

    double FordFulkerson();

    double Resolution();


    
};
