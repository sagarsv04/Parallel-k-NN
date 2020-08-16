/*
 *  knn.hpp
 *  Contains declaration of KNN algorithm
 *
 *  Author :
 *  Sagar Vishwakarma (svishwa2@binghamton.edu)
 *  State University of New York, Binghamton
 */

#ifndef _KNN_HPP_
#define _KNN_HPP_


#include "bst.hpp"
#include <cstdio>
#include <iomanip>
#include <cstring>
#include <thread>
#include <fcntl.h>
#include <unistd.h>
#include <fstream>
#include <set>
#include <map>
#include <chrono>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>


using namespace std;



class KNN {

	public:
		KDTree *tree;
		void build_tree(char *training_file);
		void run_query(char *query_file, char *result_file);
		void run_query_parallel(char *query_file, char *result_file);
		void print_result(char *result_file);

		uint64_t training_file_id;
		uint64_t query_file_id;
		uint64_t result_file_id;
		uint64_t num_queries;
		uint64_t num_training_points;
		uint64_t num_dimensions;
		uint64_t num_neighbors;
		int num_cores;
		int num_threads;

	~KNN();
};


#endif /* _KNN_HPP_ */
