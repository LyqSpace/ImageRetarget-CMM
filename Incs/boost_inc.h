#pragma once
#define BOOST_USE_WINDOWS_H
#pragma warning(push,0)

//auto link
#ifdef _M_CEE_PURE
	#define BOOST_ALL_NO_LIB
	#ifdef _DEBUG
		#pragma comment(lib, "libboost_program_options-vc90-mt-clr-gd-1_36.lib")
	#else
		#pragma comment(lib, "libboost_program_options-vc90-mt-clr-1_36.lib")
	#endif
#endif

//algorithm
#include <boost/algorithm/string.hpp>

//graph
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/graph/kruskal_min_spanning_tree.hpp>

//program option
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

//util
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/multi_array.hpp>


#pragma warning(pop)
