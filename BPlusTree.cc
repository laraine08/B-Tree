#include "BPlusTree.h"
#include <cmath>
#include <queue>
#include <string>
#include <fstream>
#include <sstream>
using namespace std;


// construct and init BPlusTree
BPlusTree::BPlusTree(int order) {
    this->bpt_order = order;
    this->bpt_root = NULL;
    //each node has at least ceil(m/2) children
    this->min_order = ceil(order / 2.0);
    this->leaves_head = NULL;
}


// deconstruct BPlusTree
BPlusTree::~BPlusTree() {
    destory_bpt();
}


// insert new pair with (key, value)
void BPlusTree::Insert(int key, double value) {
    // find the node need to be inserted 
    Leaf_Node* insert_pos = find_node_ptr(key); 
    if (insert_pos == NULL) {
        this->bpt_root = new Leaf_Node();
        this->leaves_head = (Leaf_Node*)bpt_root;
        insert_directly(bpt_root, key, &value);
        return;
    }

    insert_directly(insert_pos, key, &value);
    // check whether the node is overfull or not
    if (insert_pos->keys_num > bpt_order - 1) {
        node_split(insert_pos);
    }
}


// search the value with its key
double BPlusTree::Search(int key) {
    double* temp = find_data_ptr(key);
    if (temp != NULL) {
        return *temp;
    }
    else {
        cout << "Fail to search." << endl;
        return -1;
    }
}

// search range
std::vector<double> BPlusTree::Search(int key1, int key2) {
    // if key1 > key2, return error
    if (key1 > key2) {
        cout << "Range switch" << endl;
        int tempkey = key1;
        key1 = key2;
        key2 = tempkey;
    }

    // result list
    vector<double> result_list;
    
    // find the leaf node with key1
    BPT_Node* t = find_node_ptr(key1);
    
    //save values < key2 into resultlist
    for (int i = 0; i < t->keys_num; ++i) {
        if (t->keys[i] >= key1 && t->keys[i] <= key2) {
            result_list.push_back(((Leaf_Node*)t)->values[i]);
        }
    }
    
    // get next node contain keys < key2 recursively and save it into result_list
    while (t->keys[t->keys_num - 1] <= key2) {
        t = ((Leaf_Node*)t)->next;
        for (int i = 0; i < t->keys_num; ++i) {
            if (t->keys[i] <= key2) {
                result_list.push_back(((Leaf_Node*)t)->values[i]);
            }
        }
    }

    return result_list;

}


