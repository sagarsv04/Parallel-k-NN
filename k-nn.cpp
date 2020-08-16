/*
 *  knn.cpp
 *  Contains implementation of KNN algorithm
 *
 *  Author :
 *  Sagar Vishwakarma (svishwa2@binghamton.edu)
 *  State University of New York, Binghamton
 */

#include "k-nn.hpp"

#include <functional>

#define DEBUG 0
#define PARALLEL 1


void KNN::build_tree(char *training_file) {

	this->tree = new KDTree();

	int fd = open(training_file, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "Training Failed ...\n");
		fprintf(stderr, "Failed to open %s : %s\n", training_file, strerror(errno));
		exit(2);
	}

	// Get the actual size of the file.
	struct stat sb;
	int ret_val;

	ret_val = fstat(fd, &sb); assert(ret_val == 0);

	// Make sure that the size is a multiple of the size of a double.
	assert(sb.st_size % sizeof(double) == 0);

	// Use some flags that will hopefully improve performance.
	void *vp = mmap(nullptr, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
	if (vp == MAP_FAILED) {
		fprintf(stderr, "mmap() failed: %s\n", strerror(errno));
		exit(3);
	}

	char *file_mem = (char *) vp;
	// Tell the kernel that it should evict the pages as soon as possible.
	ret_val = madvise(vp, sb.st_size, MADV_SEQUENTIAL | MADV_WILLNEED); assert(ret_val == 0);
	ret_val = close(fd); assert(ret_val == 0);

	// Start to read data, skip the file type string.
	Reader reader {file_mem + 8};

	reader >> training_file_id >> num_training_points >> num_dimensions;

	cout << "Training file ID: " << training_file_id << endl;
	cout << "Number of points: " << num_training_points << endl;
	cout << "Number of dimensions: " << num_dimensions << endl;

	vector<Node*> all_points;

	for (uint64_t i = 0; i < num_training_points; i++) {
		vector<float> p;
		for (uint64_t j = 0; j < num_dimensions; j++) {
			float f;
			reader >> f;
			p.push_back(f);
		}
		Node *node = new Node(p, num_dimensions);
		all_points.push_back(node);
	}

	tree->set_dimenstion(num_dimensions);

	cout << "Building Tree Please Wait ... " << endl;
	auto start = chrono::high_resolution_clock::now();
	// populate tree from level 0
	tree->populate_kdtree(all_points, num_dimensions, 0);

	auto stop = chrono::high_resolution_clock::now();
	auto duration = chrono::duration<double>(stop - start);

	cout << "Build Complete ... " << duration.count() << " sec.\n" << endl;

	// free the memory
	ret_val = munmap(file_mem, sb.st_size); assert(ret_val == 0);
}


