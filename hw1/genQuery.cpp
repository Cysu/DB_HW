#include <cstdio>
#include <cstring>
#include <ctime>
#include <cstdlib>
#include <string>

using std::string;

string genRandomString()
{
	string ret;
	int len = rand() % 20 + 1;
	for (int i = 0; i < len; ++i) {
		int a = rand() % 27;
		if (a == 26) ret += '_';
		else ret += (char)((int)'a'+a);
	}
	return ret;
}

int main(int argc, char *argv[])
{
	int nrQuery = atoi(argv[1]);
	FILE *fout = fopen("query.txt", "w");
	srand(time(0));
	
	fprintf(fout, "%d\n", nrQuery);
	for (int i = 0; i < nrQuery; ++i) {
		string qStr = genRandomString();
		unsigned int tau = rand() % (qStr.length()+1);
		fprintf(fout, "%s %u\n", qStr.c_str(), tau);
	}
	fclose(fout);

	return 0;
}