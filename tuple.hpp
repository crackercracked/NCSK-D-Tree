// @xl242
#ifndef KD_TUPLE_H_
#define KD_TUPLE_H_

#include <sstream>
#include <cmath>
#include <string>
#include <vector>
using std::string;
using std::vector;
using std::ostringstream;

typedef long long datatype_t;
typedef vector<datatype_t> tuple_t;

// Print a tuple_t
string tuple_string(tuple_t t);

// calculate norm2 distance between two points
datatype_t distance(tuple_t& t1, tuple_t& t2);

// Swap position of two tuples in the list
inline void swap_tuples_in_points(vector<tuple_t>& points, int i1, int i2) {
	tuple_t tmp = points[i1];
	points[i1] = points[i2];
	points[i2] = tmp;
}

// Return the index of median of points[lbd:rbd+1] using quick find by
// axis-th coordinates of each point
int quickfind_tuples_by_axis(
		vector<tuple_t>& points,
	  int lbd, int& rbd, unsigned int axis, int right_median);

tuple_t generate_tuple(int dimension, int base);
vector<tuple_t> generate_tuples(int dimension, int size);
vector<tuple_t> generate_sortedtuples(int dimension, int size);
void remove_point_fr_pool(vector<tuple_t>& points, tuple_t& target);

// Create a list of tuple from .dat file created by dataGen
vector<tuple_t> createTuplesFromFile(
	const string& dataFilePath, int dimension);
#endif