void KNN::run_query(char *query_file, char *result_file) {

	int fd = open(query_file, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "Query Failed ...\n");
		fprintf(stderr, "Failed to open %s : %s\n", query_file, strerror(errno));
		exit(2);
	}

	// Get the actual size of the file.
	struct stat sb;
	int ret_val;

	ret_val = fstat(fd, &sb); assert(ret_val == 0);

	// Make sure that the size is a multiple of the size of a double.
	assert(sb.st_size % sizeof(double) == 0);

	// Use some flags that will hopefully improve performance.
	void *vp = mmap(nullptr, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
	if (vp == MAP_FAILED) {
		fprintf(stderr, "mmap() failed: %s\n", strerror(errno));
		exit(3);
	}

	char *file_mem = (char *) vp;
	// Tell the kernel that it should evict the pages as soon as possible.
	ret_val = madvise(vp, sb.st_size, MADV_SEQUENTIAL | MADV_WILLNEED); assert(ret_val == 0);
	ret_val = close(fd); assert(ret_val == 0);

	// Start to read data, skip the file type string.
	Reader reader {file_mem + 8};

	reader >> query_file_id >> num_queries >> num_dimensions >> num_neighbors;

	cout << "Query file ID: " << query_file_id << endl;
	cout << "Number of queries: " << num_queries << endl;
	cout << "Number of dimensions: " << num_dimensions << endl;
	cout << "Number of neighbors: " << num_neighbors << endl;

	vector<NodeQuery*> all_queries;

	for (uint64_t i = 0; i < num_queries; i++) {
		vector<float> q;
		for (uint64_t j = 0; j < num_dimensions; j++) {
			float f;
			reader >> f;
			q.push_back(f);
		}
		NodeQuery *q_node = new NodeQuery();
		priority_queue<Node*, vector<Node*>, DistanceCompare> res;
		q_node->result_q = res;
		q_node->node = new Node(q, num_dimensions);
		all_queries.push_back(q_node);
	}

	ofstream result;
	char result_file_header[8] = "RESULT\0";
	time_t rawtime;
	struct tm * timeinfo;
	char buffer[80];

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(buffer, 8, "%d%H%MS", timeinfo);

	result.open(result_file, ios::binary);
	result.write(result_file_header, sizeof(result_file_header));
	result.write(reinterpret_cast<char *>(&training_file_id), 8);
	result.write(reinterpret_cast<char *>(&query_file_id), 8);
	result.write(buffer, 8);
	result.write(reinterpret_cast<char *>(&num_queries), 8);
	result.write(reinterpret_cast<char *>(&num_dimensions), 8);
	result.write(reinterpret_cast<char *>(&num_neighbors), 8);

	auto start = chrono::high_resolution_clock::now();

	for (uint64_t i = 0; i < num_queries; i++) {
		NodeQuery *query = all_queries.at(i);
		tree->find_nearest_neighbor(query, tree->root, 0, num_neighbors);
		priority_queue<Node*, vector<Node*>, DistanceCompare> res = all_queries.at(i)->result_q;

		while (!res.empty()) {
			Node *temp_node = res.top();
			vector<float> p = temp_node->get_node_value();

			for (auto it : p) {
				result.write(reinterpret_cast<char *>(&it), sizeof(float));
			}
			res.pop();
		}
	}

	auto stop = chrono::high_resolution_clock::now();
	auto duration = chrono::duration<double>(stop - start);

	cout << "Query Complete ... " << duration.count() << " sec.\n" << endl;

	// free the memory
	ret_val = munmap(file_mem, sb.st_size); assert(ret_val == 0);
}


