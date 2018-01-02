
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "z85.h"
#include "strpad.h"

#define	READ_BLK_SIZE	128

char* E85_encode( void **in_data, size_t in_data_sz) 
{
	char* out_data = NULL;
	size_t out_data_sz = 0;

	if( in_data_sz % 4 != 0 )
	{
		// Zero-Pad the input

		*in_data = (uint8_t*) realloc( *in_data, in_data_sz + (4 - in_data_sz % 4));
		memset( ( (char*) *in_data ) + in_data_sz, 0, 4 - in_data_sz % 4);
		out_data = Z85_encode( *in_data, in_data_sz + (4 - in_data_sz % 4));
	}
	else
		out_data = Z85_encode( *in_data, in_data_sz);
	
	if( in_data_sz % 4 != 0 )
	{
		// Append the erasure characters

		out_data_sz = strlen(out_data);
		out_data = (char*) realloc( out_data, out_data_sz + (4 - in_data_sz % 4));
		for( size_t i=0; i<4-in_data_sz%4; ++i)
			out_data[ out_data_sz+i ] = '_';

		// Set the string terminator
		out_data[ out_data_sz+(4-in_data_sz%4) ] = 0;
	}

	return out_data;
}

int E85_decode ( const char *in_str, uint8_t **result, size_t *decoded_size)
{
	size_t in_len = strlen( in_str);

	char *z85_str = (char*) malloc( in_len - ( in_len % 5 ) + 1 );
	memcpy( z85_str, in_str, in_len - (in_len % 5));
	z85_str[ in_len - (in_len % 5) ] = 0;

	if( Z85_decode( z85_str, result, decoded_size) != 0 )
		return 1;

	free( z85_str);

	if( in_len % 5 != 0)
	{
		*decoded_size -= in_len % 5;
		*result = (uint8_t*) realloc( *result, *decoded_size);
	}

	return 0;
}

inline int is_valid_Z85_char( char x)
{
	return ( 
			( x >= 0x3C && x <= 0x5B ) || 
			( x >= 0x61 && x <= 0x7B ) || 
			( x >= 0x2D && x <= 0x3A ) || 
			( x >= 0x23 && x <= 0x26 ) || 
			( x >= 0x28 && x <= 0x2B ) || 
			( x >= 0x5D && x <= 0x5E ) || 
			x == 0x21 || x == 0x7D 
		);
}

void strip_garbage_E85( char *in, char **out)
{
	size_t in_len = strlen( in);
	size_t out_len = 0;
	*out = (char*) malloc( in_len+1); 

	for( size_t i = 0; i < in_len; ++i)
	       if( is_valid_Z85_char( in[i] ) || in[i] == '_' )
	       {
			*(*out+out_len++) = in[i];
	       }

	*(*out+out_len++) = 0;
	*out = (char*) realloc( *out, out_len);	
}

void strip_baseline_E85( char *in, char **out)
{
	size_t in_len = strlen( in);
	size_t out_len = 0;
	*out = (char*) malloc( in_len+1); 

	for( size_t i = 0; i < in_len; ++i)
	       if( in[i] != '\n' && in[i] != '\r' && in[i] != ' ' )
	       {
			*(*out+out_len++) = in[i];
	       }

	*(*out+out_len++) = 0;
	*out = (char*) realloc( *out, out_len);	
}

int validate_E85_string( char *str)
{
	size_t in_len = strlen( str);
	char *s = str;

	size_t Z85_bound = (in_len / 5) * 5;

	if( in_len % 5 != 0 )
	{
		// Check erasure tail
		
		// 4 erasures isn't valid
		if( in_len % 5 == 4 )
			return false;
		
		// Check erasure chars
		for( size_t i=0; i< in_len%5; ++i)
			if( str[ Z85_bound + i ] != '_' )
				return false;
	}

	s = str;
	for( size_t i=0; i < Z85_bound; ++i, ++s )
	{
		if( is_valid_Z85_char(*s) == false )
			return false;
	}

	return true;
}

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
