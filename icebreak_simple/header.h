
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <setjmp.h>
#include "../PTEditor/ptedit_header.h"
#include "../cacheutils.h"

/* see asm.S */
extern void s_faulty_load();

extern uint8_t * address_supervisor;
extern uint8_t * address_normal;
extern uint8_t * oracles;

#define PAGE_SIZE 4096


void setup_oracle(){
    printf("[+] Flush+Reload Threshold: ");
    CACHE_MISS = detect_flush_reload_threshold();
    printf("%lu\n", CACHE_MISS);
    for(int i = 0; i < 256; i++){
        flush((uint8_t *)&oracles + i * PAGE_SIZE);
    }
}

void setup_pages(){
    
    uint8_t * ptr = NULL;
    for(int i = 0; i < 32; i++){
        ptr = (uint8_t *)&address_normal + i * PAGE_SIZE;        
        ptr[0] = 0;
    }    
    
    ptedit_init();  
    ptr = (uint8_t *)&address_supervisor;        
    ptr[0] = 0;
    ptedit_pte_clear_bit(ptr, 0, PTEDIT_PAGE_BIT_USER);
    ptedit_cleanup();
}


void setup_fh(){
    signal(SIGSEGV, trycatch_segfault_handler); 
    signal(SIGFPE, trycatch_segfault_handler);
}