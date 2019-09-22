// STL
#include <iostream>                  // for std::cout
#include <stdlib.h> 
#include <algorithm>
#include <ctime>
#include <vector> 
#include <fstream>
#include <string>

// Boost
#include <boost/graph/adjacency_list.hpp> // for customizable graphs
#include <boost/graph/directed_graph.hpp> // A subclass to provide reasonable arguments to adjacency_list for a typical directed graph
#include <boost/graph/undirected_graph.hpp>// A subclass to provide reasonable arguments to adjacency_list for a typical undirected graph
#include <pearce1.hpp> 
#include <boost/config.hpp>

#include <boost/graph/graphviz.hpp>
#include <boost/graph/graph_utility.hpp>

typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS> Graph;
 
int main(int argc, char * argv [])
{
  using namespace boost;

  std::string a1=argv[1];
  std::string a2=argv[2];
  int N =std::atoi(a1.c_str());
  int M =std::atoi(a2.c_str());

  Graph G(N);
  
  std::string line;
  std::ifstream myfile (argv[3]);

  int i=0; 
  if(myfile.is_open()){
    while(getline(myfile,line)){

      if (i<=N){

      }
      else if (line.compare("}")!=0){
        int k=0;
        std::string p="-";
        std::string t=";";
        while (line[k]!=p[0]){
          k++;
        }
        std::string num1=line.substr(0,k);
        int l=k;
        l++;
        l++;
        while(line[l]!=t[0]){
          l++;
        }
        std::string num2=line.substr(k+2,l);
        int v1= std::atoi(num1.c_str());
        int v2= std::atoi(num2.c_str());
        boost::add_edge(v1, v2, G);
      }

      i++;
    }
  }
 
  typedef graph_traits<adjacency_list<vecS, vecS, directedS> >::vertex_descriptor Vertex;
    
  std::vector<int> rindex(num_vertices(G));
  std::vector<bool> root(num_vertices(G));
  int num = strong_components(G,rindex,root);

  std::cout<<"Number of SCC: "<<num<<std::endl;

  return 0;
}