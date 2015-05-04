// @xl242
#include <assert.h>
#include <algorithm>
#include <stdlib.h>
#include <cstdlib>
#include <limits.h>
#include <iostream>
#include <string.h>
#include <string>
#include "mfkd-tree.hpp"
#include "../FatalError.hpp"

//===========================================================
//    MFKDTREE PUBLIC FUNCTIONS
//===========================================================

/* A deconstructor to free all memory allocated */
csmftree_t::~csmftree_t() {
	if (root == NULL) return;
	free_tree_helper(root);
}

void csmftree_t::buildfrom(vector<tuple_t>& points) {
	check_config(points.size());
	root = buildfrom_helper(points, 0, points.size()-1, 0, NULL, 0);
}

csmfnode_t* csmftree_t::insert(tuple_t& tuple, HybridMemory::MEMORY_NODE_TYPE type) {
	if (root == NULL) {
		csmfnode_t* newnode = (csmfnode_t*)HybridMemory::alloc(nodesize, type);
		memset(newnode, 0, nodesize); 
		newnode->values.push_back(tuple);
		newnode->parent = NULL;
		root = newnode;
		return newnode;
	}
	int willbe_child = -1;
	csmfnode_t* parent = find_parent(root, tuple, willbe_child);
	if (parent == NULL) {
		cout << "bad\n";
	}
	if ((int)parent->values.size() < config.fanout - 1) {
		int i = 0;
		int axis = parent->depth % config.dimension;
		while (i < (int)parent->values.size() &&
				tuple[axis] >= parent->values[i][axis]) {
			i++;
		}
		parent->values.insert(parent->values.begin()+i, tuple);
		return parent;
	}

	if (willbe_child == -1) return parent;
	if (parent->num_children == 0) {
		parent->children = HybridMemory::alloc(nodesize * config.fanout, type);
		memset(parent->children, 0, nodesize * config.fanout);
	}
	csmfnode_t* newnode = get_child(parent, willbe_child);
	newnode->parent = parent;
	newnode->depth = parent->depth + 1;
	newnode->childindex = willbe_child;
	parent->num_children++;
	if (newnode->values.size() == 0) {
		newnode->values.push_back(tuple);
	} else{
		int axis = newnode->depth % config.dimension;
		unsigned int i = 0;
		while (i < newnode->values.size() &&
				newnode->values[i][axis] < tuple[axis]) i++;
		newnode->values.insert(newnode->values.begin() + i, tuple);
	}
	return newnode;
}
	
void csmftree_t::remove(csmfnode_t* node) {
	if (node == NULL) return;
	if (node->num_children == 0) {
		if (node->parent == NULL) {
			root = NULL;
			free_node(node);
			return;
		}
		memset(node, 0, nodesize);
		return;
	}
	unsigned int i = 0;
	while (i < node->values.size()) {
		replace_node_value(node, i);
		i++;
	}
}


void csmftree_t::display() const {
	cout << "kd-tree: \n";
	display_helper(root, "root<0>: ");
}

csmfnode_t* csmftree_t::search_nearest(tuple_t& target, datatype_t& sdist) const {
	return search_nearest_helper(root, target, sdist);
}

//===========================================================
//      KDTREE PRIVATE FUNCTIONS
//===========================================================

void csmftree_t::free_node(csmfnode_t* node) {
	try { 
		HybridMemory::assertAddress(node, HybridMemory::DRAM);
		HybridMemory::free(node, nodesize, HybridMemory::DRAM);
	} catch (FatalError& err) {
		try {
			HybridMemory::assertAddress(node, HybridMemory::NVM);
			HybridMemory::free(node, nodesize, HybridMemory::NVM);
		} catch (FatalError& err) {
			cout << "Encounter invalid memory type " << node << "\n";
		}
	} 
}

void csmftree_t::free_children(csmfnode_t* parent) {
	if (parent == NULL) return;
	void* children = parent->children;
	try { 
		HybridMemory::assertAddress(children, HybridMemory::DRAM);
		HybridMemory::free(children, nodesize * config.fanout, HybridMemory::DRAM);
	} catch (FatalError& err) {
		try {
			HybridMemory::assertAddress(children, HybridMemory::NVM);
			HybridMemory::free(children, nodesize * config.fanout, HybridMemory::NVM);
		} catch (FatalError& err) {
			cout << "Encounter invalid memory type " << (csmfnode_t*)children << "\n";
		}
	}
}

void csmftree_t::free_tree_helper(csmfnode_t* start) {
/*	if (start->num_children == 0) {
		if (start->parent != NULL) {
			start->parent->num_children--;
		}
		free_node(start);
		return;
	}
	vector<csmfnode_t*> ptrs;
	for (int i = 0; i < config.fanout; i++) {
		ptrs.push_back(get_child(start, i));
	}
	for (unsigned int i = 0; i < ptrs.size(); i++) {
//		free_tree_helper(ptrs[i]);
	}
	free_node(start);
*/
}

