/* 
 * The FZ_Indexer class handles the creation, deletion, and loading of an index. All 
 * necessary initialization of the indexer component should take place within the 
 * constructor for the FZ_Indexer class. 
 * */

#pragma once
#include <vector>
#include <utility>
#include <string>
#include <map>
#include <set>

using std::string;
using std::vector;
using std::pair;
using std::map;
using std::set;

const int SUCCESS = 1;
const int FAILURE = 0; 

class FZ_Indexer {
public:
	FZ_Indexer   () {};
	~FZ_Indexer  () {};

	bool CreateIndex(const char * dataFilename, unsigned q, const char * indexFilename); // create an index and store it in a file 

	bool DestroyIndex(); // destroy the index file on disk

	bool LoadIndex(); // Load the index from disk if it's not in memory

	bool SearchED(const char *query, unsigned threshold, vector< pair<unsigned, unsigned> > &results);

	bool SearchJaccard(const char *query, double threshold, vector< pair<unsigned, double> > &results);

private:
}; 
