#include "simJoin.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define ABS(a) ((a) > 0 ? (a) : -(a))

const int DATASET_SIZE = 1000000;

typedef vector<uint> IdxList;

vector<string> db_1, db_2;
vector<string> *db, *query;
map<string, IdxList> indices;

uint count[DATASET_SIZE];
bool mark[DATASET_SIZE];

uint q;
int gs = 0x7fffffff;

void readData(const char *filename, vector<string> &db)
{
    FILE *fin = fopen(filename, "r");
    char buf[BUFSIZ];
    while (fgets(buf, BUFSIZ, fin) > 0) {
        string tmp = buf;
        int j = tmp.length()-1;
        while (j >= 0 && (tmp[j] == '\r' || tmp[j] == '\n')) --j;
        tmp = tmp.substr(0, j+1);
        db.push_back(tmp);
        gs = MIN(gs, tmp.length() + 1 - q);
    }
    fclose(fin);
}

void createIndex()
{
    for (int i = 0; i < db->size(); ++i) {
        set<string> contain;
        string &tmp = db->at(i);
        for (int k = 0; k < tmp.length()+1-q; ++k) {
            string sub = tmp.substr(k, q);
            if (contain.find(sub) != contain.end()) continue;
            contain.insert(sub);
            indices[sub].push_back(i);
        }
    }
}

