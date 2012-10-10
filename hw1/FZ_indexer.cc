#include "FZ_indexer.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define ABS(a) ((a) > 0 ? (a) : -(a))

// #define NAIVE
#define SCAN_COUNT
// #define HEAP

const int DATASET_SIZE = 1000000;

typedef unsigned int uint;
typedef vector<uint> IdxList;

vector<string> db;
map<string, IdxList> indices;

uint count[DATASET_SIZE];
bool mark[DATASET_SIZE];

uint q;
int gs = 0x7fffffff;

void swap(string &s, string &t)
{
    string tmp = s;
    s = t;
    t = tmp;
}

void swap(uint &a, uint &b)
{
    uint t = a;
    a = b;
    b = t;
}

void heapAdjustUp(uint *heap, uint *listIds, uint i)
{
    while (i > 0) {
        uint j = (i-1)/2;
        if (heap[i] >= heap[j]) break;
        swap(heap[i], heap[j]);
        swap(listIds[i], listIds[j]);
        i = j;
    }
}

void heapAdjustDown(uint *heap, uint *listIds, uint heapSize, uint i)
{
    while (i*2+1 < heapSize) {
        uint j = i*2+1;
        if (j+1 < heapSize && heap[j+1] < heap[j]) ++j;
        if (heap[i] <= heap[j]) break;
        swap(heap[i], heap[j]);
        swap(listIds[i], listIds[j]);
        i = j;
    }
}

void heapPush(uint *heap, uint *listIds, uint &heapSize, uint idx, uint listId)
{
    heap[heapSize] = idx;
    listIds[heapSize] = listId;
    ++heapSize;
    heapAdjustUp(heap, listIds, heapSize-1);
}

bool heapPop(uint *heap, uint *listIds, uint &heapSize)
{
    if (heapSize <= 0) return false;
    --heapSize;
    swap(heap[heapSize], heap[0]);
    swap(listIds[heapSize], listIds[0]);
    heapAdjustDown(heap, listIds, heapSize, 0);
    return true;
}

uint editDistance(string s, string t, uint tau)
{
    if (s.length() > t.length()) swap(s, t);
    int n = s.length(), m = t.length();
    uint f[n+1][m+1];
    memset(f, 0, sizeof(f));
    for (int i = 0; i <= n; ++i) f[i][0] = i;
    for (int j = 0; j <= m; ++j) f[0][j] = j;
    for (int i = 1; i <= n; ++i) {
        int l = MAX(1, i-tau), r = MIN(m, i+tau);
        for (int j = l; j <= r; ++j) {
            int w = (s[i-1] == t[j-1]) ? 0 : 1;
            f[i][j] = f[i-1][j-1]+w;
            if (j > l) f[i][j] = MIN(f[i][j], f[i][j-1]+1);
            if (j < r) f[i][j] = MIN(f[i][j], f[i-1][j]+1);
        }
    }
    return f[n][m];
}

double jaccard(string s, string t, double tau)
{
    map<string, int> gramS;
    int gs = s.length()+1-q, gt = t.length()+1-q;
    int combine = 0;
    for (int i = 0; i < gs; ++i) {
        string sub = s.substr(i, q);
        ++gramS[sub];
    }
    for (int i = 0; i < gt; ++i) {
        string sub = t.substr(i, q);
        if (gramS[sub] > 0) {
            --gramS[sub];
            ++combine;
        }
    }
    return (double)combine / (gs+gt-combine);
}

