#include "signals.h"

void block_signal(int sig) {
   //Ignore SIGINT signal
}

void setup_sighandler(struct sigaction* sa, void (*handler)(int), 
        int signal) {
    memset(sa, 0, sizeof(*sa));
    sa->sa_handler = handler;
    sa->sa_flags = SA_RESTART;
    sigaction(signal, sa, 0);
}