void csmftree_t::check_config(int num_points) {
	switch (config.policy) {
		case BY_HEIGHT_FROM_BOTTOM:
			nvm_level = config.value;
			break;
		case BY_PERCENTILE:
			cout << num_points << " points in total\n";
			int num_inmemory_nodes = int(num_points * (1.0 - config.value));
			int memory_depth = 0;
			if (num_inmemory_nodes != 0) {
				memory_depth = bottomheight(num_inmemory_nodes, config.fanout);
			}
			cout << memory_depth << " depth of nodes will be in memory\n";
			break;
	} // Switch
}

bool csmftree_t::shouldbe_inmemory(int h, int d) const {
	switch (config.policy) {
		case BY_HEIGHT_FROM_BOTTOM:
			return h > nvm_level;
		case BY_PERCENTILE:
			return d <= memory_depth; 
		default:
			return false;
	}
}

csmfnode_t* csmftree_t::get_child(csmfnode_t* parent, int index) const {
	if (index >= config.fanout) {
		if (MFKD_DEBUG) {
			cout << "bad get child for " << parent << " at " << index << "\n";
		}
		throw -1;
	}
	char* ptr = (char*)parent->children;
	for (int i = 0; i < index; i++) {
		for (int k = 0; k < nodesize; k++) ptr++;
	}
	return (csmfnode_t*)ptr;
}

// Base on pesudo-code on wikipedia
csmfnode_t* csmftree_t::buildfrom_helper(
		vector<tuple_t>& points,
		int lbd, int rbd, int depth, csmfnode_t* parent, int childindex) const {
	if (lbd > rbd) return NULL;
	
	// Assume key all sorted at this point
	assert(MFKD_KEY_SORTED);
	vector<int> lbds;
	if (lbd != rbd) {
		int unitsize = (rbd - lbd + 1) / config.fanout;
		int index = 0;
		for (int i = 0; i < config.fanout; i++) {
			lbds.push_back(lbd + index);
			index += std::max(1, unitsize);
			if (lbd + index > rbd) break;
		}
	}
	csmfnode_t* newnode = NULL;
	int height = bottomheight(rbd - lbd + 1, config.fanout);
	if (parent != NULL) {
		if (parent->num_children == 0) {
			if (shouldbe_inmemory(depth, height)) {
				parent->children = HybridMemory::alloc(
						nodesize * config.fanout, HybridMemory::DRAM);
			} else {
				parent->children = HybridMemory::alloc(
						nodesize * config.fanout, HybridMemory::NVM);
			}
			memset(parent->children, 0, nodesize * config.fanout);
		}
		newnode = get_child(parent, childindex);
		parent->num_children++;
	} else {
		if (shouldbe_inmemory(depth, height)) {
			newnode = (csmfnode_t*)HybridMemory::alloc(
					sizeof(csmfnode_t), HybridMemory::DRAM);
		} else {
			newnode = (csmfnode_t*)HybridMemory::alloc(
					sizeof(csmfnode_t), HybridMemory::NVM);
		}
		memset(newnode, 0, nodesize);
	}
	newnode->depth = depth;
	newnode->parent = parent;
	newnode->num_children = 0;
	newnode->childindex = childindex;
	if (rbd - lbd + 1 <= config.fanout - 1) {
		for (int i = lbd; i <= rbd; i++) {
			newnode->values.push_back(points[i]);
		}
		return newnode;
	}

	for (unsigned int i = 1; i < lbds.size(); i++) {
		newnode->values.push_back(points[lbds[i]]);
	}
	buildfrom_helper(points, lbds[0], lbds[1]-1, depth+1, newnode, 0);
	for (unsigned int i = 1; i < lbds.size() - 1; i++) {
		buildfrom_helper(points, lbds[i]+1, lbds[i+1]-1, depth+1, newnode, i);
	}
	buildfrom_helper(points, lbds.back()+1, rbd, depth+1, newnode, lbds.size()-1);
	return newnode;
}

bool csmftree_t::is_null(csmfnode_t* node) const {
	char* nullchild = (char*)malloc(nodesize);
	memset(nullchild, 0, nodesize);
	int ret = strncmp((char*)node, nullchild, nodesize);
	free(nullchild);
	return ret == 0;
}

void csmftree_t::print_node(csmfnode_t* node) const {
	for (unsigned int i = 0; i < node->values.size() - 1; i++) {
		cout << tuple_string(node->values[i]) << "|";
	} cout << tuple_string(node->values.back());
	// Debug statement
	if (MFKD_DEBUG) {
		cout <<" :" << node;
		cout << " -> " << node->parent;
	} cout << "\n";
}

