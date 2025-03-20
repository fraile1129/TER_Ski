#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <cmath>

#include "TER_ski.hpp"


void print_graph(graph& g);

void reset_graph(graph& g);

void symmetrize(graph& g);

double fordfulkerson(graph &g, int source, int sink);
