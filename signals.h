#ifndef SIGNALS_H
#define SIGNALS_H

#include "job.h"
#include "helper.h"
#include "parsing.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <csse2310a3.h>
#include <fcntl.h>
#include <ctype.h>
#include <signal.h>

#endif //SIGNALS_H

/* block_signal()
 * --------------
 * Does nothing as to cause jobthing to do nothing is response to send signal
 */
void block_signal(int sig);

/* setup_sighandler()
 * ------------------
 * This function sets up the basic signal handlers which use the sa_handler
 * not the sa_action signal handler.
 *
 * sa: pointer to sigaction that needs to be set up.
 *
 * handler: the function that is used to handle the signal.
 *
 * signal: the signal that the handler responds to.
 */
void setup_sighandler(struct sigaction* sa, void (*handler)(int), int signal);


