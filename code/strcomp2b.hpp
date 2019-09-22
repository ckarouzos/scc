//
//=======================================================================
// Copyright 1997, 1998, 1999, 2000 University of Notre Dame.
// Authors: Andrew Lumsdaine, Lie-Quan Lee, Jeremy G. Siek
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
//

#ifndef BOOST_GRAPH_STRONG_COMPONENTS_HPP
#define BOOST_GRAPH_STRONG_COMPONENTS_HPP

#include <stack>
#include <boost/config.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/type_traits/conversion_traits.hpp>
#include <boost/static_assert.hpp>
#include <boost/graph/overloading.hpp>
#include <boost/concept/assert.hpp>
#include <iostream>





namespace boost {

  //==========================================================================
  // This is Tarjan's algorithm for strongly connected components
  // from his paper "Depth first search and linear graph algorithms".
  // It calculates the components in a single application of DFS.
  // We implement the algorithm as a dfs-visitor.
  
  namespace detail {
    
    template <typename ComponentMap, typename RootMap, typename DiscoverTime,
              typename Stack>
    class nuutila2_scc_visitor : public dfs_visitor<> 
    {
      typedef typename property_traits<ComponentMap>::value_type comp_type;
      typedef typename property_traits<DiscoverTime>::value_type time_type;
    public:
      nuutila2_scc_visitor(ComponentMap comp_map, RootMap r, DiscoverTime d, 
                         comp_type& c_, Stack& s_)
        : c(c_), comp(comp_map), root(r), discover_time(d),
          dfs_time(time_type()), s(s_) {   }

      template <typename Graph>
      void discover_vertex(typename graph_traits<Graph>::vertex_descriptor v,
                           const Graph&) {
        put(root, v, v);
        put(comp, v, (std::numeric_limits<comp_type>::max)());
        put(discover_time, v, dfs_time++);
        // Initialize a stack to contain a value < any node in Graph
        if (dfs_time==1) s.push(-1);
        //  Vector for instant check if v is in the Stack s
        inStack.resize(dfs_time);
        inStack[dfs_time]=false;
      }
      
      template <typename Graph>
      void finish_vertex(typename graph_traits<Graph>::vertex_descriptor v,
                         const Graph& g) {
        //Only final candidate root nodes on the stack
        typename graph_traits<Graph>::vertex_descriptor w;
        typename graph_traits<Graph>::out_edge_iterator ei, ei_end;
        for (boost::tie(ei, ei_end) = out_edges(v, g); ei != ei_end; ++ei) {
          w = target(*ei, g);
          // if not InComponent[root[w]] then ... (before: if not InComponent[w]... )
          if (get(comp, get(root, w)) == (std::numeric_limits<comp_type>::max)())
            put(root, v, this->min_discover_time(get(root,v), get(root,w)));
        }
        if (get(root, v) == v) {
          //Check if s is not empty, to use .top()
          if (!s.empty()){
            //Removes if TOP(stack)>= v
            if (s.top()!=-1 && get(discover_time, s.top())>=get(discover_time, v)){
              do{
                w=s.top(); s.pop();
                // Update inStack
                inStack[get(discover_time, w)]=false;
                put(comp, w, c);
                if(s.empty()){
                  break;
                }  
              }while(s.top()!=-1 && get(discover_time, s.top())>=get(discover_time, v));
            }else{
              //roots of trivial components are not stored on the stack
              //ComponentMap is updated 
              put(comp,v,c);
            }
          } 
          ++c;  
        }else if(!inStack[get(discover_time, get(root,v))]){
            //Stores on stack the final candidate root nodes
            s.push(get(root,v));
            //Update inStack
            inStack[get(discover_time, v)]=true;
        }
  }
    private:
      template <typename Vertex>
      Vertex min_discover_time(Vertex u, Vertex v) {
        return get(discover_time, u) < get(discover_time,v) ? u : v;
      }

      comp_type& c;
      ComponentMap comp;
      RootMap root;
      DiscoverTime discover_time;
      time_type dfs_time;
      Stack& s, copy;
      std::vector<bool> inStack;
    };
    
    template <class Graph, class ComponentMap, class RootMap,
              class DiscoverTime, class P, class T, class R>
    typename property_traits<ComponentMap>::value_type
    strong_components_impl
      (const Graph& g,    // Input
       ComponentMap comp, // Output
       // Internal record keeping
       RootMap root, 
       DiscoverTime discover_time,
       const bgl_named_params<P, T, R>& params)
    {
      typedef typename graph_traits<Graph>::vertex_descriptor Vertex;
      BOOST_CONCEPT_ASSERT(( ReadWritePropertyMapConcept<ComponentMap, Vertex> ));
      BOOST_CONCEPT_ASSERT(( ReadWritePropertyMapConcept<RootMap, Vertex> ));
      typedef typename property_traits<RootMap>::value_type RootV;
      BOOST_CONCEPT_ASSERT(( ConvertibleConcept<RootV, Vertex> ));
      BOOST_CONCEPT_ASSERT(( ReadWritePropertyMapConcept<DiscoverTime, Vertex> ));

      typename property_traits<ComponentMap>::value_type total = 0;

      std::stack<Vertex> s;
      detail::nuutila2_scc_visitor<ComponentMap, RootMap, DiscoverTime, 
        std::stack<Vertex> > 
        vis(comp, root, discover_time, total, s);
      depth_first_search(g, params.visitor(vis));
      return total;
    }
  

