#include <ctime>
#include <cstdio>
#include <cstring>
#include "AEE_indexer.h"

#define TIME_COUNT
#ifdef TIME_COUNT
    #define REC_TIME_START(t) \
        double t = clock() / (double)CLK_TCK;
    #define REC_TIME_END(tc, t) \
        tc += clock() / (double)CLK_TCK - t;
#else
    #define REC_TIME_START(t)
    #define REC_TIME_END(tc, t)
#endif

#define CHECK_ED
// #define CHECK_JACCARD

typedef unsigned int uint;

string toLower(const string &s)
{
    string ret = s;
    for (int i = 0; i < ret.length(); ++i) {
        if ('A' <= ret[i] && ret[i] <= 'Z')
            ret[i] = ret[i] - 'A' + 'a';
    }
    return ret;
}

int main()
{
#ifdef TIME_COUNT
    double timeCount = 0.0;
#endif

    char entityFilename[] = "entity";
    char documentFilename[] = "doc";

    uint q = 2;

    AEE_Indexer aee;
    aee.CreateIndex(entityFilename, q);

    // Read doc file
    char buf[BUFSIZ];
    vector<string> docs;
    FILE *fin = fopen(documentFilename, "r");
    while (fgets(buf, BUFSIZ, fin) > 0) {
        string tmp = buf;
        int j = tmp.length() - 1;
        while (j >= 0 && (tmp[j] == '\r' || tmp[j] == '\n')) --j;
        tmp = tmp.substr(0, j+1);
        docs.push_back(toLower(tmp));
    }
    fclose(fin);

#if defined(CHECK_ED)
    uint tau = 4;

    REC_TIME_START(t1);
    for (int i = 0; i < 10; ++i) {
        vector< quadruple<uint, uint, uint, uint > > results;
        aee.AppEntityExtractED(docs[i].c_str(), tau, results);
        printf("#%d, tot = %d\n", i, results.size());
        // for (int i = 0; i < results.size(); ++i) {
        //     printf("id = %u, pos = %u, len = %u, sim = %u\n",
        //         results[i].id, results[i].pos, results[i].len, results[i].sim);
        // }
    }
    REC_TIME_END(timeCount, t1);
#elif defined(CHECK_JACCARD)
    double tau = 0.5;

    REC_TIME_START(t1);
    for (int i = 0; i < docs.size(); ++i) {
        vector< quadruple<uint, uint, uint, double > > results;
        aee.AppEntityExtractJaccard(docs[i].c_str(), tau, results);
        printf("#%d, tot = %d\n", i, results.size());
        /*for (int i = 0; i < results.size(); ++i) {
            printf("id = %u, pos = %u, len = %u, sim = %lf\n",
                results[i].id, results[i].pos, results[i].len, results[i].sim);
        }*/
    }
    REC_TIME_END(timeCount, t1);
#endif

    #ifdef TIME_COUNT
    printf("time = %lf\n", timeCount / 10);
#endif
    
	return 0;
}