void KNN::run_query_parallel(char *query_file, char *result_file) {

	int fd = open(query_file, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "Query Failed ...\n");
		fprintf(stderr, "Failed to open %s : %s\n", query_file, strerror(errno));
		exit(2);
	}

	// Get the actual size of the file.
	struct stat sb;
	int ret_val;

	ret_val = fstat(fd, &sb); assert(ret_val == 0);

	// Make sure that the size is a multiple of the size of a double.
	assert(sb.st_size % sizeof(double) == 0);

	// Use some flags that will hopefully improve performance.
	void *vp = mmap(nullptr, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
	if (vp == MAP_FAILED) {
		fprintf(stderr, "mmap() failed: %s\n", strerror(errno));
		exit(3);
	}

	char *file_mem = (char *) vp;
	// Tell the kernel that it should evict the pages as soon as possible.
	ret_val = madvise(vp, sb.st_size, MADV_SEQUENTIAL | MADV_WILLNEED); assert(ret_val == 0);
	ret_val = close(fd); assert(ret_val == 0);

	// Start to read data, skip the file type string.
	Reader reader {file_mem + 8};

	reader >> query_file_id >> num_queries >> num_dimensions >> num_neighbors;

	cout << "Query file ID: " << query_file_id << endl;
	cout << "Number of queries: " << num_queries << endl;
	cout << "Number of dimensions: " << num_dimensions << endl;
	cout << "Number of neighbors to return for each point: " << num_neighbors << endl;

	vector<NodeQuery*> all_queries;

	for (uint64_t i = 0; i < num_queries; i++) {
		vector<float> q;
		for (uint64_t j = 0; j < num_dimensions; j++) {
			float f;
			reader >> f;
			q.push_back(f);
		}
		NodeQuery *q_node = new NodeQuery();
		priority_queue<Node*, vector<Node*>, DistanceCompare> res;
		q_node->result_q = res;
		q_node->node = new Node(q, num_dimensions);
		all_queries.push_back(q_node);
	}

	ofstream result;
	char result_file_header[8] = "RESULT\0";
	time_t rawtime;
	struct tm * timeinfo;
	char buffer[80];

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(buffer, 8, "%d%H%MS", timeinfo);

	result.open(result_file, ios::binary);
	result.write(result_file_header, sizeof(result_file_header));
	result.write(reinterpret_cast<char *>(&training_file_id), 8);
	result.write(reinterpret_cast<char *>(&query_file_id), 8);
	result.write(buffer, 8);
	result.write(reinterpret_cast<char *>(&num_queries), 8);
	result.write(reinterpret_cast<char *>(&num_dimensions), 8);
	result.write(reinterpret_cast<char *>(&num_neighbors), 8);

	auto start = chrono::high_resolution_clock::now();

	vector<thread> vec_threads(num_threads);

	for (int thr = 0; thr < num_threads; thr++) {
		// needs to understand how this lamda works
		vec_threads.at(thr) = thread(bind(
			[&](const int bi, const int ei, const int thr) {
				for(int i = bi;i<ei;i++) {
					NodeQuery *query = all_queries.at(i);
					tree->find_nearest_neighbor(query, tree->root, 0, num_neighbors);
				}
				if (DEBUG) {
					cout << "Thread ... " << thr << endl;
				}
			}, thr * num_queries / num_threads,
			(thr + 1) == num_threads ?
			num_queries : (thr + 1) * num_queries / num_threads, thr
		));
	}

	for_each(vec_threads.begin(), vec_threads.end(), [](thread& x) {x.join();});

	auto stop = chrono::high_resolution_clock::now();

	auto duration = chrono::duration<double>(stop - start);

	cout << "Parallel Query took ... " << duration.count() << " sec.\n" << endl;

	for (uint64_t i = 0; i < num_queries; i++) {
		priority_queue<Node*, vector<Node*>, DistanceCompare> res = all_queries.at(i)->result_q;

		while (!res.empty()) {
			Node *temp_node = res.top();
			vector<float> p = temp_node->get_node_value();

			for (auto it : p) {
				result.write(reinterpret_cast<char *>(&it), sizeof(float));
			}
			res.pop();
		}
	}

	stop = chrono::high_resolution_clock::now();
	duration = chrono::duration<double>(stop - start);

	cout << "Parallel Query Complete ... " << duration.count() << " sec.\n" << endl;

	// free the memory
	ret_val = munmap(file_mem, sb.st_size); assert(ret_val == 0);
}


