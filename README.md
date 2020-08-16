# Parallel k-NN

An implementation of KNN algorithm using KD Tree.


Author :
============
Sagar Vishwakarma (svishwa2@binghamton.edu)

State University of New York, Binghamton


File :
============

1)	Makefile                   - Compile the program
2)	k-nn.cpp                   - Contains implementation of KNN algorithm
3)	k-nn.hpp                   - Contains declaration of KNN algorithm
4)	bst.cpp                    - Contains implementation of Binary Tree, KD Tree
5)	bst.hpp                    - Contains declaration of Binary Tree, KD Tree
6)	res/query_file.py          - To generate binary query file.
7)	res/training_data.py       - To generate binary training file.


Run :
============

- Open a terminal in project directory      : make (to build project)
- To generate training file                 : python training_data.py
- To generate query file                    : python query_file.py
- To run KNN algorithm                      : ./k-nn n_cores training_file query_file result_file