// delete the pair with (key, value)
void BPlusTree::Delete(int key) {
    // find the pointer of the node
    Leaf_Node* delete_pos = find_node_ptr(key); 
    if (delete_pos == NULL) {
        cout << "Fail to delete." << endl;
        return;
    }
    bool is_exist = false;
    // search the key in the leaf node
    for (int i = 0; i < delete_pos->keys_num; i++) { 
        // if find the key then delete
        if (delete_pos->keys[i] == key) {
            delete_pos->keys.erase(delete_pos->keys.begin() + i); 
            ((Leaf_Node*)delete_pos)->values.erase(((Leaf_Node*)delete_pos)->values.begin() + i);
            delete_pos->keys_num--;
            is_exist = true;
            break;
        }
    }
    // if not find the key
    if (!is_exist) { 
        cout << "Fail to delete." << endl;
        return;
    }
    // the order of this node is underfilled
    if (delete_pos->keys_num < min_order - 1 && delete_pos->parent != NULL) { 
        Leaf_Node* right_node = NULL;
        Leaf_Node* left_node = NULL;
        // find the number of this child node
        int child_num = find_child_num(delete_pos); 
        // find left sibling
        if (child_num - 1 >= 0) {
            left_node = (Leaf_Node*)((Internal_Node*)delete_pos->parent)->children[child_num - 1];
        }
        // find right sibling
        if (child_num + 1 < delete_pos->parent->keys_num + 1) {
            right_node = (Leaf_Node*)((Internal_Node*)delete_pos->parent)->children[child_num + 1];
        }
        if (left_node != NULL && left_node->keys_num >= this->min_order) {
            // borrow a key from left sibling
            delete_pos->keys.insert(delete_pos->keys.begin(), left_node->keys.back());
            ((Leaf_Node*)delete_pos)->values.insert(((Leaf_Node*)delete_pos)->values.begin(), left_node->values.back());
            delete_pos->keys_num++;

            // delete the key from left sibling
            left_node->keys.erase(left_node->keys.end() - 1); 
            left_node->values.erase(left_node->values.end() - 1);
            left_node->keys_num--;

            // update parent node
            delete_pos->parent->keys[child_num - 1] = delete_pos->keys.front(); 
            return;
        }
        else if (right_node != NULL && right_node->keys_num >= this->min_order) {
            // borrow a key from right sibling
            delete_pos->keys.push_back(right_node->keys.front());
            ((Leaf_Node*)delete_pos)->values.push_back(right_node->values.front());
            delete_pos->keys_num++;

            // delete the key from right sibling
            right_node->keys.erase(right_node->keys.begin()); 
            right_node->values.erase(right_node->values.begin());
            right_node->keys_num--;

            // update parent node
            delete_pos->parent->keys[child_num] = right_node->keys.front();
            return;
        }
        else {
            if (left_node != NULL && left_node->keys_num < this->min_order) {
                // combine with the left sibling
                left_node->keys.insert(left_node->keys.end(), delete_pos->keys.begin(), delete_pos->keys.end());
                left_node->values.insert(left_node->values.begin(), ((Leaf_Node*)delete_pos)->values.begin(), ((Leaf_Node*)delete_pos)->values.end());

                left_node->next = ((Leaf_Node*)delete_pos)->next;
                if (left_node->next != NULL) {
                    left_node->next->pre = left_node;
                }
                left_node->keys_num += delete_pos->keys_num;

                // delete the key from parent node
                delete_pos->parent->keys.erase(delete_pos->parent->keys.begin() + child_num - 1); 
                ((Internal_Node*)delete_pos->parent)->children.erase(((Internal_Node*)delete_pos->parent)->children.begin() + child_num);
                delete_pos->parent->keys_num--;

                delete delete_pos; 
                delete_pos = left_node;
            }
            else if (right_node != NULL && right_node->keys_num < this->min_order) {
                // combine with the right sibling
                delete_pos->keys.insert(delete_pos->keys.end(), right_node->keys.begin(), right_node->keys.end());
                ((Leaf_Node*)delete_pos)->values.insert(((Leaf_Node*)delete_pos)->values.end(), right_node->values.begin(), right_node->values.end());

                ((Leaf_Node*)delete_pos)->next = right_node->next;
                if (right_node->next != NULL) {
                    right_node->next->pre = ((Leaf_Node*)delete_pos);
                }

                delete_pos->keys_num += right_node->keys_num;
                delete_pos->parent->keys.erase(delete_pos->parent->keys.begin() + child_num);
                delete_pos->parent->keys_num--;
                ((Internal_Node*)delete_pos->parent)->children.erase(((Internal_Node*)delete_pos->parent)->children.begin() + child_num + 1);

                delete right_node;
            }

            // combine the internal node
            Internal_Node* del_inter_pos = (Internal_Node*)delete_pos; 
            while (del_inter_pos->parent != this->bpt_root) { 
                Internal_Node *left_inter_node = NULL;
                Internal_Node *right_inter_node = NULL;
                del_inter_pos = (Internal_Node*)del_inter_pos->parent;

                // the order of parent node is unfilled 
                if (del_inter_pos->keys_num >= this->min_order - 1) { 
                    return;
                }
                int child_inter_num = find_child_num(del_inter_pos);
                
                // find the left sibling
                if (child_inter_num - 1 >= 0) { 
                    left_inter_node = (Internal_Node*)((Internal_Node*)del_inter_pos->parent)->children[child_inter_num - 1];
                }

                // find the right sibling
                if (child_inter_num + 1 < del_inter_pos->parent->keys_num + 1) { 
                    right_inter_node = (Internal_Node*)((Internal_Node*)del_inter_pos->parent)->children[child_inter_num + 1];
                }

                if (right_inter_node != NULL && right_inter_node->keys_num >= this->min_order) {
                    // borrow from right sibling
                    del_inter_pos->keys.push_back(del_inter_pos->parent->keys[child_inter_num]);
                    del_inter_pos->children.push_back(right_inter_node->children.front());
                    // update the parent node
                    del_inter_pos->parent->keys[child_inter_num] = right_inter_node->keys.front(); 

                    del_inter_pos->keys_num++;

                    right_inter_node->children.erase(right_inter_node->children.begin());
                    right_inter_node->keys.erase(right_inter_node->keys.begin());

                    right_inter_node->keys_num--;
                    del_inter_pos->children.back()->parent = del_inter_pos;
                    return;
                }
                else if (left_inter_node != NULL && left_inter_node->keys_num >= this->min_order) {
                    // borrow from left sibling
                    del_inter_pos->children.insert(del_inter_pos->children.begin(), left_inter_node->children.back());
                    del_inter_pos->keys.insert(del_inter_pos->keys.begin(), del_inter_pos->parent->keys[child_inter_num - 1]);
                    // update the parent node
                    del_inter_pos->parent->keys[child_inter_num - 1] = left_inter_node->keys.back(); 

                    del_inter_pos->keys_num++;
                    left_inter_node->children.erase(left_inter_node->children.end() - 1);
                    left_inter_node->keys.erase(left_inter_node->keys.end() - 1);
                    left_inter_node->keys_num--;

                    del_inter_pos->children.front()->parent = del_inter_pos;
                    return;
                }
                else {
                    if (left_inter_node != NULL && left_inter_node->keys_num < this->min_order) {
                        // combine with left sibling
                        left_inter_node->children.insert(left_inter_node->children.end(), del_inter_pos->children.begin(), del_inter_pos->children.end());
                        left_inter_node->keys.insert(left_inter_node->keys.end(), del_inter_pos->parent->keys[child_inter_num - 1]);
                        left_inter_node->keys.insert(left_inter_node->keys.end(), del_inter_pos->keys.begin(), del_inter_pos->keys.end());
                        left_inter_node->keys_num += (del_inter_pos->keys_num + 1);
                        for (int i = 0; i < del_inter_pos->keys_num + 1; i++) {
                            del_inter_pos->children[i]->parent = left_inter_node;
                        }

                        del_inter_pos->parent->keys.erase(del_inter_pos->parent->keys.begin() + child_inter_num - 1);
                        ((Internal_Node*)del_inter_pos->parent)->children.erase(((Internal_Node*)del_inter_pos->parent)->children.begin() + child_inter_num);
                        del_inter_pos->parent->keys_num--;

                        delete del_inter_pos; 
                        del_inter_pos = left_inter_node;
                    }
                    else if (right_inter_node != NULL && right_inter_node->keys_num < this->min_order) {
                        // combine with right sibling
                        right_inter_node->children.insert(right_inter_node->children.begin(), del_inter_pos->children.begin(), del_inter_pos->children.end());
                        right_inter_node->keys.insert(right_inter_node->keys.begin(), del_inter_pos->parent->keys[child_inter_num]);
                        right_inter_node->keys.insert(right_inter_node->keys.begin(), del_inter_pos->keys.begin(), del_inter_pos->keys.end());
                        right_inter_node->keys_num += (del_inter_pos->keys_num + 1);
                        for (int i = 0; i < del_inter_pos->keys_num + 1; i++) {
                            del_inter_pos->children[i]->parent = right_inter_node;
                        }
                        // delete the key from parent node
                        del_inter_pos->parent->keys.erase(del_inter_pos->parent->keys.begin() + child_inter_num);
                        ((Internal_Node*)del_inter_pos->parent)->children.erase(((Internal_Node*)del_inter_pos->parent)->children.begin() + child_inter_num);
                        del_inter_pos->parent->keys_num--;

                        delete del_inter_pos; 
                        del_inter_pos = right_inter_node;
                    }
                }
            }
            if (del_inter_pos->parent == this->bpt_root && this->bpt_root->keys_num == 0) {
                del_inter_pos->parent = NULL;
                this->bpt_root = del_inter_pos;
                return;
            }
        }
    }
}