// Use MergeSkip to find the indices that appears >= @T times in @lists.
void findAppearHeap(const vector<IdxList> &lists, int T,
    vector< pair<uint, uint> > &ret)
{
    uint heap[lists.size()];
    uint listIds[lists.size()];
    uint cntPtr[lists.size()];
    uint heapSize = 0;
    memset(cntPtr, 0, sizeof(cntPtr));

    for (int i = 0; i < lists.size(); ++i) {
        if (!lists[i].empty()) {
            heapPush(heap, listIds, heapSize, lists[i].at(0), i);
            cntPtr[i] = 1;
        }
    }

    while (heapSize > 0) {
        vector<uint> popedList;
        uint t = heap[0];
        while (heapSize > 0 && heap[0] == t) {
            popedList.push_back(listIds[0]);
            heapPop(heap, listIds, heapSize);
        }

        int n = popedList.size();
        if (n >= T) {
            pair<uint, uint> tmp(t, n);
            ret.push_back(tmp);
            for (int i = 0; i < n; ++i) {
                uint listId = popedList[i];
                if (cntPtr[listId] < lists[listId].size()) {
                    heapPush(heap, listIds, heapSize,
                        lists[listId].at(cntPtr[listId]++), listId);
                }
            }
        } else {
            for (int i = 0; i < T-1-n; ++i) {
                popedList.push_back(listIds[0]);
                if (!heapPop(heap, listIds, heapSize)) break;
            }

            if (heapSize <= 0) break;

            uint t = heap[0];
            for (int i = 0; i < popedList.size(); ++i) {
                uint listId = popedList[i];
                uint &ptr = cntPtr[listId];
                --ptr;
                while (ptr < lists[listId].size() &&
                    lists[listId].at(ptr) < t) ++ptr;
                if (ptr < lists[listId].size())
                    heapPush(heap, listIds, heapSize,
                        lists[listId].at(ptr++), listId);
            }
        }
    }
}

void findAppearScanCount(const vector<IdxList> &lists, int T, IdxList &proper)
{
    memset(count, 0, sizeof(count));
    memset(mark, false, sizeof(mark));

    for (int i = 0; i < lists.size(); ++i) {
        for (int j = 0; j < lists[i].size(); ++j) {
            uint idx = lists[i].at(j);
            ++count[idx];
            if (count[idx] >= T && !mark[idx]) {
                mark[idx] = true;
                proper.push_back(idx);
            }
        }
    }
}

// Use binary search to find whether the index is in the list.
bool inList(const IdxList &list, int head, int idx)
{
    int i = head, j = list.size()-1;
    while (i <= j) {
        int k = (i+j) / 2;
        uint x = list[k];
        if (x == idx) return true;
        if (idx < x) j = k-1;
        else i = k+1;
    }
    return false;
}

// Find the @q grams belong to @qstr. Sort it from long to short.
void getSortedGram(const string &qstr, vector<string> &gram)
{
    set<string> gramSet;
    for (int k = 0; k < qstr.length()+1-q; ++k) {
        string sub = qstr.substr(k, q);
        if (gramSet.find(sub) != gramSet.end()) continue;
        gramSet.insert(sub);
        gram.push_back(sub);
    }

    for (int i = 0; i < gram.size(); ++i)
        for (int j = i+1; j < gram.size(); ++j) {
            IdxList &x = indices[gram[i]];
            IdxList &y = indices[gram[j]];
            if (x.size() < y.size()) {
                string t = gram[i];
                gram[i] = gram[j];
                gram[j] = t;
            }
        }
}

void naiveSearchED(const string &qstr, uint tau,
    vector< pair<uint, uint> > &results)
{
    for (int i = 0; i < db.size(); ++i) {
        string &s = db[i];
        if (ABS((int)(s.length()-qstr.length())) > tau) continue;
        uint ed = editDistance(s, qstr, tau);
        if (ed <= tau) {
            pair<uint, uint> tmp(i, ed);
            results.push_back(tmp);
        }
    }
}

void naiveSearchJaccard(const string &qstr, double tau,
    vector< pair<uint, double> > &results)
{
    for (int i = 0; i < db.size(); ++i) {
        string &s = db[i];
        int gs = s.length() + 1 - q;
        int gq = qstr.length() + 1 - q;
        if (MIN(gs, gq) * 1.0 / MAX(gs, gq) < tau) continue;
        double jac = jaccard(s, qstr, tau);
        if (jac >= tau) {
            pair<uint, double> tmp(i, jac);
            results.push_back(tmp);
        }
    }
}

