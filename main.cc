#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "BPlusTree.h"

//#include "BPlusTree.h"

using namespace std;

int main(int argc, char** argv) {
	ifstream readFile(argv[1]);                // open input file
	ofstream writeFile("output_file.txt");     // open output file
	string i;                                  
	int order = 0;                             

	//get orders
    while (order == 0) {
		if (readFile) {
			getline(readFile, i);
			int index = 0;

			if (!i.empty())
			{
				while ((index = i.find(' ', index)) != string::npos)
				{
					i.erase(index, 1);
				}
			}
            
            // find init command
			if (i.find("Initialize") != string::npos)
			{
				int leftParen = i.find_first_of("(", 0);
				int rightParen = i.find_first_of(")", 0);
				order = stoi(i.substr(leftParen + 1, rightParen - leftParen - 1));
			}
		}
	}

    // init
	BPlusTree t(order);

	if (readFile){
		while (getline(readFile, i)) {
			int index = 0;
            
			if (!i.empty()){
				while ((index = i.find(' ', index)) != string::npos){
					i.erase(index, 1);
				}
			}

            
            // insert
			if (i.find("Insert") != string::npos) {
				int leftParen = i.find_first_of("(", 0);
				int rightParen = i.find_first_of(")", 0);
				int comma = i.find_first_of(",", 0);
				int key = stoi(i.substr(leftParen + 1, comma - leftParen - 1));
				double value = stof(i.substr(comma + 1, rightParen - comma - 1));
				t.Insert(key, value);
			}
            // delete
			else if (i.find("Delete") != string::npos)
			{
				int leftParen = i.find_first_of("(", 0);
				int rightParen = i.find_first_of(")", 0);
				int key = stoi(i.substr(leftParen + 1, rightParen - leftParen - 1));
				t.Delete(key);
			}
            // search
			else if (i.find("Search") != string::npos)
			{
				// search range
                int m = i.find(",");
				if (m > i.size()) {
					m = 0;
				}
				if (m) {
					// search range
					int leftParen = i.find_first_of("(", 0);
					int rightParen = i.find_first_of(")", 0);
					int comma = i.find_first_of(",", 0);
					int key1 = stoi(i.substr(leftParen + 1, comma - leftParen - 1));
					int key2 = stoi(i.substr(comma + 1, rightParen - comma - 1));
					vector<double> list = t.Search(key1, key2);
                    // search results
					if(list.size()==0){
						writeFile << "Null" << endl;
					}
					else {
						int i = 0;
						for (i; i < list.size() - 1; i++) {
							writeFile << list[i] << ",";
						}
						writeFile << list[i] << endl;
					}
				}
				else {
                    // search
					int leftParen = i.find_first_of("(", 0);
					int rightParen = i.find_first_of(")", 0);
					int key = stoi(i.substr(leftParen + 1, rightParen - leftParen - 1));
					double result = t.Search(key);
                   
					if (result != 0) {
						writeFile << result << endl;
					}
					else {
						writeFile << "Null" << endl;
					}
				}
			}
			else {
				cout << "Invalid command." << endl;
				return 1;
			}
		}
	}
	else {
		cout << "Read error." << endl;
		return 1;
	}
    
    // close files
    writeFile.close();
    readFile.close();

	return 0;
}
