#ifndef __E85_H
#define __E85_H

#include <stdint.h>

char* E85_encode( void **in_data, size_t in_data_sz); 

int E85_decode ( const char *in_str, uint8_t **result, size_t *decoded_size);

inline int is_valid_Z85_char( char x);

void strip_garbage_E85( char *in, char **out);

void strip_baseline_E85( char *in, char **out);

int validate_E85_string( char *str);

#endif
