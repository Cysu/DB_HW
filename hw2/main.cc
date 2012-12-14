#include <cstdio>
#include <cstring>
#include <ctime>
#include "simJoin.h"

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

// #define CHECK_ED
#define CHECK_JACCARD

typedef unsigned int uint;

int main(int argc, char *argv[])
{
    char firstDataFilename[] = "query.data";
    char secondDataFilename[] = "author.data";

#ifdef TIME_COUNT
    double timeCount = 0.0;
#endif

    simJoin joiner;

    // Set parameters.
#if defined(CHECK_ED)
    uint q = 3;
    uint tau = 3;

    vector< triple<uint, uint, uint > > results;

    REC_TIME_START(t1);
    joiner.SimilarityJoinED(firstDataFilename,
                            secondDataFilename,
                            q, tau,
                            results);
    REC_TIME_END(timeCount, t1);
#elif defined(CHECK_JACCARD)
    uint q = 2;
    double tau = 0.6;

    vector< triple<uint, uint, double > > results;

    REC_TIME_START(t1);
    joiner.SimilarityJoinJaccard(firstDataFilename,
                                 secondDataFilename,
                                 q, tau,
                                 results);
    REC_TIME_END(timeCount, t1);
#endif

    printf("tot = %d\n", results.size());
#ifdef TIME_COUNT
    printf("time = %lf\n", timeCount / 10);
#endif
    // for (int i = 0; i < results.size(); ++i) {
    //     printf("%d %d %d\n", results[i].id1, results[i].id2, results[i].sim);
    // }

return 0;
}
