#ifndef H_PAGING
#define H_PAGING

#include "types.h"

void init_paging( uint64_t phys_offset );
void showEntries( int ptEntries, int ptTables );

#endif