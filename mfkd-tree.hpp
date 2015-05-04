// @xl242
#ifndef MFKD_TREE_H_
#define MFKD_TREE_H_

#include <iostream>
#include <string>
#include "config.hpp"
#include "tuple.hpp"
#include "../HybridMemory.hpp"
using std::cout;
using namespace hmindex;

const int MFKD_DEBUG = false;
const bool MFKD_KEY_SORTED = true;

// Node of kd-tree
struct csmfnode_t {
	int depth; // Depth of node in the tree
	int num_children;
	int childindex; // Number-th of child of its parent
	void* children;
	csmfnode_t* parent;
	vector<tuple_t> values;
};

// Class for kd-tree
class csmftree_t {
public:
	csmfnode_t* root;
	csmftree_t(config_t& config) : 
			root(NULL), 
			config(config) {
		nodesize = sizeof(csmfnode_t) +
				sizeof(datatype_t) * config.dimension * (config.fanout - 1);
	}

	~csmftree_t();

	// Build a tree from a list of points, 
	void buildfrom(vector<tuple_t>& points);

	csmfnode_t* insert(tuple_t& tuple, HybridMemory::MEMORY_NODE_TYPE type);

	void remove(csmfnode_t* node);

  // Search nearest neighbor
	csmfnode_t* search_nearest(tuple_t& target, datatype_t& sdist) const;
	
	// Print the kd-tree
	void display() const; 
	void print_node(csmfnode_t* node) const;

	int get_dimension() const {
		return config.dimension;
	}

private:
	// Configuration of the hybrid memory allocation
	config_t config;
	
	int nodesize;

	// Level of tree nodes from leaves should be resides in NVM
	int nvm_level = -1;
	
	// Depth of tree nodes from root should be resides in DRAM
	int memory_depth = -1;

	// Determine how to spread nodes across memory base
	void check_config(int num_points);

	// Check whether a node should be allocated in memory
	bool shouldbe_inmemory(int h, int d) const;
	
	// Insert newnode into children nodes of parent at childindex
	csmfnode_t* get_child(csmfnode_t* parent, int index) const;
	
	bool is_null(csmfnode_t* node) const;

	csmfnode_t* buildfrom_helper(
			vector<tuple_t>& points,
			int lbd, int rbd, int depth, csmfnode_t* parent, int childindex) const;
	
	// Find parent of tuple if it to be inserted into tree rooted at starter
	// willbe_child is the index of child node going to be
	csmfnode_t* find_parent(
			csmfnode_t* starter, tuple_t& tuple, int& willbe_child) const;
	
	void display_helper(csmfnode_t* node, string label) const;
 	
	csmfnode_t* search_nearest_helper(
			csmfnode_t* starter, tuple_t& target, datatype_t& sdist) const;

	// Helper function for destructor
	void free_tree_helper(csmfnode_t* start);
	void free_node(csmfnode_t* node);
	void free_children(csmfnode_t* parent);

	void replace_node_value(csmfnode_t* replaced, int vindex);
	csmfnode_t* find_largest(
			csmfnode_t* start, int comp_axis, int& index_smallest) const;
	csmfnode_t* find_smallest(
			csmfnode_t* start, int comp_axis, int& index_smallest) const;
	int index_of_smallest(csmfnode_t* node, int axis) const;
	int index_of_largest(csmfnode_t* node, int axis) const;
};
#endif
