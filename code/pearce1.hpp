#include <stack>
#include <iostream>
#include <vector> 
#include <stdlib.h> 

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/config.hpp>
#include <boost/type_traits/conversion_traits.hpp>


using namespace boost;

typedef adjacency_list<vecS, vecS, bidirectionalS> Graph;
typedef graph_traits<Graph>::vertex_descriptor Vertex;
typedef graph_traits<Graph>::vertex_iterator vertex_iter;
typedef typename graph_traits<Graph>::out_edge_iterator out_edge_iter; 

class pearce_imper_scc_finder {

public:
	pearce_imper_scc_finder(const Graph& g_, std::vector<int> rindex_, std::vector<bool> root_) 
						: g(g_), rindex(rindex_), root(root_)
	{ }

	int scc () {
		//Initialization 
		index=1;
		c= num_vertices(g)-1;
		std::pair<vertex_iter, vertex_iter> vp;
		//Loop over vertices
		for (vp = vertices(g); vp.first != vp.second; ++vp.first){
			if (rindex[*vp.first] == 0) {
				visit(*vp.first);
			}
		}
		// Number of SCC 
		return num_vertices(g)-c-1;
	}

private:

	void finishEdge(Vertex v, int k){
		//Create help, to find the k-th out-edge of v
		out_edge_iter ai, ai_end;
		std::vector<out_edge_iter> help;
		for (tie(ai, ai_end) = out_edges(v, g); ai != ai_end; ++ai) {
         	help.push_back(ai);
        }
		Vertex w= target (*help[k],g) ;
		if (rindex[w]< rindex[v]){
			rindex[v]=rindex[w];
			root[v]=false;
		}
	}

	bool beginEdge(Vertex v, int k){
		//Create help, to find the k-th out-edge of v
		out_edge_iter ai, ai_end;
		std::vector<out_edge_iter> help;
		for (tie(ai, ai_end) = out_edges(v, g); ai != ai_end; ++ai) {
         	help.push_back(ai);
        }
		Vertex w= target (*help[k],g) ;		                   
		if (rindex[w]==0){
			iS.pop();
			iS.push(k+1);			
			beginVisiting(w);
			return true;
		}else{
			return false;
		}
	}

	void finishVisiting(Vertex v){
		//Pop from "DFS" Stack
		vS.pop();
		iS.pop();
		if (root[v]){
			index=index-1;
			while (!S.empty() && (rindex[v] <= rindex [S.top()])){
				//Removes evrything from backtracking stack, same component as v
				Vertex w=S.top();
				S.pop();
				rindex[w]=c;
				index=index-1;
			}
			rindex[v]=c;
			//v's component members found and marked
			c=c-1;
		}else{
			//v is not a root, store it on backtracking stack
			S.push(v);  	
		}
	}

	void beginVisiting(Vertex v){
		//Store on stack
		vS.push(v);
		iS.push(0); 
		root[v]=true;
		rindex[v]=index;
		index=index+1;
	}


	void visitLoop(){
		Vertex v=vS.top(); 
		int i=iS.top();
		out_edge_iter ai, ai_end;
		// help is used to store out edges of v
		// Based on Pearce's Java implementation
		std::vector<out_edge_iter> help; 
		for (tie(ai, ai_end) = out_edges(v, g); ai != ai_end; ++ai) {
         	help.push_back(ai);
        }
		while (i <= out_degree(v,g)){
			if ( i>0 ){
				finishEdge(v, i-1);
			}
			if ( i< out_degree(v,g) && beginEdge(v,i)){
				return;
			}
			i=i+1;	
		}
		finishVisiting(v);
	}
	
	void visit (Vertex v){
		beginVisiting(v);
		while (!vS.empty()){
			visitLoop();
		}
	}

	Graph g;
	std::stack<Vertex> vS, S;  // vS impements DFS stack
							   // S implements Backtracking stack
	std::stack<int> iS;		   // iS is the imperative version of recursive's call stack 
	std::vector<int> rindex;
  	std::vector<bool> root;
  	int index, c;
};

int strong_components(const Graph& g, std::vector<int> rindex, std::vector<bool> root){

	pearce_imper_scc_finder finder(g, rindex, root);
	return finder.scc();
}

