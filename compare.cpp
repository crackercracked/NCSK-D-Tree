#include <ctime>
#include <chrono>
#include <assert.h>
#include <cstdlib>
#include <iostream>
#include <vector>
#include "mfkd-tree.hpp"
#include "kd-tree.hpp"
using std::vector;
using std::cout;


int main(int argc, char** argv) {
	if (argc != 5) {
		cout << "Usage: <path> <dimension> <fanout> <range>\n";
		exit(1);
	}
	string filename(argv[1]);
	cout << "build tree from " << filename << "\n";
	int dimension = atoi(argv[2]);
	int fanout = atoi(argv[3]);
	time_t tstart, tend;
//	int range = atoi(argv[4]);
//	int numsearches = atoi(argv[5]);
	vector<tuple_t> points = createTuplesFromFile(filename, dimension); 
	unsigned int numPoints = 20000; //points.size()
	
	config_t kd_config(dimension, BY_PERCENTILE, 0, 2);
	config_t mfkd_config(dimension, BY_PERCENTILE, 0, fanout);
	tree_t kd_tree(kd_config);
	csmftree_t mfkd_tree(mfkd_config);

	// Test kd build and nns
	cout << "start tree load\n";
//	tstart = time(0);
//	kd_tree.buildfrom(points);
//	tend = time(0);
//	cout << "finished kd tree laod. Used ";
//	cout << difftime(tend, tstart) << "s\n";
//	cout << "start tree nns\n";
//	tstart = time(0);
//	for (int i = points.size() - 1; i >= 0; i--) {
//		kd_tree.search_nearest(points[i]);
//	}
//	tend = time(0);
//	cout << "finished kd tree search. Used ";
//	cout << difftime(tend, tstart) << "s\n";
	
	// Test mfkd build and nns
	cout << "start tree load\n";
	tstart = time(0);
	mfkd_tree.buildfrom(points);
	tend = time(0);
	cout << "finished kd tree laod. Used ";
	cout << difftime(tend, tstart) << "s\n";
	cout << "start tree nns\n";
	tstart = time(0);
	for (int i = points.size() - 1; i >= 0; i--) {
		datatype_t sdist;
		mfkd_tree.search_nearest(points[i], sdist);
	}
	tend = time(0);
	cout << "finished kd tree search. Used ";
	cout << difftime(tend, tstart) << "s\n";



	return 0;

	// Test insert
	cout << "start tree insert\n";
	tstart = time(0);
	for (unsigned int i = 0; i < numPoints; i++) {
		kd_tree.insert(points[i], hmindex::HybridMemory::DRAM);
	}
	tend = time(0);
	cout << "finished kd tree insert. Used ";
	cout << difftime(tend, tstart) << "s\n";

	tstart = time(0);
	cout << "start csmf tree insert\n";
	for (unsigned int i = 0; i < numPoints; i++) {
		mfkd_tree.insert(points[i], hmindex::HybridMemory::DRAM);
	}
	tend = time(0);
	cout << "finished cache sensitive nary kd tree insert. Used ";
	cout << difftime(tend, tstart) << "s\n";

	tstart = time(0);
	cout << "start tree nns\n";
	for (unsigned int i = 0; i < numPoints; i++) {
		tuple_t target = points[i];
	//	node_t* nearest = kd_tree.search_nearest(target);
		kd_tree.search_nearest(target);
	}
	tend = time(0);
	cout << "finished kd tree nns. Used ";
	cout << difftime(tend, tstart) << "s\n";
	
	tstart = time(0);
	cout << "start csmf tree nns\n";
	for (unsigned int i = 0; i < numPoints; i++) {
//		tuple_t target = generate_tuple(dimension, range);		
		tuple_t target = points[i];
		datatype_t sdist;
//		csmfnode_t* nearest = mfkd_tree.search_nearest(target, sdist);
		mfkd_tree.search_nearest(target, sdist);
	}
	tend = time(0);
	cout << "finished csmf tree nns. Used ";
	cout << difftime(tend, tstart) << "s\n";

	return 0;
}