void BPlusTree::insert_directly(BPT_Node* insert_pos, int key, void* value) {
    // insert the pair into the node pointed by insert_pos
    for (int i = 0; i < insert_pos->keys_num; i++) {
        if (insert_pos->keys[i] > key) {
            insert_pos->keys.insert(insert_pos->keys.begin() + i, key);
            if (insert_pos->is_leaf)
                ((Leaf_Node*)insert_pos)->values.insert(((Leaf_Node*)insert_pos)->values.begin() + i, *(double*)value);
            else
                ((Internal_Node*)insert_pos)->children.insert(((Internal_Node*)insert_pos)->children.begin() + i + 1, (BPT_Node*)value);

            insert_pos->keys_num++;
            return;
        }
        else if (insert_pos->keys[i] == key) {
            cout << "Duplicate Key, fail to insert." << endl;
            return;
        }
    }
    insert_pos->keys.push_back(key);
    if (insert_pos->is_leaf)
        ((Leaf_Node*)insert_pos)->values.push_back(*(double*)value);
    else
        ((Internal_Node*)insert_pos)->children.push_back((BPT_Node*)value);
    insert_pos->keys_num++;
}



void BPlusTree::node_split(BPT_Node* split_pos) {
    // split current node
    BPT_Node* new_node;
    int all_num = split_pos->keys_num;
    int left_num = ceil(all_num / 2.0); // the number of node in left when split from middle
    int right_num = all_num - left_num; // the number of node in right when split from middle
    int middle_key;
    // assign(begin, end): copy from begin to end
    // eraser(begin, end): delete from begin to end 
    
    // split leaf node
    if (split_pos->is_leaf == true) { 
        // insert the key into parent node
        middle_key = split_pos->keys[left_num]; 
        new_node = new Leaf_Node();
    
        Leaf_Node* new_leaf = (Leaf_Node*)new_node;
        Leaf_Node* leaf_split_pos = (Leaf_Node*)split_pos;
        new_leaf->keys_num = right_num;
        new_leaf->is_leaf = true;
        new_leaf->keys.assign(leaf_split_pos->keys.begin() + left_num, leaf_split_pos->keys.end()); // copy the data into new node
        new_leaf->values.assign(leaf_split_pos->values.begin() + left_num, leaf_split_pos->values.end());

        new_leaf->next = leaf_split_pos->next; // insert linked list
        new_leaf->pre = leaf_split_pos;
        if (new_leaf->next != NULL) {
            new_leaf->next->pre = new_leaf;
        }

        new_leaf->parent = leaf_split_pos->parent;

        leaf_split_pos->keys_num = left_num;
        leaf_split_pos->keys.erase(leaf_split_pos->keys.begin() + left_num, leaf_split_pos->keys.end()); // 原节点中删除分裂出去的数据
        leaf_split_pos->values.erase(leaf_split_pos->values.begin() + left_num, leaf_split_pos->values.end());
        leaf_split_pos->next = new_leaf;
    }
    // split internal node
    else {
        // insert the key into parent node
        middle_key = split_pos->keys[left_num - 1]; 
        new_node = new Internal_Node();

        Internal_Node* new_internal = (Internal_Node*)new_node;
        Internal_Node* internal_split_pos = (Internal_Node*)split_pos;
        new_internal->keys_num = right_num;
        new_internal->is_leaf = false;
        // copy the data into new node
        new_internal->keys.assign(internal_split_pos->keys.begin() + left_num, internal_split_pos->keys.end()); 
        new_internal->children.assign(internal_split_pos->children.begin() + left_num, internal_split_pos->children.end());
        for (auto it = internal_split_pos->children.begin() + left_num; it != internal_split_pos->children.end(); it++) {
            (*it)->parent = new_internal;
        }

        new_internal->parent = internal_split_pos->parent;
        internal_split_pos->keys_num = left_num - 1;
        // delete the data from original node
        internal_split_pos->keys.erase(internal_split_pos->keys.begin() + left_num - 1, internal_split_pos->keys.end()); 
        internal_split_pos->children.erase(internal_split_pos->children.begin() + left_num, internal_split_pos->children.end());
    }

    // find the pointer of parent node
    BPT_Node* parent_node = split_pos->parent; 
    // the node is the root
    if (parent_node == NULL) {
        parent_node = new Internal_Node(); 
    
        parent_node->is_leaf = false;
        parent_node->keys_num++;
        // insert the key from children node
        parent_node->keys.push_back(middle_key); 
        // insert the pointer of children node
        ((Internal_Node*)parent_node)->children.push_back(split_pos); 
        ((Internal_Node*)parent_node)->children.push_back(new_node);
        split_pos->parent = parent_node;
        new_node->parent = parent_node;
        // update pointer
        bpt_root = parent_node; 

    }
    else {
        // insert the key into parent node
        insert_directly(parent_node, middle_key, new_node);
        // check whether the parent node is underfilled or not 
        if (parent_node->keys_num > bpt_order - 1) { 
            // split parent node
            node_split(parent_node); 
        }
    }
}