void checkValidED(const string &qstr, uint tau, const IdxList &valid,
    vector< pair<uint, uint> > &results)
{
    for (int i = 0; i < valid.size(); ++i) {
        string &s = db[valid[i]];
        if (ABS((int)(s.length()-qstr.length())) > tau) continue;
        uint ed = editDistance(s, qstr, tau);
        if (ed <= tau) {
            pair<uint, uint> tmp(valid[i], ed);
            results.push_back(tmp);
        }
    }
}

void checkValidJaccard(const string &qstr, double tau, const IdxList &valid,
    vector< pair<uint, double> > &results)
{
    for (int i = 0; i < valid.size(); ++i) {
        string &s = db[valid[i]];
        int gs = s.length() + 1 - q;
        int gq = qstr.length() + 1 - q;
        if (MIN(gs, gq) * 1.0 / MAX(gs, gq) < tau) continue;
        double jac = jaccard(s, qstr, tau);
        if (jac >= tau) {
            pair<uint, double> tmp(valid[i], jac);
            results.push_back(tmp);
        }
    }
}


/*
 * This method creates an index on the strings stored in the given data file name. 
 * The format of the file is as follows: 
	* each line represents a string. 
	* The ID of each string is its line number, starting from 0. 
 * The index is created and  serialized to a file on the disk, and the file name is indexFilename. 
 */
bool FZ_Indexer::CreateIndex(const char * dataFilename, uint qgram, const char * indexFilename) {
    // Save database strings to @db.
    FILE *fin = fopen(dataFilename, "r");
    char buf[BUFSIZ];
    while (fgets(buf, BUFSIZ, fin) > 0) {
        string tmp = buf;
        int j = tmp.length()-1;
        while (j >= 0 && (tmp[j] == '\r' || tmp[j] == '\n')) --j;
        tmp = tmp.substr(0, j+1);
        db.push_back(tmp);
        gs = MIN(gs, tmp.length() + 1 - qgram);
    }
    fclose(fin);

    q = qgram;

    // Create index file if needed.
#if defined(SCAN_COUNT) || defined(HEAP)
    for (int i = 0; i < db.size(); ++i) {
        set<string> contain;
        string &tmp = db[i];
        for (int k = 0; k < tmp.length()+1-q; ++k) {
            string sub = tmp.substr(k, q);
            if (contain.find(sub) != contain.end()) continue;
            contain.insert(sub);
            indices[sub].push_back(i);
        }
    }
    fclose(fin);
#endif

    return SUCCESS;
}


/*
 * This method should destroy the index and delete the correspond index file on
 * disk (if any).
 */

bool FZ_Indexer::DestroyIndex() {
	return SUCCESS;
}

/*
 * This method should load the index from the disk into memory. If it's not in memory. 
 * Return an error if the index has not been constructed. 
 */

bool FZ_Indexer::LoadIndex() {
	return SUCCESS;
}

/*
 * It should do a search using the index by finding all the strings in the data
 * file whose edit distance to the query string is within the threshold. The 
 * format of result is a pair of integers which respectively stand for the 
 * qualified string ID and the edit distance between the qualified string and 
 * query string. All results are stored in a vector, sorted based on the 
 * qualified string IDs in an ascending order. Return an error if the index is 
 * not constructed or not loaded in memory.
 */

