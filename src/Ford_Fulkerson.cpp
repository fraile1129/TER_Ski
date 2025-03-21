#include "Ford_Fulkerson.hpp"

// Set to true for debugging messages
bool verbose = true;



void print_graph(graph& g) {
  int i = 0;
  for (auto &liste_de_voisins: g) {
    cout << "Voisins de " << i << ": ";
    for(auto& pair: liste_de_voisins){
      cout << "(" << pair.first << ": " << pair.second.capacity << ", " << pair.second.flow << ") ";
    }
    cout << endl;
    ++i;
  }
}

void reset_graph(graph& g) 
    {
        for (auto &liste_de_voisins: g) {
            for(auto& pair: liste_de_voisins){
              pair.second.flow = 0;
              pair.second.residual = pair.second.capacity;
            }
        }
    }


void symmetrize(graph& g)
    {
        for (int i=0; i<int(g.size()); i++) {
            for(auto& pair: g[i]) {
                Arc a = {0.,0.,0.};
                g[pair.first][i] = a;
                /*if (g[neighbor].count(i) == 0) {
                    //add_arc(g, neighbor, i, 0);
                }*/
            }
        }
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

    for (auto& pair: g[visited]) {
      if((pair.second.residual > 1e-9) && (predecessors[pair.first] == -1)) {
	predecessors[pair.first] = visited;
	to_visit.push(pair.first);

	if (verbose) {
	  cerr << pair.first << " ";
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
    for(auto &pair: neighbors) {
      excess += pair.second.flow;
      Arc &symmetric = g[pair.first].at(u);
      excess -= symmetric.flow;
      if( (fabs(symmetric.flow) > 1e-9) && (fabs(pair.second.flow) > 1e-9)) {
	cerr << "Arcs " << u << " <-> " << pair.first << " both have non-zero flow: " << pair.second.flow << " " << symmetric.flow << endl;
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
