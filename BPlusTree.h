#ifndef BPlusTree_h
#define BPlusTree_h
#include <iostream>
#include <vector>
#include <string>
#include <condition_variable>
#include <queue>

using namespace std;


class BPT_Node{
public:
	std::vector<int> keys; // keys in node
	BPT_Node* parent; // parent node of this node
	int keys_num; // number of keys in node
	bool is_leaf; // leaf node or not
};


class Leaf_Node : public BPT_Node {
public:
	Leaf_Node* next; // pointer of next node
	Leaf_Node* pre; // pointer of pre node
	std::vector<double> values; // value of key
	Leaf_Node() {
		keys_num = 0;
		is_leaf = true;
		next = NULL;
		pre = NULL;
		parent = NULL;
	}
};


class Internal_Node : public BPT_Node {
public:
	std::vector<BPT_Node*> children; // pointer of children node
	Internal_Node() {
		keys_num = 0;
		is_leaf = false;
		parent = NULL;
	}
};


class BPlusTree {
public:
	BPlusTree(int order);
	~BPlusTree();
	
	void Insert(int key, double value); 
	double Search(int key); 
	std::vector<double> Search(int key1, int key2);
	void Delete(int key); 
 
	void destory_bpt(); 
	void print_leaves(); 
	void print_tree(); 
	
private:
	BPT_Node* bpt_root; // pointer of root
	int bpt_order; // max order of node
	int min_order;  // min order of node
	Leaf_Node* leaves_head; // pointer of far left node

	void insert_directly(BPT_Node* insert_pos, int key, void* value); // insert the pait into the pointed node 
	void node_split(BPT_Node* insert_pos); // pointer of splited node
	Leaf_Node* find_node_ptr(int key); // pointer of inserted node
	double* find_data_ptr(int key); // pointer of value with key
	int find_child_num(BPT_Node* raw_node); // the sequence of its parent node

};
#endif
