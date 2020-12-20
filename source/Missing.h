#if defined( VITA )
#include <stddef.h>
char* getcwd( char* buf, size_t size );
int chdir( const char* path);
#endif
