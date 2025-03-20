#include "test.hpp"


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
                graph.resize(numNodes);
            }
            else if (temp == "LINKS")
            {
                int node1, node2;
                double capacity = 1.0;
                while (inf >> node1 >> node2)
                {
                    if (graph[node1].find(node2) != graph[node1].end())
                    {
                        doublons.emplace_back(node1, node2);
                    }
                    else
                    {
                        Arc arc;
                        arc.capacity = capacity;
                        arc.flow = 0.0;
                        arc.residual = capacity;
                        graph[node1][node2] = arc;
                    }
                }
            }
        }
        size = numNodes;
        inf.close();
    }
