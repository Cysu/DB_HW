#include "AEE_indexer.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define ABS(a) ((a) > 0 ? (a) : -(a))

const int DATASET_SIZE = 1000000;
const int BUF_SIZE = 100000;

typedef vector<uint> IdxList;

vector<string> db;
map<string, IdxList> indices;

uint count[DATASET_SIZE];
bool mark[DATASET_SIZE];

char buf[BUF_SIZE];
int cntED[DATASET_SIZE];
int cntJoin[DATASET_SIZE];

uint q;
int gs = 0x7fffffff;
int dbMaxLen = 0, dbMinLen = 0x7fffffff;

int pos, len;


void getGramIndices(const string &qstr, vector<IdxList> &lists)
{
    lists.resize(qstr.length()+1-q);
    for (int k = 0; k < qstr.length()+1-q; ++k) {
        string gram = qstr.substr(k, q);
        lists[k] = indices[gram];
    }
}

void updateProperlistED(const string &delGram, const string &newGram, int T,
                        IdxList &proper)
{
    IdxList &delList = indices[delGram];
    IdxList &newList = indices[newGram];

    for (int i = 0; i < newList.size(); ++i) {
        uint idx = newList[i];
        ++count[idx];
        if (!mark[idx] && count[idx] >=
                T + MAX((int)(db[idx].length()-len), 0)) {
            mark[idx] = true;
            proper.push_back(idx);
        }
    }

    for (int i = 0; i < delList.size(); ++i) --count[delList[i]];

    for (int i = 0; i < proper.size(); ++i) {
        uint idx = proper[i];
        if (count[idx] < T + MAX((int)(db[idx].length()-len), 0)) {
            mark[idx] = false;
            cntED[idx] = -1;
            proper[i] = proper.back();
            proper.pop_back();
            --i;
        }
    }
}

void updateProperlistJaccard(const string &delGram, const string &newGram, int T, double k,
                             IdxList &proper)
{
    int gq = len + 1 - q;

    IdxList &delList = indices[delGram];
    IdxList &newList = indices[newGram];

    for (int i = 0; i < newList.size(); ++i) {
        uint idx = newList[i];
        ++count[idx];
        if (!mark[idx] && count[idx] >=
                MAX(T, (gq+db[idx].length()+1-q) * k)) {
            mark[idx] = true;
            proper.push_back(idx);
        }
    }

    for (int i = 0; i < delList.size(); ++i) --count[delList[i]];

    for (int i = 0; i < proper.size(); ++i) {
        uint idx = proper[i];
        if (count[idx] < MAX(T, (gq+db[idx].length()+1-q) * k)) {
            mark[idx] = false;
            cntJoin[idx] = -1;
            proper[i] = proper.back();
            proper.pop_back();
            --i;
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
        // int l = MAX(1, (int)(i-tau)), r = MIN(m, (int)(i+tau));
        int l = 1, r = m;
        for (int j = l; j <= r; ++j) {
            int w = (s[i-1] == t[j-1]) ? 0 : 1;
            f[i][j] = f[i-1][j-1]+w;
            // if (j > l || 1 > (int)(i-tau)) f[i][j] = MIN(f[i][j], f[i][j-1]+1);
            // if (j < r || m < (int)(i+tau)) f[i][j] = MIN(f[i][j], f[i-1][j]+1);
            f[i][j] = MIN(f[i][j], f[i][j-1]+1);
            f[i][j] = MIN(f[i][j], f[i-1][j]+1);
        }
    }
    return f[n][m];
}

double jaccard(string s, string t, double tau, int &join)
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
    join = combine;
    return (double)combine / (gs+gt-combine);
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
                    T + MAX((int)(db[idx].length()-qstr.length()), 0)) {
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
                    MAX(T, (gq+db[idx].length()+1-q) * k)) {
                mark[idx] = true;
                proper.push_back(idx);
            }
        }
    }
}

void naiveSearchED(const string &qstr, uint tau,
                   vector< quadruple<uint, uint, uint, uint> > &results)
{
    for (int i = 0; i < db.size(); ++i) {
        if (cntED[i] - 2 > (int)(tau)) {
            cntED[i] = -1;
            continue;
        }
        string &s = db[i];
        if (ABS((int)(s.length()-qstr.length())) > tau) continue;
        uint ed = editDistance(s, qstr, tau);
        cntED[i] = (int)(ed);
        if (ed <= tau) {
            quadruple<uint, uint, uint, uint> tmp = {i, pos, len, ed};
            results.push_back(tmp);
        }
    }
}

void naiveSearchJaccard(const string &qstr, double tau,
                        vector< quadruple<uint, uint, uint, double> > &results)
{
    for (int i = 0; i < db.size(); ++i) {
        string &s = db[i];
        int gs = s.length() + 1 - q;
        int gq = qstr.length() + 1 - q;

        int j = cntJoin[i];
        if (j != -1 && (double)(j+2) / (gs+gq-(j+2)) < tau) {
            cntJoin[i] = -1;
            // continue;
        }

        if (MIN(gs, gq) * 1.0 / MAX(gs, gq) < tau) continue;
        double jac = jaccard(s, qstr, tau, cntJoin[i]);
        if (jac >= tau) {
            quadruple<uint, uint, uint, double> tmp = {i, pos, len, jac};
            results.push_back(tmp);
        }
    }
}

