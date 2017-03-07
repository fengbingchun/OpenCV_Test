#include <assert.h>
#include <iostream>
#include <string>
#include <fstream>

#include "test_all.hpp"
#include "funset.hpp"

int main()
{
	int ret = test_encode_decode();

	if (ret == 0) fprintf(stderr, "test success\n");
	else fprintf(stderr, "testn fail\n");

	return 0;
}

