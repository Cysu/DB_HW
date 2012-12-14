#pragma once
#include <vector>
#include <utility>
#include <string>
#include <map>
#include <set>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>

using std::string;
using std::vector;
using std::pair;
using std::map;
using std::set;
using std::sort;

typedef unsigned int uint;

const int SUCCESS = 1;
const int FAILURE = 0; 

template<typename T1, typename T2, typename T3, typename T4>
struct quadruple 
{
    T1 id;
    T2 pos;
    T3 len;
    T4 sim;

    friend bool operator < (const quadruple &a, const quadruple &b) {
        if (a.id < b.id) return true;
        if (a.id > b.id) return false;
        if (a.pos < b.pos) return true;
        if (a.pos > b.pos) return false;
        return a.len <= b.len;
    }
};

class AEE_Indexer
{
public:
	AEE_Indexer() {};
	~AEE_Indexer() {};

	bool CreateIndex(const char *dataFilename, uint q);

	bool AppEntityExtractED(const char *doc, uint threshold,
        vector< quadruple<uint, uint, uint, uint> > &results);

	bool AppEntityExtractJaccard(const char *doc, double threshold,
        vector<quadruple<uint, uint, uint, double> > &results);
}; 