void getSortedGram(const string &qstr, vector<string> &gram)
{
    for (int k = 0; k < qstr.length()+1-q; ++k) {
        string sub = qstr.substr(k, q);
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

uint editDistance(string s, string t, uint tau)
{
    if (s.length() > t.length()) std::swap(s, t);
    int n = s.length(), m = t.length();
    uint f[n+1][m+1];
    memset(f, 0, sizeof(f));
    for (int i = 0; i <= n; ++i) f[i][0] = i;
    for (int j = 0; j <= m; ++j) f[0][j] = j;
    for (int i = 1; i <= n; ++i) {
        int l = MAX(1, (int)(i-tau)), r = MIN(m, (int)(i+tau));
        for (int j = l; j <= r; ++j) {
            int w = (s[i-1] == t[j-1]) ? 0 : 1;
            f[i][j] = f[i-1][j-1]+w;
            if (j > l || 1 > (int)(i-tau)) f[i][j] = MIN(f[i][j], f[i][j-1]+1);
            if (j < r || m < (int)(i+tau)) f[i][j] = MIN(f[i][j], f[i-1][j]+1);
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

void naiveSearchED(int qIdx, uint tau,
                   vector< triple<uint, uint, uint> > &results)
{
    string qstr = query->at(qIdx);
    for (int i = 0; i < db->size(); ++i) {
        string &s = db->at(i);
        if (ABS((int)(s.length()-qstr.length())) > tau) continue;
        uint ed = editDistance(s, qstr, tau);
        if (ed <= tau) {
            triple<uint, uint, uint> tmp = {qIdx, i, ed};
            results.push_back(tmp);
        }
    }
}

void naiveSearchJaccard(int qIdx, double tau,
                        vector< triple<uint, uint, double> > &results)
{
    string qstr = query->at(qIdx);
    for (int i = 0; i < db->size(); ++i) {
        string &s = db->at(i);
        int gs = s.length() + 1 - q;
        int gq = qstr.length() + 1 - q;
        if (MIN(gs, gq) * 1.0 / MAX(gs, gq) < tau) continue;
        double jac = jaccard(s, qstr, tau);
        if (jac >= tau) {
            triple<uint, uint, double> tmp = {qIdx, i, jac};
            results.push_back(tmp);
        }
    }
}


void findAppearED(const vector<IdxList> &lists, const string &qstr,
                  int T, IdxList &proper)
{
    memset(count, 0, sizeof(count));
    memset(mark, false, sizeof(mark));

    for (int i = 0; i < lists.size(); ++i) {
        for (int j = 0; j < lists[i].size(); ++j) {
            uint idx = lists[i].at(j);
            ++count[idx];
            if (!mark[idx] && count[idx] >=
                    T + MAX((int)(db->at(idx).length()-qstr.length()), 0)) {
                mark[idx] = true;
                proper.push_back(idx);
            }
        }
    }
}

void findAppearJaccard(const vector<IdxList> &lists, const string &qstr,
                       int T, double k, IdxList &proper)
{
    int gq = qstr.length() + 1 - q;

    memset(count, 0, sizeof(count));
    memset(mark, false, sizeof(mark));

    for (int i = 0; i < lists.size(); ++i) {
        for (int j = 0; j < lists[i].size(); ++j) {
            uint idx = lists[i].at(j);
            ++count[idx];
            if (!mark[idx] && count[idx] >=
                    MAX(T, (gq+db->at(idx).length()+1-q) * k)) {
                mark[idx] = true;
                proper.push_back(idx);
            }
        }
    }
}


void checkValidED(int qIdx, uint tau, const IdxList &valid,
                  vector< triple<uint, uint, uint> > &results)
{
    string qstr = query->at(qIdx);
    for (int i = 0; i < valid.size(); ++i) {
        string &s = db->at(valid[i]);
        if (ABS((int)(s.length()-qstr.length())) > tau) continue;
        uint ed = editDistance(s, qstr, tau);
        if (ed <= tau) {
            triple<uint, uint, uint> tmp = {qIdx, valid[i], ed};
            results.push_back(tmp);
        }
    }
}

void checkValidJaccard(int qIdx, double tau, const IdxList &valid,
                       vector< triple<uint, uint, double> > &results)
{
    string qstr = query->at(qIdx);
    for (int i = 0; i < valid.size(); ++i) {
        string &s = db->at(valid[i]);
        int gs = s.length() + 1 - q;
        int gq = qstr.length() + 1 - q;
        if (MIN(gs, gq) * 1.0 / MAX(gs, gq) < tau) continue;
        double jac = jaccard(s, qstr, tau);
        if (jac >= tau) {
            triple<uint, uint, double> tmp = {qIdx, valid[i], jac};
            results.push_back(tmp);
        }
    }
}

void searchED(int qIdx, uint tau, vector< triple<uint, uint, uint> > &results)
{
    string qstr = query->at(qIdx);
    int T = qstr.length() - q + 1 - tau * q;

    if (T <= 0) {
        naiveSearchED(qIdx, tau, results);
        return;
    }

    // Get sorted grams.
    vector<string> gram;
    getSortedGram(qstr, gram);

    // Find indices appear more than T times in lists.
    vector<IdxList> lists;
    for (int i = 0; i < gram.size(); ++i) lists.push_back(indices[gram[i]]);

    IdxList proper;
    findAppearED(lists, qstr, T, proper);

    checkValidED(qIdx, tau, proper, results);
}

void searchJaccard(int qIdx, double tau,
                   vector< triple<uint, uint, double> > &results)
{
    string qstr = query->at(qIdx);
    int gq = qstr.length() + 1 - q;
    int T = MAX(tau * gq, (gq+gs) * tau / (1+tau));

    if (T <= 0) {
        naiveSearchJaccard(qIdx, tau, results);
        return;
    }

    // Get sorted grams.
    vector<string> gram;
    getSortedGram(qstr, gram);

    // Find indices appear more than T times in lists.
    vector<IdxList> lists;
    for (int i = 0; i < gram.size(); ++i) lists.push_back(indices[gram[i]]);

    IdxList proper;
    findAppearJaccard(lists, qstr, T, tau / (1+tau), proper);

    checkValidJaccard(qIdx, tau, proper, results);
}

 
bool simJoin::SimilarityJoinED(const char *firstDataFilename,
                               const char *secondDataFilename,
                               uint qgram, uint threshold,
                               vector< triple<uint, uint, uint> > &results)
{
    q = qgram;

    readData(firstDataFilename, db_1);
    readData(secondDataFilename, db_2);

    if (db_1.size() > db_2.size()) {
        db = &db_1;
        query = &db_2;
    } else {
        db = &db_2;
        query = &db_1;
    }

    createIndex();

    for (int i = 0; i < query->size(); ++i)
        searchED(i, threshold, results);

    if (db_1.size() > db_2.size()) {
        for (int i = 0; i < results.size(); ++i)
            std::swap(results[i].id1, results[i].id2);
    }
    sort(results.begin(), results.end());

    editDistance("arai", "hara", 2);

	return SUCCESS;
}


bool simJoin::SimilarityJoinJaccard(const char *firstDataFilename,
                                    const char *secondDataFilename,
                                    uint qgram, double threshold,
                                    vector< triple<uint, uint, double> > &results)
{
    q = qgram;

	readData(firstDataFilename, db_1);
    readData(secondDataFilename, db_2);

    if (db_1.size() > db_2.size()) {
        db = &db_1;
        query = &db_2;
    } else {
        db = &db_2;
        query = &db_1;
    }

    createIndex();

    for (int i = 0; i < query->size(); ++i)
        searchJaccard(i, threshold, results);

    if (db_1.size() > db_2.size()) {
        for (int i = 0; i < results.size(); ++i)
            std::swap(results[i].id1, results[i].id2);
    }
    sort(results.begin(), results.end());

    return SUCCESS;
}
