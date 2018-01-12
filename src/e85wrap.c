
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "e85.h"
#include "z85.h"
#include "strpad.h"

int main( int argc, char **argv)
{
	int wrap_cols = 76;
	int decode = 0;
	int ignore_garbage = 0;
	int opt;
	FILE* fIn = stdin;

	uint8_t *inputData = NULL;
	size_t inputDataSZ = 0;
	char * outputData = NULL;
	size_t outputDataSZ = 0;

	while ((opt = getopt( argc, argv, "diw:")) != -1)
	{
		switch(opt)
		{
		case 'd':
			decode = 1;
			break;

		case 'i':
			ignore_garbage = 1;
			break;

		case 'w':
			wrap_cols = atoi(optarg);
			if( wrap_cols == 0)
			{
				fprintf( stderr, "Usage: %s [-d] [-i] [-w cols] filename\n", argv[0]);
				exit( EXIT_FAILURE);
			}
			break;

		default:
			fprintf( stderr, "Usage: %s [-d] [-i] [-w cols] filename\n", argv[0]);
			exit( EXIT_FAILURE);
		}
	}

	if( optind < argc && strncmp( argv[optind], "-", 2) != 0)
	{
		fIn = fopen( argv[optind], "rb");
		if( fIn == NULL )
		{
			fprintf( stderr, "Cannot open file %s.\n", argv[optind]);
			exit( EXIT_FAILURE);
		}

		// Get File size
		fseek( fIn, 0, SEEK_END);
		inputDataSZ = ftell( fIn);
		rewind( fIn);

		//fadvise( fIn, FADV_SEQUENTIAL);
		inputData = (uint8_t*) malloc( inputDataSZ);
		if( inputData == NULL)
		{
			fprintf( stderr, "Memory allocation error");
			fclose(fIn);
			exit(1);
		}
		fread( inputData, 1, inputDataSZ, fIn);
		fclose(fIn);


	}
	else
	{
		// Reopen STDIN in binary
		freopen( NULL, "rb", stdin);

		char c = fgetc(stdin);

		inputData = (uint8_t*) malloc(1);
		++inputDataSZ;

		if( !feof(stdin) )
			inputData[0] = c;
		else
			inputData[0] = 0;

		c = fgetc( stdin);
		while( !feof(stdin) )
		{
			inputData = (uint8_t*) realloc( inputData, ++inputDataSZ);
			inputData[ inputDataSZ-1 ] = c;
			c = fgetc( stdin);
		}
	}

	if( decode != 1 )
	{
		outputData = E85_encode( (void**) &inputData, inputDataSZ);
		fprintf( stdout, "%s\n", strpad(outputData, '\n', wrap_cols) );
	}
	else
	{
		char *cleanInputData = NULL;

		if( ignore_garbage == 1 )
			strip_garbage_E85( (char*) inputData, &cleanInputData);
		else
			strip_baseline_E85( (char*) inputData, &cleanInputData);
			
	
		free( inputData);
		inputData = (uint8_t*) cleanInputData;

		if( validate_E85_string( (char*) inputData) != true )
		{
			fprintf( stderr, "Input is not a valid E85 stream.\n");
			exit(1);
		}
		
		E85_decode( (char*) inputData, (uint8_t**) &outputData, &outputDataSZ);
		fwrite( outputData, outputDataSZ, 1, stdout);
		fflush( stdout);
	}

	return 0;
}
