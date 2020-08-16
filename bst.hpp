/*
 *  bst.hpp
 *  Contains declaration of Binary Tree, KD Tree
 *
 *  Author :
 *  Sagar Vishwakarma (svishwa2@binghamton.edu)
 *  State University of New York, Binghamton
 */


#ifndef _BST_HPP_
#define _BST_HPP_

#include <cstdint>
#include <vector>
#include <cmath>
#include <cassert>
#include <cstdlib>
#include <atomic>
#include <chrono>
#include <iostream>
#include <algorithm>
#include <queue>

using namespace std;


class Node {

	public:
		Node *left_child;
		Node *right_child;

		Node(vector<float> value,  uint64_t length) :
		left_child(nullptr),
		right_child(nullptr),
		value(value),
		kd(length) {
		}

		void insert_node(Node **pp, Node *node);
		void verify_node(Node *node, uint64_t level);
		void print_node_value();
		vector<float> get_node_value();
		void set_node_distance(float distance);
		float get_node_distance();
		uint64_t get_node_dimentions();

		~Node();

	private:
		vector<float> value;
		uint64_t kd;
		float distance;
};


struct DimensionsCompare {

	DimensionsCompare(uint64_t dimension):
		dimension(dimension) {
	}

	const uint64_t dimension;

	bool operator ()(Node *a, Node *b) {
		return a->get_node_value().at(dimension) < b->get_node_value().at(dimension);
	}
};


struct DistanceCompare {
	bool operator ()(Node *a, Node *b) {
		return a->get_node_distance() < b->get_node_distance();
	}
};


// Wrapper around a pointer, for reading values from byte sequence.
class Reader {

	public:
		Reader (const char *p):
			ptr(p) {
		}

		template <typename T>
		Reader &operator >> (T &o) {
			// Assert alignment.
			assert(uintptr_t(ptr) % sizeof(T) == 0);
			o = *(T *) ptr;
			ptr += sizeof(T);
			return *this;
		}

	private:
		const char *ptr;
};


void print_points(vector<Node*> all_points);

float euclidean_distance(Node *query, Node *node);

class NodeQuery {

	public:
		Node *node;

		priority_queue<Node*, vector<Node*>, DistanceCompare> result_q;

		~NodeQuery();
};


class KDTree {

	public:
		Node *root;

		void populate_kdtree(vector<Node*> all_points, uint64_t num_cols, uint64_t level);
		void set_dimenstion(uint64_t dimension);
		void insert_tree(Node **root, Node* node, uint64_t level);
		void verify_tree();
		void traverse_tree(Node *node);
		void find_nearest_neighbor(NodeQuery* query, Node* node, uint64_t level, uint64_t num_neighbors);

		~KDTree();

	private:
		uint64_t dimension;
};


#endif /* _BST_HPP_ */