    //-------------------------------------------------------------------------
    // The dispatch functions handle the defaults for the rank and discover
    // time property maps.
    // dispatch with class specialization to avoid VC++ bug

    template <class DiscoverTimeMap>
    struct strong_comp_dispatch2 {
      template <class Graph, class ComponentMap, class RootMap, class P, class T, class R>
      inline static typename property_traits<ComponentMap>::value_type
      apply(const Graph& g,
            ComponentMap comp,
            RootMap r_map,
            const bgl_named_params<P, T, R>& params,
            DiscoverTimeMap time_map)
      {
        return strong_components_impl(g, comp, r_map, time_map, params);
      }
    };


    template <>
    struct strong_comp_dispatch2<param_not_found> {
      template <class Graph, class ComponentMap, class RootMap,
                class P, class T, class R>
      inline static typename property_traits<ComponentMap>::value_type
      apply(const Graph& g,
            ComponentMap comp,
            RootMap r_map,
            const bgl_named_params<P, T, R>& params,
            param_not_found)
      {
        typedef typename graph_traits<Graph>::vertices_size_type size_type;
        size_type       n = num_vertices(g) > 0 ? num_vertices(g) : 1;
        std::vector<size_type> time_vec(n);
        return strong_components_impl
          (g, comp, r_map,
           make_iterator_property_map(time_vec.begin(), choose_const_pmap
                                      (get_param(params, vertex_index),
                                       g, vertex_index), time_vec[0]),
           params);
      }
    };

    template <class Graph, class ComponentMap, class RootMap,
              class P, class T, class R, class DiscoverTimeMap>
    inline typename property_traits<ComponentMap>::value_type
    scc_helper2(const Graph& g,
                ComponentMap comp,
                RootMap r_map,
                const bgl_named_params<P, T, R>& params,
                DiscoverTimeMap time_map)
    {
      return strong_comp_dispatch2<DiscoverTimeMap>::apply(g, comp, r_map, params, time_map);
    }

    template <class RootMap>
    struct strong_comp_dispatch1 {

      template <class Graph, class ComponentMap, class P, class T, class R>
      inline static typename property_traits<ComponentMap>::value_type
      apply(const Graph& g,
            ComponentMap comp,
            const bgl_named_params<P, T, R>& params,
            RootMap r_map)
      {
        return scc_helper2(g, comp, r_map, params, get_param(params, vertex_discover_time));
      }
    };
    template <>
    struct strong_comp_dispatch1<param_not_found> {

      template <class Graph, class ComponentMap, 
                class P, class T, class R>
      inline static typename property_traits<ComponentMap>::value_type
      apply(const Graph& g,
            ComponentMap comp,
            const bgl_named_params<P, T, R>& params,
            param_not_found)
      {
        typedef typename graph_traits<Graph>::vertex_descriptor Vertex;
        typename std::vector<Vertex>::size_type
          n = num_vertices(g) > 0 ? num_vertices(g) : 1;
        std::vector<Vertex> root_vec(n);
        return scc_helper2
          (g, comp, 
           make_iterator_property_map(root_vec.begin(), choose_const_pmap
                                      (get_param(params, vertex_index),
                                       g, vertex_index), root_vec[0]),
           params, 
           get_param(params, vertex_discover_time));
      }
    };

    template <class Graph, class ComponentMap, class RootMap,
              class P, class T, class R>
    inline typename property_traits<ComponentMap>::value_type
    scc_helper1(const Graph& g,
               ComponentMap comp,
               const bgl_named_params<P, T, R>& params,
               RootMap r_map)
    {
      return detail::strong_comp_dispatch1<RootMap>::apply(g, comp, params,
                                                           r_map);
    }

  } // namespace detail 

  template <class Graph, class ComponentMap, 
            class P, class T, class R>
  inline typename property_traits<ComponentMap>::value_type
  strong_components(const Graph& g, ComponentMap comp,
                    const bgl_named_params<P, T, R>& params
                    BOOST_GRAPH_ENABLE_IF_MODELS_PARM(Graph, vertex_list_graph_tag))
  {
    typedef typename graph_traits<Graph>::directed_category DirCat;
    BOOST_STATIC_ASSERT((is_convertible<DirCat*, directed_tag*>::value == true));
    return detail::scc_helper1(g, comp, params, 
                               get_param(params, vertex_root_t()));
  }

  template <class Graph, class ComponentMap>
  inline typename property_traits<ComponentMap>::value_type
  strong_components(const Graph& g, ComponentMap comp
                    BOOST_GRAPH_ENABLE_IF_MODELS_PARM(Graph, vertex_list_graph_tag))
  {
    typedef typename graph_traits<Graph>::directed_category DirCat;
    BOOST_STATIC_ASSERT((is_convertible<DirCat*, directed_tag*>::value == true));
    bgl_named_params<int, int> params(0);
    return strong_components(g, comp, params);
  }

  template <typename Graph, typename ComponentMap, typename ComponentLists>
  void build_component_lists
    (const Graph& g,
     typename graph_traits<Graph>::vertices_size_type num_scc,
     ComponentMap component_number,
     ComponentLists& components)
  {
    components.resize(num_scc);
    typename graph_traits<Graph>::vertex_iterator vi, vi_end;
    for (boost::tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi)
      components[component_number[*vi]].push_back(*vi);
  }


} // namespace boost

#ifdef BOOST_GRAPH_USE_MPI
#  include <boost/graph/distributed/strong_components.hpp>
#endif

#endif // BOOST_GRAPH_STRONG_COMPONENTS_HPP