void KNN::print_result(char *result_file){

	int fd = open(result_file, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "Print Failed ...\n");
		fprintf(stderr, "Failed to open %s : %s\n", result_file, strerror(errno));
		exit(2);
	}

	// Get the actual size of the file.
	struct stat sb;
	int ret_val;

	ret_val = fstat(fd, &sb); assert(ret_val == 0);

	// Make sure that the size is a multiple of the size of a double.
	assert(sb.st_size % sizeof(double) == 0);

	// Use some flags that will hopefully improve performance.
	void *vp = mmap(nullptr, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
	if (vp == MAP_FAILED) {
		fprintf(stderr, "mmap() failed: %s\n", strerror(errno));
		exit(3);
	}

	char *file_mem = (char *) vp;
	// Tell the kernel that it should evict the pages as soon as possible.
	ret_val = madvise(vp, sb.st_size, MADV_SEQUENTIAL | MADV_WILLNEED); assert(ret_val == 0);
	ret_val = close(fd); assert(ret_val == 0);

	// Start to read data, skip the file type string.
	Reader reader {file_mem + 8};

	reader >> training_file_id >> query_file_id >> result_file_id >> num_queries >> num_dimensions >> num_neighbors;

	cout << "Training file ID: " << training_file_id << endl;
	cout << "Query file ID: " << query_file_id << endl;
	cout << "Result file ID: " << result_file_id << endl;
	cout << "Number of queries: " << num_queries << endl;
	cout << "Number of dimensions: " << num_dimensions << endl;
	cout << "Number of neighbors: " << num_neighbors << endl;

	auto start = chrono::high_resolution_clock::now();

	for (uint64_t i = 0; i < num_queries; i++) {
		cout << "Query Result " << i << ":\n";
		for (uint64_t k = 0; k < num_neighbors; k++) {
			cout << "Neighbor " << k << ": ";
			for (uint64_t j = 0; j < num_dimensions; j++) {
				float f;
				reader >> f;
				cout << setprecision(6) << setw(15) << setfill(' ') << f;
				if (j < num_dimensions - 1) {
					cout << ", ";
				}
			}
			cout << endl;
		}
		cout << endl;
	}

	auto stop = chrono::high_resolution_clock::now();
	auto duration = chrono::duration<double>(stop - start);

	cout << "Print Complete ... " << duration.count() << " sec.\n" << endl;

	// free the memory
	ret_val = munmap(file_mem, sb.st_size); assert(ret_val == 0);
}


KNN::~KNN() {
	if (tree) {
		delete tree;
	}
}


int main(int argc, char const* argv[]) {

	if (argc != 5) {
		cerr << "Help : Usage "<< argv[0] << " n_cores training_file query_file result_file" << endl;
		exit(1);
	}
	else {
		// int n_cores, n_threads;
		int n_cores;
		char *training_file = new char[strlen(argv[2])+1];
		char *query_file = new char[strlen(argv[3])+1];
		char *result_file = new char[strlen(argv[4])+1];

		// assign argv values to variables
		n_cores	= atoi(argv[1]);
		strcpy(training_file, argv[2]);
		strcpy(query_file, argv[3]);
		strcpy(result_file, argv[4]);
		if (n_cores==0) {
			cerr << "Error : Number of cores to compute should be greater than 0" << endl;
			exit(1);
		}
		else if ((strlen(training_file)==0)||(strlen(query_file)==0)||(strlen(result_file)==0)) {
			cerr << "Help : Usage "<< argv[0] << " n_cores training_file query_file result_file" << endl;
			exit(1);
		}
		else {
			// code goes here
			printf("num_core :: %d\n", n_cores);
			printf("training_file :: %s\n", training_file);
			printf("query_file :: %s\n", query_file);
			printf("result_file :: %s\n\n", result_file);

			int ret_val;

			KNN *knn = new KNN();
			knn->num_cores = n_cores;
			knn->num_threads = (int)((float)n_cores*1.5); // 1.5 just a safe number considering cpu cores with hyperthreading
			knn->build_tree(training_file);
			if (PARALLEL) {
				knn->run_query_parallel(query_file, result_file);
			}
			else {
				knn->run_query(query_file, result_file);
			}
			if (DEBUG) {
				knn->print_result(result_file);
			}

			struct rusage res_usage;
			ret_val = getrusage(RUSAGE_SELF, &res_usage); assert(ret_val == 0);
			auto cv = [](const timeval &time_val) {
				return double(time_val.tv_sec) + double(time_val.tv_usec)/1000000;
			};
			cerr <<"Resource Usage: \n";
			cerr << "    User CPU Time: " << cv(res_usage.ru_utime) << '\n';
			cerr << "    Sys CPU Time: " << cv(res_usage.ru_stime) << '\n';
			cerr << "    Max Resident: " << res_usage.ru_maxrss << '\n';
			cerr << "    Page Faults: " << res_usage.ru_majflt << '\n';
		}
		delete[] training_file;
		delete[] query_file;
		delete[] result_file;
	}
	return 0;
}
