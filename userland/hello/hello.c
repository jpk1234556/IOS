/* hello.c - Brandon Media OS User Application */
#include "libc.h"

int main(int argc, char *argv[]) {
    (void)argc;  /* Suppress unused parameter warning */
    (void)argv;  /* Suppress unused parameter warning */
    
    /* Neural interface greeting */
    neural_output("Greetings from the neural user space!");
    
    /* Display process information */
    int pid = getpid();
    printf("Neural Process ID: ");
    
    /* Simple number to string conversion and output */
    char pid_str[16];
    itoa(pid, pid_str, 10);
    puts(pid_str);
    
    /* Test system calls */
    neural_output("Testing neural interface commands...");
    
    /* Test write system call */
    write(STDOUT_FILENO, "[APP] Direct neural write test\\n", 31);
    
    /* Test sleep */
    neural_output("Entering temporal suspension for 2 seconds...");
    sleep(2);
    neural_output("Temporal suspension complete!");
    
    /* Test yield */
    neural_output("Yielding neural processing cycles...");
    yield();
    neural_output("Neural processing resumed!");
    
    /* Get status */
    int status = neural_status();
    
    /* Main application loop */
    for (int i = 0; i < 5; i++) {
        neural_output("Neural application iteration");
        
        /* Convert iteration number to string */
        char iter_str[16];
        itoa(i + 1, iter_str, 10);
        write(STDOUT_FILENO, "[APP] Iteration: ", 17);
        puts(iter_str);
        
        /* Sleep between iterations */
        sleep(1);
        yield();
    }
    
    neural_output("Neural application completing execution...");
    
    /* Exit cleanly */
    exit(42);  /* Answer to the ultimate question */
    
    return 0;  /* Should never reach here */
}