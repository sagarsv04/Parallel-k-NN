/*
 *  bst.cpp
 *  Contains implementation of Binary Tree, KD Tree
 *
 *  Author :
 *  Sagar Vishwakarma (svishwa2@binghamton.edu)
 *  State University of New York, Binghamton
 */

#include "bst.hpp"

// using namespace std;


void Node::insert_node(Node **pp, Node *node) {

	if (*pp == nullptr) {
		*pp = node;
	}
	else {
		Node *p = *pp;
		// May have dupes, in theory.
		if (node->get_node_value() <= p->get_node_value()) {
			this->insert_node(&p->left_child, node);
		}
		else {
			this->insert_node(&p->right_child, node);
		}
	}
}


float Node::get_node_distance() {
	float distance = this->distance;
	return distance;
}


void Node::set_node_distance(float distance) {
	this->distance = distance;
}


vector<float> Node::get_node_value() {
	vector<float> value = this->value;
	return value;
}


void Node::verify_node(Node *node, uint64_t level) {

	if (node->left_child) {
		assert(node->left_child->get_node_value() <= node->get_node_value());
		verify_node(node->left_child, level+1);
	}

	if (node->right_child) {
		assert(node->right_child->get_node_value() > node->get_node_value());
		verify_node(node->right_child, level+1);
	}
}


uint64_t Node::get_node_dimentions() {
	return this->kd;
}


void Node::print_node_value() {

	cout << "Node Dimention ... " << this->get_node_dimentions() << "D" << endl;
	for (size_t i = 0; i < this->get_node_value().size(); i++) {
		cout << this->get_node_value().at(i) << ", ";
	}
	cout << endl;
}


float euclidean_distance(Node *query, Node *node) {

	float distance = 0.0;
	float difference = 0.0;
	float power = 0.0;

	for (size_t i = 0; i < query->get_node_value().size(); i++) {
		difference = query->get_node_value().at(i) - node->get_node_value().at(i);
		power = pow(difference, 2);
		distance += sqrt(power);
	}
	return distance;
}


void KDTree::populate_kdtree(vector<Node*> all_points, uint64_t num_cols, uint64_t level) {

	if (!all_points.empty()) {

		DimensionsCompare compare(level % num_cols);

		sort(all_points.begin(), all_points.end(), compare);

		size_t half_size = all_points.size() / 2;

		Node *median = all_points.at(half_size);

		all_points.erase(all_points.begin() + half_size);

		vector<Node*> split_lo(all_points.begin(), all_points.begin() + half_size);

		vector<Node*> split_hi(all_points.begin() + half_size, all_points.end());

		this->insert_tree(&this->root, median, level);

		populate_kdtree(split_lo, num_cols, level);
		populate_kdtree(split_hi, num_cols, level);
	}
}


void KDTree::find_nearest_neighbor(NodeQuery *query, Node* node, uint64_t level, uint64_t num_neighbors) {

	if (node == nullptr)
		return;

	float distance = euclidean_distance(query->node, node);
	node->set_node_distance(distance);
	uint64_t index = level % this->dimension;

	if (query->result_q.empty()) {
		query->result_q.push(node);
	}
	else if (distance < query->result_q.top()->get_node_distance()) {

		if (query->result_q.size() == num_neighbors) {
			query->result_q.pop();
		}
		query->result_q.push(node);
	}
	else {

		if (query->result_q.size() < num_neighbors) {
			query->result_q.push(node);
		}
		else {
			return;
		}
	}

	if (query->node->get_node_value().at(index) < node->get_node_value().at(index)) {
		// find for left node
		find_nearest_neighbor(query, node->left_child, level + 1, num_neighbors);
		if (abs(query->node->get_node_value().at(index) - node->get_node_value().at(index)) < query->result_q.top()->get_node_distance()) {
			// find for right node
			find_nearest_neighbor(query, node->right_child, level + 1, num_neighbors);
		}
	}
	else {
		// find for right node
		find_nearest_neighbor(query, node->right_child, level + 1, num_neighbors);
		if (abs(query->node->get_node_value().at(index) - node->get_node_value().at(index)) < query->result_q.top()->get_node_distance()) {
			// find for left node
			find_nearest_neighbor(query, node->left_child, level + 1, num_neighbors);
		}
	}
}


void KDTree::insert_tree(Node **pp, Node *node, uint64_t level) {

	if (*pp == nullptr) {
		*pp = node;
	}
	else {
		Node *p = *pp;
		uint64_t index = level % dimension;
		if (node->get_node_value().at(index) < p->get_node_value().at(index)) {
			this->insert_tree(&p->left_child, node, level + 1);
		}
		else {
			this->insert_tree(&p->right_child, node, level + 1);
		}
	}
}


void KDTree::set_dimenstion(uint64_t dimension) {
	this->dimension = dimension;
}


Node::~Node() {
	if (left_child) {
		delete left_child;
	}
	if (right_child) {
		delete right_child;
	}
}

NodeQuery::~NodeQuery() {
	if (node) {
		delete node;
	}
}

KDTree::~KDTree() {
	if (root) {
		delete root;
	}
}
