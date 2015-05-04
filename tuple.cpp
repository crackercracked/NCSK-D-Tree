// @xl242
#include <iostream>
#include <fstream>
#include <vector>
#include "tuple.hpp"
#include "../../dataGen/DataDef.hpp"

using std::cerr;
using std::cout;
using std::vector;
using std::ifstream;
using namespace hmindex;

string tuple_string(tuple_t t) {
	ostringstream buf;
	buf << "(";
	for (unsigned int i = 0; i < t.size() - 1; i++) {
		buf << t[i] << " ";
	} buf << t.back() << ")";
	return buf.str();
}

datatype_t distance(tuple_t& t1, tuple_t& t2) {
	if (t1.size() != t2.size()) return -datatype_t(1);
	datatype_t ret = 0.0;
	for (unsigned int i = 0; i < t1.size(); i++) {
		ret += (t1[i] - t2[i]) * (t1[i] - t2[i]);
	}
	return pow(ret, 0.5);
}

int quickfind_tuples_by_axis(
		vector<tuple_t>& points, int lbd, int& rbd,
		unsigned int axis, int right_median) {
	if (lbd == rbd) return lbd;
	tuple_t pivot = points[lbd];
	int i = lbd+1;
	for (int j = lbd+1; j <= rbd; j++) {
		tuple_t compare = points[j];
		if (compare.size() <= axis) {
			// Tuple compare is a invalid tuple as it doesn't have axis-th coordinate,
			// swap to the end, shrink valid tuples range
			swap_tuples_in_points(points, j, rbd);
			rbd--;
		} 
		if (compare[axis] < pivot[axis]) {
			swap_tuples_in_points(points, i, j);
			i++;
		}
	}
	i--;
	swap_tuples_in_points(points, lbd, i);
	if (right_median == i - lbd) {
		return i;
	}
	if (right_median < i - lbd) {
		int new_rbd = i-1;
	 	return quickfind_tuples_by_axis(points, lbd, new_rbd, axis, right_median);
	}
 	return quickfind_tuples_by_axis(points, i+1, rbd, axis, right_median-(i-lbd+1));	
}

tuple_t generate_tuple(int dimension, int base) {
	tuple_t t(dimension);
	for (int i = 0; i < dimension; i++) {
		int b = rand() % base + 10;
		t[i] = double(rand() % b);
	}
	return t;
}

vector<tuple_t> generate_tuples(int dimension, int size) {
	vector<tuple_t> ret;
	for (int i = 0; i < size; i++) {
		ret.push_back(generate_tuple(dimension, 200));
	}
	return ret;
}

vector<tuple_t> generate_sortedtuples(int dimension, int size) {
	vector<tuple_t> ret;
	int counter = 0;
	for (int i = 0; i < size; i++) {
		tuple_t t(dimension);
		for (int i = 0; i < dimension; i++) {
			t[i] = counter;
			counter += 2;
		}
		ret.push_back(t);
	}
	return ret;
}

void remove_point_fr_pool(vector<tuple_t>& points, tuple_t& target) {
	for (unsigned int i = 0; i < points.size(); i++) {
		if (points[i] == target) {
			points.erase(points.begin() + i);
			return;
		}
	}
}


vector<tuple_t> createTuplesFromFile(
	const string& dataFilePath, int dimension) {
	ifstream dataFile(dataFilePath);
	if (!dataFile.is_open()) {
		cerr << "Unable to print data file: " << dataFilePath << "\n";
		throw -1;
	}

	dataFile.seekg(0, std::ios_base::end);
	size_t dataSize = dataFile.tellg();
	cout << "{ DataFileSize: " << dataSize
			 << "B, tupleSize: " << getTupleSize() << " }\n";
	dataFile.seekg(0);

	char* dataBlock = static_cast<char*>(malloc(dataSize));
	dataFile.read(dataBlock, dataSize);

	vector<tuple_t> ret;
	tuple_t tuple;
	int d = 0;
	for (size_t i = 0; i < dataSize; i += getTupleSize()) {
		// TODO add random base, or salt?
		Tuple* f = reinterpret_cast<Tuple*>(dataBlock + i);
		double data = double(f->key);
		tuple.push_back(data);
		d++;
		if (d == dimension) {
//			cout << tuple_string(tuple) << "\n";
			ret.push_back(tuple);
			tuple.clear();
			d = 0;
		}
	}
	dataFile.close();
	free(dataBlock);
	return ret;
}