bool FZ_Indexer::SearchED(const char *query, uint threshold, vector< pair<uint, uint> > &results) {
#if defined (NAIVE)
    naiveSearchED(string(query), threshold, results);
#elif defined (SCAN_COUNT) || defined(HEAP)
    string qstr = query;
    uint tau = threshold;
    int T = qstr.length() - q + 1 - tau * q;
    int L = 0;  // Should be in [0..T-1]

    // printf("T = %d\n", T);

    if (T <= 0) {
        naiveSearchED(query, tau, results);
        return SUCCESS;
    }

    // Get sorted grams.
    vector<string> gram;
    getSortedGram(qstr, gram);

    // Find indices appear more than T-L times in shorter lists.
    vector<IdxList> lists;
    for (int i = L; i < gram.size(); ++i) lists.push_back(indices[gram[i]]);

#if defined(SCAN_COUNT)
    IdxList proper;
    findAppearScanCount(lists, T-L, proper);
#elif defined(HEAP)
    vector< pair<uint, uint> > proper;
    findAppearHeap(lists, T-L, proper);
#endif

    // Use binary search in longer lists to filter out the idx in proper list.
    IdxList valid;
    for (int i = 0; i < proper.size(); ++i) {
#if defined(SCAN_COUNT)
        uint idx = proper[i];
        uint n = count[idx];
#elif defined(HEAP)
        uint idx = proper[i].first;
        uint n = proper[i].second;
#endif
        // Optimize thx to xch.
        if (n >= T + MAX((int)(db[idx].length()-qstr.length()), 0)) {
            valid.push_back(idx);
            continue;
        }

        for (int j = 0; j < L; ++j) {
            IdxList &list = indices[gram[j]];
            if (inList(list, 0, idx)) {
                if (++n >= T + MAX((int)(db[idx].length()-qstr.length()), 0)) {
                    valid.push_back(idx);
                    break;
                }
            }
        }
    }

    checkValidED(qstr, tau, valid, results);
#endif
    return SUCCESS;
}


/*
 * It should do a search using the index by finding all the strings in the data
 * file whose jaccard similarity to the query string is not smaller than the threshold. The 
 * format of result is a pair of number which respectively stand for the 
 * qualified string ID and the jaccard similarity between the qualified string and 
 * query string. All results are stored in a vector, sorted based on the 
 * qualified string IDs in an ascending order. Return an error if the index is 
 * not constructed or not loaded in memory.
 */

bool FZ_Indexer::SearchJaccard(const char *query, double threshold, vector< pair<uint, double> > &results) {
#if defined (NAIVE)
    naiveSearchJaccard(string(query), threshold, results);
#elif defined (SCAN_COUNT) || defined (HEAP)
    string qstr = query;
    double tau = threshold;
    int gq = qstr.length() + 1 - q;
    int T = MAX(tau * gq, (gq+gs) * tau / (1+tau));
    int L = 0;  // Should be in [0..T-1]

    // printf("T = %d\n", T);

    if (T <= 0) {
        naiveSearchJaccard(query, tau, results);
        return SUCCESS;
    }

    // Get sorted grams.
    vector<string> gram;
    getSortedGram(qstr, gram);

    // Find indices appear more than T-L times in shorter lists.
    vector<IdxList> lists;
    for (int i = L; i < gram.size(); ++i) lists.push_back(indices[gram[i]]);

#if defined(SCAN_COUNT)
    IdxList proper;
    findAppearScanCount(lists, T-L, proper);
#elif defined(HEAP)
    vector< pair<uint, uint> > proper;
    findAppearHeap(lists, T-L, proper);
#endif

    // Use binary search in longer lists to filter out the idx in proper list.
    IdxList valid;
    for (int i = 0; i < proper.size(); ++i) {
#if defined(SCAN_COUNT)
        uint idx = proper[i];
        uint n = count[idx];
#elif defined(HEAP)
        uint idx = proper[i].first;
        uint n = proper[i].second;
#endif
        int newT = MAX(T, (gq+db[idx].length()+1-q) * tau / (1+tau));
        // Optimize thx to xch.
        if (n >= newT) {
            valid.push_back(idx);
            continue;
        }

        for (int j = 0; j < L; ++j) {
            IdxList &list = indices[gram[j]];
            if (inList(list, 0, idx)) {
                if (++n >= newT) {
                    valid.push_back(idx);
                    break;
                }
            }
        }
    }

    checkValidJaccard(qstr, tau, valid, results);
#endif
    return SUCCESS;
}