void csmftree_t::display_helper(csmfnode_t* node, string label) const {
	if (node == NULL) return;
	cout << string(2*node->depth, ' ') << label;
	print_node(node);
	if (node->num_children == 0) return;
	for (int i = 0; i < config.fanout; i++) {
		csmfnode_t* child = get_child(node, i);
		bool isnull = is_null(child);
		if (!isnull) {
			int axis = child->depth % config.dimension;
			display_helper(
					child, std::to_string(i) +
					"<" + std::to_string(axis) + ">: ");
		}
	}
}

csmfnode_t* csmftree_t::find_parent(
		csmfnode_t* starter, tuple_t& target, int& willbe_child) const {
	if (starter == NULL) return NULL;
	csmfnode_t* key = starter;
	while (true) {
		for (unsigned int i = 0; i < key->values.size(); i++) {
			if (key->values[i] == target) {
				willbe_child = -1;
				return key;
			}
		}
		int axis = key->depth % config.dimension;
		if (key->num_children == 0) {
			willbe_child = 0;
			while(willbe_child < (int)key->values.size() && 
					target[axis] >= key->values[willbe_child][axis]) {
				willbe_child++;
			}
			return key;	
		}
		bool csearch = false;
		for (unsigned int i = 0; i < key->values.size(); i++) {
			if (target[axis] < key->values[i][axis]) {
				csmfnode_t* child = get_child(key, i);
				if (is_null(child)) {
					willbe_child = i;
					return key;
				}
				key = child;
				csearch = true;
				break;
			}
		}
		if (csearch) continue;
		csmfnode_t* lastchild = get_child(key, key->values.size());
		if (is_null(lastchild)) {
			willbe_child = (int)key->values.size();
			return key;
		}
		key = lastchild;
	}
	return NULL;
}

void csmftree_t::replace_node_value(csmfnode_t* replaced, int vindex) {
	if (replaced == NULL) return;
	if (replaced ->num_children == 0) {
		replaced->values.erase(replaced->values.begin() + vindex);
		if (replaced->values.size() == 0) {
			if (replaced->parent == NULL) {
				assert(replaced == root);
				free_node(replaced);
				root = NULL;
				return;
			}
			replaced->parent->num_children--;
			if (replaced->parent->num_children == 0) {
				free_children(replaced->parent);
				return;
			}
			csmfnode_t* rstart = get_child(replaced->parent, replaced->childindex);
			memset(rstart, 0, nodesize);
		}
		return;
	}
	int axis = replaced->depth % config.dimension;
	// Probe left
	csmfnode_t* probe = NULL;
	for (int c = vindex; c >= 0; c--) {
		probe = get_child(replaced, c);
		if (!is_null(probe)) break;
		else probe = NULL;
	}
	if (probe != NULL) {
		int index_largest;
		csmfnode_t* r = find_largest(probe, axis, index_largest);
		replaced->values[vindex] = r->values[index_largest];
		return replace_node_value(r, index_largest);
	}

	// Probe right
	for (int c = vindex + 1; c < config.fanout; c++) {
		probe = get_child(replaced, c);
		if (!is_null(probe)) break;
		else probe = NULL;
	}
	assert(probe != NULL);
	int index_smallest;
	csmfnode_t* r = find_smallest(probe, axis, index_smallest);
	replaced->values[vindex] = r->values[index_smallest];
	return replace_node_value(r, index_smallest);
}


int csmftree_t::index_of_largest(csmfnode_t* node, int axis) const {
	if (node->depth % config.dimension == axis) return node->values.size()-1;
	int ret = 0;
	for (unsigned int i = 1; i < node->values.size(); i++) {
		if (node->values[ret][axis] < node->values[i][axis]) ret = i;
	}
	return ret;
}

csmfnode_t* csmftree_t::find_largest(csmfnode_t* start, int comp_axis, int& index) const {
	if (start == NULL) return start;
	int idx =	index_of_largest(start, comp_axis);
	if (start->num_children == 0) {
		index = idx;
		return start;
	}
	int axis = start->depth % config.dimension;
	if (axis == comp_axis) {
		csmfnode_t* probe = NULL;
		for (int i = config.fanout - 1; i >= 0; i--) {
			probe = get_child(start, i);
			if (!is_null(probe)) {
				if (probe->childindex <= idx) {
					probe = NULL;
				}
				break;
			}
			else probe = NULL;
		}
		if (probe == NULL) {
			index = idx;
			return start;
		}
		return find_largest(probe, comp_axis, index);
	}
	csmfnode_t* replacement = start;
	int idx_largest = idx;
	for (int i = 0; i < config.fanout; i++) {
		csmfnode_t* probe = get_child(start, i);
		if (is_null(probe)) continue;
		int idx;
		csmfnode_t* cnode = find_largest(probe, comp_axis, idx);
		if (cnode->values[idx][comp_axis] >
				replacement->values[idx_largest][comp_axis]) {
			replacement = cnode;
			idx_largest = idx;
		}
	}
	index = idx_largest;
	return replacement;
}

