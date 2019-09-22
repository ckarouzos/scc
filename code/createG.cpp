// STL
#include <iostream>                  // for std::cout
#include <stdlib.h> 
#include <algorithm>
#include <ctime>
#include <vector> 
#include <fstream>

// Boost
#include <boost/graph/adjacency_list.hpp> // for customizable graphs
#include <boost/graph/directed_graph.hpp> // A subclass to provide reasonable arguments to adjacency_list for a typical directed graph
#include <boost/graph/undirected_graph.hpp>// A subclass to provide reasonable arguments to adjacency_list for a typical undirected graph
#include <boost/graph/strong_components.hpp> 
#include <boost/config.hpp>

#include <boost/graph/graphviz.hpp>
#include <boost/graph/graph_utility.hpp>

  typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS> Graph;


void RandDirectedGraph(Graph &g, int N, int M);
 
int main(int argc, char * argv [])
{
  using namespace boost;

  std::string a1=argv[1];
  std::string a2=argv[2];
  int N =std::atoi(a1.c_str());
  int M =std::atoi(a2.c_str());
  
  Graph G(N);

  std::ofstream myfile (argv[3]);

  RandDirectedGraph(G, N, M);
  
  if (myfile.is_open())
  {
    write_graphviz(myfile, G);  
    myfile.close();
  }
  return 0;
}


void RandDirectedGraph(Graph &g, int N, int M)
{
  // directed_graph is a subclass of adjacency_list which gives you object oriented access to functions like add_vertex and add_edge, which makes the code easier to understand. However, it hard codes many of the template parameters, so it is much less flexible.

  for (int i=0; i<M; i++){
    int k1= rand() % N;
    int k2= rand() % N;
    boost::add_edge(k1, k2, g);
  }
 }