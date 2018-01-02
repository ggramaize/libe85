#include "strpad.h"

#include <stdlib.h>
#include <string.h>

char* strpad( const char* input, char padchar, size_t every_n)
{
	size_t input_sz = strlen( input);
	size_t num_of_inserts = input_sz / every_n;
	
	char *result = (char*) malloc( input_sz + num_of_inserts + 1);
	memset( result, 0, input_sz + num_of_inserts + 1 );

	memcpy( result, input, input_sz + 1);

	size_t pivot = every_n * num_of_inserts;
	size_t blk2mv_sz = (input_sz % every_n )+ 1;
	for( size_t i=0 ; i<num_of_inserts ; ++i)
	{
		memmove( result+pivot+1, result+pivot, blk2mv_sz);
		result[ pivot] = padchar;
		blk2mv_sz += every_n + 1;
		pivot -= every_n;
	}

	return result;
}