Leaf_Node* BPlusTree::find_node_ptr(int key) {
    // find the pointer of node included this key from root to leaf
    BPT_Node* node_ptr = this->bpt_root; 
    if (this->bpt_root == NULL) {
        return NULL;
    }
    while (!node_ptr->is_leaf) {
        bool flag = false;
        for (int i = 0; i < node_ptr->keys_num; i++) {
            if (node_ptr->keys[i] > key) {
                node_ptr = ((Internal_Node*)node_ptr)->children[i];
                flag = true;
                break;
            }
        }
        if (!flag) {
            node_ptr = ((Internal_Node*)node_ptr)->children[node_ptr->keys_num];
        }
    }
    return (Leaf_Node*)node_ptr;
}

// find the pointer of the value in this key
double* BPlusTree::find_data_ptr(int key) { 
    Leaf_Node* bp = find_node_ptr(key);
    if (bp == NULL)
        return NULL;
    for (int i = 0; i < bp->keys_num; i++) {
        if (bp->keys[i] == key) {
            return &(bp->values[i]);
        }
    }
    return NULL;
}


// find the children node and its siblings
int BPlusTree::find_child_num(BPT_Node* raw_node) { 
    int child_num = -1;
    Internal_Node* parent = (Internal_Node*)raw_node->parent;
    for (int i = 0; i < parent->keys_num + 1; i++) {
        if (parent->children[i] == raw_node) {
            child_num = i;
            return i;
        }
    }
}


