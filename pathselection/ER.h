#ifndef ER_H
#define ER_H

#include "Corpus.h"
#include "Transformation.h"
using namespace std;

class ER
{
public:
	ER() {}
	ER(string);
	void run_ER();

	//constants
	const static double JAC_THRESHOLD;

private:

	//number of strings
	int n;

	//strings;
	vector<string> str;
};

# endif // ER_H
