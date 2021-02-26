#ifndef H_PAGING
#define H_PAGING

#include "types.h"

void init_paging();
void showEntries( int ptEntries, int ptTables );

inline uint8_t getLv4Index( uint64_t virt_addr );
inline uint8_t getLv3Index( uint64_t virt_addr );
inline uint8_t getLv2Index( uint64_t virt_addr );
inline uint8_t getLv1Index( uint64_t virt_addr );

uint64_t to_phys( uint64_t virt_addr );

#endif