// print bplus tree in order to debug
void BPlusTree::print_tree() {
    int level = 1;
    BPT_Node* p = this->bpt_root;
    if (p==NULL){
        cout << "Empty BPlusTree." << endl;
        return;
    }
    queue<BPT_Node*> que;
    que.push(p);
    // NULL is the signal of finishing print one layer
    que.push(NULL); 
    while (!que.empty()) {
        BPT_Node* head = que.front();
        // finish print when NULL == head
        if (head == NULL) { 
            cout << endl; 
            que.pop(); 
            // keep printing if head == internal node
            if (!que.front()->is_leaf) 
                que.push(NULL); 
            continue;
        }
        if (head->is_leaf == false) {
            bool flag = true;
            for (auto it = head->keys.begin(); it != head->keys.end(); it++) {
                flag = false;
                cout << *it << ' ';
            }
            if (flag) {
                cout << head->keys_num << endl;
            }
            // $ is the signal of printing an internal node
            cout << "$ "; 
            for (auto it = ((Internal_Node*)head)->children.begin(); it != ((Internal_Node*)head)->children.end(); it++) {
                que.push(*it);
            }
            que.pop();
        }
        else {
            for (int i = 0; i < head->keys_num; i++) {
                cout << ((Leaf_Node*)head)->keys[i] << ':' << ((Leaf_Node*)head)->values[i] << ' ';
            }
            if (head->keys_num > 0)
                // $ is the signal of printing a leaf node
                cout << "| "; 
            que.pop();
        }
    }
    cout << endl;
}

// print leaf nodes
void BPlusTree::print_leaves() {
    Leaf_Node* p = this->leaves_head;
    while (p != NULL) {
        for (int i = 0; i < p->keys_num; i++) {
            cout << p->keys[i] << ":" << p->values[i] << "  ";
        }
        cout << " | ";
        p = p->next;
    }
    cout << endl;
}

// delete the bplus tree
void BPlusTree::destory_bpt() {
    if (this->bpt_root != NULL) {
        delete this->bpt_root;
    }
}
