#include <ctime>
#include "FZ_indexer.h"

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

// #define CHECK_JACCARD
#define CHECK_ED

int main() {

	char dataFilename[]="author.data";
	char indexFilename[]="author.index";
    unsigned q = 2;

	FZ_Indexer indexer;
	
	indexer.CreateIndex(dataFilename, q, indexFilename); 	
	indexer.LoadIndex(); 

#ifdef TIME_COUNT
	double timeCount = 0.0;
#endif

#if defined(CHECK_ED)
	FILE *fin = fopen("query_ed.txt", "r");
#elif defined(CHECK_JACCARD)
	FILE *fin = fopen("query_jaccard.txt", "r");
#endif

	int nrQuery;
	fscanf(fin, "%d\n", &nrQuery);


	for (int i = 0; i < nrQuery; ++i) {
		char qStr[BUFSIZ];

#if defined(CHECK_ED)
		unsigned int tau;
		fscanf(fin, "%s %u\n", qStr, &tau);
		vector< pair<unsigned, unsigned > > resultsED;
#elif defined(CHECK_JACCARD)
		double tau;
		fscanf(fin, "%s %lf\n", qStr, &tau);
		vector< pair<unsigned, double > > resultsJac;
#endif

		REC_TIME_START(t1);
#if defined(CHECK_ED)
		indexer.SearchED(qStr, tau, resultsED);
#elif defined(CHECK_JACCARD)
		indexer.SearchJaccard(qStr, tau, resultsJac);
#endif
		REC_TIME_END(timeCount, t1);


#if defined(CHECK_ED)
		printf("#%d: %d\n", i, resultsED.size());
#elif defined(CHECK_JACCARD)
		printf("#%d: %d\n", i, resultsJac.size());
#endif
	}

	fclose(fin);

#ifdef TIME_COUNT
	printf("average time = %lf\n", timeCount / nrQuery / 10);
#endif

	indexer.DestroyIndex(); 
	return 0;
}