int csmftree_t::index_of_smallest(csmfnode_t* node, int axis) const {
	if (node->depth % config.dimension == axis) return 0;
	int ret = 0;
	for (unsigned int i = 1; i < node->values.size(); i++) {
		if (node->values[ret][axis] > node->values[i][axis]) ret = i;
	}
	return ret;
}

csmfnode_t* csmftree_t::find_smallest(csmfnode_t* start, int comp_axis, int& index) const {
	if (start == NULL) return start;
	int idx =	index_of_smallest(start, comp_axis);
	if (start->num_children == 0) {
		index = idx;
		return start;
	}
	int axis = start->depth % config.dimension;
	if (axis == comp_axis) {
		csmfnode_t* probe = NULL;
		for (int i = 0; i < config.fanout; i++) {
			probe = get_child(start, i);
			if (!is_null(probe)) {
				if (probe->childindex > idx) {
					probe = NULL;
				}
				break;
			}
			else probe = NULL;
		}
		if (probe == NULL) {
			index = idx;
			return start;
		}
		return find_smallest(probe, comp_axis, index);
	}
	csmfnode_t* replacement = start;
	int idx_smallest = idx;
	for (int i = 0; i < config.fanout; i++) {
		csmfnode_t* probe = get_child(start, i);
		if (is_null(probe)) continue;
		int idx;
		csmfnode_t* cnode = find_smallest(probe, comp_axis, idx);
		if (cnode->values[idx][comp_axis] <
				replacement->values[idx_smallest][comp_axis]) {
			replacement = cnode;
			idx_smallest = idx;
		}
	}
	index = idx_smallest;
	return replacement;
}

datatype_t smallest_distdiff_innode(
		csmfnode_t* node, tuple_t& target) {
	datatype_t cur_dist = (datatype_t)LLONG_MAX;
	for (unsigned int i = 0; i < node->values.size(); i++) {
		cur_dist = std::min(cur_dist, distance(node->values[i], target));
	}
	return cur_dist;
}

csmfnode_t* csmftree_t::search_nearest_helper(
		csmfnode_t* starter, tuple_t& target, datatype_t& sdist) const {
	if (starter == NULL) return NULL;
	int willbe_child = 0;
	csmfnode_t* cur_best = find_parent(starter, target, willbe_child);
	if (willbe_child == -1) {
		sdist = 0;
		return cur_best;
	}

	datatype_t cur_dist = smallest_distdiff_innode(cur_best, target);
	csmfnode_t* p = cur_best;
	if (cur_best->num_children != 0 ) {
		for (int i = 0; i < config.fanout; i++) {
			csmfnode_t* child = get_child(p, i);
			bool isnull = is_null(child);
			if (isnull) continue;
			datatype_t canddist;
			csmfnode_t* candidate = search_nearest_helper(child, target, canddist);
			if (canddist < cur_dist) {
				cur_best = candidate;
				cur_dist = canddist;
			}
		}
	}

	csmfnode_t* key = cur_best->parent;
	csmfnode_t* prev = cur_best;
	int childindex = cur_best->childindex;
	while (key != NULL && prev != starter) {
		datatype_t d = smallest_distdiff_innode(key, target);
		if (d < cur_dist) {
			cur_best = key;
			cur_dist = d;
		}
		int axis = key->depth % config.dimension;
		// Probe left if there is any
		for (int c = childindex - 1; c >= 0; c--) {
			csmfnode_t* probe = get_child(key, c);
			bool isnull = is_null(probe);
			if (isnull) continue;
			datatype_t distbd = abs(target[axis] - key->values[childindex-1][axis]);
			if (cur_dist > distbd) {
				datatype_t canddist;
				csmfnode_t* candidate = search_nearest_helper(probe, target, canddist);
				if (canddist < cur_dist) {
					cur_best = candidate;
					cur_dist = canddist;
				}
			} else {
				break;
			}
		}
		// Probe right if there is any
		for (int c = childindex+1; c <= (int)key->values.size(); c++) {
			csmfnode_t* probe = get_child(key, c);
			bool isnull = is_null(probe);
			if (isnull) continue;
			datatype_t distbd = abs(target[axis] - key->values[childindex][axis]);
			if (cur_dist > distbd) {
				datatype_t canddist;
				csmfnode_t* candidate = search_nearest_helper(probe, target, canddist);
				if (canddist < cur_dist) {
					cur_best = candidate;
					cur_dist = canddist;
				}
			} else {
				break;
			}
		}
		childindex = key->childindex;
		prev = key;
		key = key->parent;
	} // While
	sdist = cur_dist;
	return cur_best;
}

