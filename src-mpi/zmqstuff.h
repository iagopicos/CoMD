#ifndef __ZMQSTUFF_H_
#define __ZMQSTUFF_H_

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <zmq.h>
#include "CoMDTypes.h"
#include "parallel.h"
#include "mycommand.h"
#include "helpers.h"

void *initZmqStuff(Command *cmd, SimFlat *s);
void closeZmqStuff(void *context, SimFlat *s);
void logDataSizeSent(long totalBytesSent);

#endif
