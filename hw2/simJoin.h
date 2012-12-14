/* 
* The simJoin class handles the creation, deletion, and loading of an index. 
* */

#pragma once
#include <vector>
#include <utility>
#include <string>
#include <map>
#include <set>
#include <algorithm>

using std::string;
using std::vector;
using std::pair;
using std::map;
using std::set;
using std::sort;

typedef unsigned int uint;

const int SUCCESS = 1;
const int FAILURE = 0; 

template<typename T1, typename T2, typename T3>
struct triple
{
    T1 id1;
    T2 id2;
    T3 sim;

    friend bool operator < (const triple<T1, T2, T3> &a, 
                            const triple<T1, T2, T3> &b)
    {
        return (a.id1 < b.id1) || (a.id1 == b.id1 && a.id2 < b.id2);
    }
};

class simJoin
{
public:
    simJoin() {};
    ~simJoin() {};

    bool SimilarityJoinED(const char *firstDataFilename,
                          const char *secondDataFilename,
                          uint q, uint threshold,
                          vector< triple<uint, uint, uint> > &results);

    bool SimilarityJoinJaccard(const char *firstDataFilename,
                               const char *secondDataFilename,
                               uint q, double threshold,
                               vector< triple<uint, uint, double> > &results);

private:
    char *firstDataFilename;
    char *secondDataFilename;
}; 
