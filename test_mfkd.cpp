// @xl242
#include <ctime>
#include <assert.h>
#include <cstdlib>
#include <iostream>
#include <stack>
#include <unordered_map>
#include "mfkd-tree.hpp"

bool is_nearest(vector<tuple_t>& points, tuple_t& target, datatype_t actual_sdist) {
	for (unsigned int i = 0; i < points.size(); i++) {
		double d = distance(points[i], target);
		if (d < actual_sdist) {
			if (MFKD_DEBUG) {
				cout << "Err! nns for " << tuple_string(target);
				cout << "Found nearer: " << tuple_string(points[i]) << " (" << d << ").\n";
			}
			return false;
		}
	}
	return true;
}

void experiment_randomnns(
		csmftree_t* kdtree, vector<tuple_t>& points,
		int num_trials, int randbase) {
	csmfnode_t* nearest = NULL;
	for (int i = 0; i < num_trials; i++) {
		tuple_t target = generate_tuple(kdtree->get_dimension(), randbase);		
		if (MFKD_DEBUG) {
			cout << "wt find nearest for " << tuple_string(target) << "\n";
		}
		datatype_t sdist;
		nearest = kdtree->search_nearest(target, sdist);
		if (MFKD_DEBUG) {
			cout << "found nearest for " << tuple_string(target);
			cout << ": " << sdist << " as ";
			kdtree->print_node(nearest);
		}
		assert(is_nearest(points, target, sdist));
	}
}

void experiment_inputnns(
		csmftree_t* kdtree, vector<tuple_t>& points) {
	csmfnode_t* nearest = NULL;
	for (unsigned int i = 0; i < points.size(); i++) {
		tuple_t target = points[i];
		datatype_t sdist;
		nearest = kdtree->search_nearest(target, sdist);
		assert(nearest != NULL);
	}
}


void testInsertRemove() {
	cout << "Lets test kd's insert and remove\n";
	int num_points = 20;
	int dimension = 2;
	int base = 1000;
	int fanout = 3;
	config_t config(
			dimension, 
			BY_PERCENTILE,
			1.0, // Useless here
			fanout);
	csmftree_t kdtree(config);
	vector<tuple_t> points;
	vector<csmfnode_t*> nodes;
	for (int i = 0; i < num_points; i++) {
		tuple_t tuple = generate_tuple(dimension, base);
		cout << "wt insert " << tuple_string(tuple) << "\n";
		points.push_back(tuple);
		nodes.push_back(kdtree.insert(tuple, hmindex::HybridMemory::DRAM));
	}
	if (MFKD_DEBUG) {
		kdtree.display();
	}
	int num_trials = 1;
	experiment_randomnns(&kdtree, points, num_trials, base); 

//	kdtree.replace_node_value(kdtree.root, 1);
/*	remove_point_fr_pool(points, kdtree.root->values[0]);
	kdtree.replace_node_value(kdtree.root, 0);
	kdtree.display();
	int num_trials = 100;
	experiment_randomnns(&kdtree, points, num_trials, base); 
*/
	while (kdtree.root != NULL) {
		for (unsigned int i = 0; i < kdtree.root->values.size(); i++) {
			remove_point_fr_pool(points, kdtree.root->values[i]);
		}
		kdtree.remove(kdtree.root);
		if (MFKD_DEBUG) {
			kdtree.display();
		}
		if (kdtree.root != NULL) {
			experiment_randomnns(&kdtree, points, num_trials, 2*base); 
		}
	}
}


void testSingleAndMultDimension(string pathname) {
	cout << "Test for single dimension\n";
	int dimension = 1;
	int fanout = 10;
	int num_trials = 2000;
	int base = 200000;
	vector<tuple_t> points = createTuplesFromFile(pathname, dimension); 
//	vector<tuple_t> points = generate_sortedtuples(dimension, 200);
	config_t config(
			dimension, 
			BY_PERCENTILE,
			0.99,
			fanout);
	tuple_t target(dimension);
	
	csmftree_t tree(config);
	time_t tstart = time(0);
	tree.buildfrom(points);
	time_t tend = time(0);
	cout << "finished building the tree. Used ";
	cout << difftime(tend, tstart) << "s\n";
	if (MFKD_DEBUG) {
		tree.display();
	}

	// Test knn search
	cout << "start nns...\n";
	tstart = time(0);
	experiment_randomnns(&tree, points, num_trials, base);
	tend = time(0);
	cout << "Used: "<< difftime(tend, tstart) << "s\n";
}

void testSpeed(string pathname, int dimension, int fanout) {
	cout << "Test for speed\n";
	vector<tuple_t> points = createTuplesFromFile(pathname, dimension); 
	config_t config( dimension, BY_PERCENTILE, 0.0, fanout);
	tuple_t target(dimension);
	
	csmftree_t tree(config);
	time_t tstart = time(0);
	tree.buildfrom(points);
	time_t tend = time(0);
	cout << "finished building the tree. Used ";
	cout << difftime(tend, tstart) << "s\n";
	
	// Test knn search
	cout << "start nns...\n";
	tstart = time(0);
	experiment_inputnns(&tree, points);
	tend = time(0);
	cout << "Used: "<< difftime(tend, tstart) << "s\n";
}

int main(int argc, char** argv) {
	if (argc != 4) {
		cout << "Please define a data source, dimension and fanout\n";
		exit(1);
	}
	string filename(argv[1]);
//	int dimension = atoi(argv[2]);
//	int fanout = atoi(argv[3]);
	cout << "build tree from " << filename << "\n";
//	testSpeed(filename, dimension, fanout);
	


//	testInsertRemove();
	testSingleAndMultDimension(filename);
	return 0;
}
