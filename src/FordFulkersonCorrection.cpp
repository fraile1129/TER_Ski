#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <cmath>

using namespace std;

struct Arc {
  double capacity;
  double flow;
  double residual;
};

typedef vector<unordered_map<int, Arc>> graph;

// Set to true for debugging messages
bool verbose = true;

graph make_empty_graph(int n) {
  graph result(n);
  return result;
}

bool is_valid_index(graph& g, int i){
  int n = g.size();
  return((i >= 0) && (i < n));
}

bool add_arc(graph& g, int a, int b, double capacity) {
  if ((!is_valid_index(g, a)) || (!is_valid_index(g, b))) {
    cout << "Invalid Index " << a << " or " << b << " for a graph of size " << g.size()<< endl;
    throw(1);
  }
  Arc a_to_b = {capacity, 0, capacity };
  if (g[a].count(b) == 0) {
    g[a][b] = a_to_b;
    return true;
  }
  return false;
}

void print_graph(graph& g) {
  int i = 0;
  for (auto &liste_de_voisins: g) {
    cout << "Voisins de " << i << ": ";
    for(auto& [neighbor, arc]: liste_de_voisins){
      cout << "(" << neighbor << ": " << arc.capacity << ", " << arc.flow << ") ";
    }
    cout << endl;
    ++i;
  }
}

void reset_graph(graph& g) {
  for (auto &liste_de_voisins: g) {
    for(auto& [neighbor, arc]: liste_de_voisins){
      arc.flow = 0;
      arc.residual = arc.capacity;
    }
  }
}

void symmetrize(graph& g) {
  int i = 0;
  for (auto &liste_de_voisins: g) {
    for(auto& [neighbor, arc]: liste_de_voisins){
      if (g[neighbor].count(i) == 0) {
	      add_arc(g, neighbor, i, 0);
      }
    }
    ++i;
  }
}

graph read_graph(string filename) {
  ifstream file(filename);
  if(!file){
    cerr << "Could not open " << filename << endl;
    throw(1);
  }

  int nb_nodes;
  file >> nb_nodes;
  graph result = make_empty_graph(nb_nodes);
  for(int i = 0; i < nb_nodes; ++i) {
    int nb_arcs;
    file >> nb_arcs;
    for(int j = 0; j < nb_arcs; ++j) {
      int neighbor;
      double cost;
      file >> neighbor >> cost;
      add_arc(result, i, neighbor, cost);
    }
  }
  symmetrize(result);
  return result;
}

// Parcours en largeur (BFS)
vector<int> search(graph &g, int source, int sink) {
  vector<int> predecessors(g.size(), -1);
  queue<int> to_visit;

  predecessors[source] = source;
  to_visit.push(source);

  if (verbose) {
    cerr << "Start visit: " << endl;
  }

  while( (!(to_visit.empty())) && (predecessors[sink] < 0)) {
    int visited = to_visit.front();
    to_visit.pop();

    if (verbose) {
      cerr << " " << visited << ": ";
    }

    for (auto& [neighbor, arc]: g[visited]) {
      if((arc.residual > 1e-9) && (predecessors[neighbor] == -1)) {
	predecessors[neighbor] = visited;
	to_visit.push(neighbor);

	if (verbose) {
	  cerr << neighbor << " ";
	}

      }
    }

    if (verbose) {
      cerr << endl;
    }
  }

  if (verbose) {
    cerr << endl;
  }

  return predecessors;
}

double flow_on_path(graph &g, int source, int sink, vector<int> &preds) {
  int node = sink;
  double delta = std::numeric_limits<double>::max();
  if (verbose) {
    cerr << "Path: ";
  }
  while (preds[node] != node) {
    if (verbose) {
      cerr << node << " ";
    }
    Arc &arc = g[preds[node]].at(node);
    delta = min(delta, arc.residual);
    node = preds[node];
  }
  return delta;
}

void update(graph &g, int source, int sink, vector<int> &preds, double delta) {
  int node = sink;
  while (preds[node] != node) {
    Arc &arc = g[preds[node]].at(node);
    Arc &symmetric = g[node].at(preds[node]);

    arc.residual -= delta;
    symmetric.residual += delta;

    if (symmetric.flow > delta) {
      symmetric.flow -= delta;
    } else {
      arc.flow += delta - symmetric.flow;
      symmetric.flow = 0;
    }

    node = preds[node];
  }
}

double fordfulkerson(graph &g, int source, int sink) {
  double flow = 0;
  bool finished = false;
  int nb_steps = 0;
  while(!finished) {
    vector<int> preds = search(g, source, sink);

    if (verbose) {
      cout << "Done computing preds, result= ";
      for(auto &p: preds) {
	cout << p << " ";
      }
      cout << endl;
    }
    if(preds[sink] == -1) {
      finished = true;
    }
    else {
      double delta = flow_on_path(g, source, sink, preds);
      flow += delta;
      update(g, source, sink, preds, delta);
    }
    nb_steps++;
  }
  cerr << "Nb steps: " << nb_steps << endl;
  return flow;
}

bool check(graph &g, int source, int sink, double flow) {
  int u = 0;
  for(auto &neighbors: g) {
    double excess = 0;
    for(auto &[v, arc]: neighbors) {
      excess += arc.flow;
      Arc &symmetric = g[v].at(u);
      excess -= symmetric.flow;
      if( (fabs(symmetric.flow) > 1e-9) && (fabs(arc.flow) > 1e-9)) {
	cerr << "Arcs " << u << " <-> " << v << " both have non-zero flow: " << arc.flow << " " << symmetric.flow << endl;
	return false;
      }
    }
    if(u == source){
      if (fabs(excess - flow) > 1e-9) {
	cerr << "Flow around source " << u << " is wrong " << excess << " " << flow << endl;
	return false;
      }
    } else if (u == sink) {
      if (fabs(excess + flow) > 1e-9) {
	cerr << "Flow around sink " << u << " is wrong " << excess << " " << flow << endl;
	return false;
      }
    } else if (fabs(excess) > 1e-9) {
      cerr << "Flow around node " << u << " is wrong " << excess << " " << flow << endl;

      return false;
    }
    ++u;
  }
  return (true);
}





int main(int argc, char* argv[]) {
  string filename = "test.dat";

  if (argc >= 2) {
    filename = argv[1];
  }

  graph g = read_graph(filename);

  if (verbose) {
    print_graph(g);
  }

  int s = 0, t = 9;
  double val = fordfulkerson(g, s, t);
  cout << "Flow from " << s << " to " << t << ": " << val << endl;
  cout << "Validity " << (check(g, s, t, val) ? "OK" : "WRONG") << endl;
  reset_graph(g);
  
  if(!verbose) {
    for(int s = 0; s < min((int)g.size(), 3); ++s) {
      for(int t = max(0, (int)g.size() - 3); t < (int)g.size(); ++t) {
	if(s != t) {
	  double val = fordfulkerson(g, s, t);
	  cout << "Flow from " << s << " to " << t << ": " << val << endl;
	  cout << "Validity " << (check(g, s, t, val) ? "OK" : "WRONG") << endl;
	  reset_graph(g);
	}
      }
    }
  }

  return 0; 
}