void checkValidED(const string &qstr, uint tau, const IdxList &valid,
                  vector< quadruple<uint, uint, uint, uint> > &results)
{
    for (int i = 0; i < valid.size(); ++i) {
        if (cntED[valid[i]] - 2 > (int)(tau)) {
            cntED[valid[i]] = -1;
            continue;
        }
        string &s = db[valid[i]];
        if (ABS((int)(s.length()-qstr.length())) > tau) continue;
        uint ed = editDistance(s, qstr, tau);
        cntED[valid[i]] = (int)(ed);
        if (ed <= tau) {
            quadruple<uint, uint, uint, uint> tmp = {valid[i], pos, len, ed};
            results.push_back(tmp);
        }
    }
}

void checkValidJaccard(const string &qstr, double tau, const IdxList &valid,
                       vector< quadruple<uint, uint, uint, double> > &results)
{
    for (int i = 0; i < valid.size(); ++i) {
        string &s = db[valid[i]];

        int gs = s.length() + 1 - q;
        int gq = qstr.length() + 1 - q;

        int j = cntJoin[valid[i]];
        if (j != -1 && (double)(j+2) / (gs+gq-(j+2)) < tau) {
            cntJoin[valid[i]] = -1;
            // continue;
        }

        if (MIN(gs, gq) * 1.0 / MAX(gs, gq) < tau) continue;
        double jac = jaccard(s, qstr, tau, cntJoin[valid[i]]);
        if (jac >= tau) {
            quadruple<uint, uint, uint, double> tmp = {valid[i], pos, len, jac};
            results.push_back(tmp);
        }
    }
}

bool AEE_Indexer::CreateIndex(const char *dataFilename, uint qgram)
{
    q = qgram;

    FILE *fin = fopen(dataFilename, "r");
    while (fgets(buf, BUF_SIZE, fin) > 0) {
        string tmp = buf;
        int j = tmp.length()-1;
        while (j >= 0 && (tmp[j] == '\r' || tmp[j] == '\n')) --j;
        tmp = tmp.substr(0, j+1);
        db.push_back(tmp);
        gs = MIN(gs, tmp.length() + 1 - q);
        dbMaxLen = MAX(dbMaxLen, tmp.length());
        dbMinLen = MIN(dbMinLen, tmp.length());
    }
    fclose(fin);

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

	return SUCCESS;
}

bool AEE_Indexer::AppEntityExtractED(const char *doc, uint threshold,
    vector< quadruple<uint, uint, uint, uint> > &results)
{
    string sentense(doc);
    uint tau = threshold;
    for (len = MAX(dbMinLen-tau, 0); len <= dbMaxLen+tau; ++len) {
        if (sentense.length() < len) break;
        int T = len - q + 1 - tau * q;

        memset(cntED, -1, sizeof(cntED));

        if (T <= 0) {
            for (pos = 0; pos <= sentense.length()-len; ++pos) {
                string sub = sentense.substr(pos, len);
                naiveSearchED(sub, tau, results);
            }
        } else {
            pos = 0;
            string sub = sentense.substr(0, len);

            vector<IdxList> lists;
            getGramIndices(sub, lists);

            IdxList proper;
            findAppearED(lists, sub, T, proper);

            checkValidED(sub, tau, proper, results);

            for (pos = 1; pos <= sentense.length()-len; ++pos) {
                string sub = sentense.substr(pos, len);
                string delGram = sentense.substr(pos-1, q);
                string newGram = sentense.substr(pos+len-q, q);
                updateProperlistED(delGram, newGram, T, proper);
                checkValidED(sub, tau, proper, results);
            }
        }
    }

    sort(results.begin(), results.end());
	return SUCCESS;
}

bool AEE_Indexer::AppEntityExtractJaccard(const char *doc, double threshold,
    vector<quadruple<uint, uint, uint, double> > &results)
{
    string sentense(doc);
    double tau = threshold;
    int rMin = dbMinLen - q + 1, rMax = dbMaxLen - q + 1;
    for (len = MAX((int)(floor(rMin*tau))-1+q, 0); len <= (int)(ceil(rMax/tau))-1+q; ++len) {
        if (sentense.length() < len) break;
        int gq = len + 1 - q;
        int T = MAX(tau * gq, (gq+gs) * tau / (1+tau));

        memset(cntJoin, -1, sizeof(cntJoin));

        if (T <= 0) {
            for (pos = 0; pos <= sentense.length()-len; ++pos) {
                string sub = sentense.substr(pos, len);
                naiveSearchJaccard(sub, tau, results);
            }
        } else {
            pos = 0;
            string sub = sentense.substr(0, len);

            vector<IdxList> lists;
            getGramIndices(sub, lists);

            IdxList proper;
            findAppearJaccard(lists, sub, T, tau / (1+tau), proper);

            checkValidJaccard(sub, tau, proper, results);

            for (pos = 1; pos <= sentense.length()-len; ++pos) {
                string sub = sentense.substr(pos, len);
                string delGram = sentense.substr(pos-1, q);
                string newGram = sentense.substr(pos+len-q, q);
                updateProperlistJaccard(delGram, newGram, T, tau / (1+tau), proper);
                checkValidJaccard(sub, tau, proper, results);
            }
        }
    }

    sort(results.begin(), results.end());
	return SUCCESS;
}
