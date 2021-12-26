#ifndef H_SYSCALL
#define H_SYSCALL

#include "types.h"
#include "io/io.h"

// static uint64_t* syscall_table[1] = {
//     (uint64_t*) &test_syscall
// };

static inline void syscall() {
    // rax syscall nummber
    
    __asm__ ("syscall");
}

#endif // H_SYSCALL
