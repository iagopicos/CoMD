#include "zmqstuff.h"

void *initZmqStuff(Command *cmd, SimFlat *s)
{
  //Print 0MQ version
  if (getMyRank() == 0) {
    int major, minor, patch;
    zmq_version(&major, &minor, &patch);
    printf("Current ØMQ version is %d.%d.%d\n\n", major, minor, patch);

#ifdef SINGLE
    printf("Using SINGLE precision\n");
#else
    printf("Using DOUBLE precision\n");
#endif
    printf("Datatype sizes in this machine:\n\tInteger: %d bytes\n\tLong: %d bytes\n\tReal: %d bytes\n\tPointer: %d bytes\n\n", sizeof(int), sizeof(long), sizeof(real_t), sizeof(void *));
    int nmsgs = (int)ceilf((float)s->nSteps/s->printRate);
    real_t msgsize = (2 * sizeof(int) + sizeof(long) + s->atoms->nLocal * (sizeof(real3) + sizeof(int)))*1.0/1024/1024;
    printf("ZMQ comm. in this run: %d messages from each rank, %f MB/msg => %f MB per rank\n\n", nmsgs, msgsize, nmsgs * msgsize);
    printf("ZMQ Messages format:\n  MPIrank  msgid(ts)  timestamp  id[#local_atoms_in_rank]  locations[#local_atoms_in_rank]\n      int        int       long        #local_atoms x int          #local_atoms x 3 x real\n  %d bytes    %d bytes    %d bytes    %12d x %d bytes       %12d x 3 x %d bytes\n\n", sizeof(int), sizeof(int), sizeof(long), s->atoms->nLocal, sizeof(int), s->atoms->nLocal, sizeof(real_t));
  }

  int port = cmd->port + (getMyRank() % cmd->portNum);
  char *server = 0;

  unsigned int dir_length = 0;
  char **dir_names = read_dir(cmd->hostDir, &dir_length);
  if (dir_names == NULL)
    exit(1);

  for (unsigned int i = 0; i < dir_length - 1; ++i) {
    //printf("value of a: %s\n", dir_names[i]);
    if (atoi(dir_names[i]) == port) {
      //if(server)
      //free(server);
      server = read_hostname(cmd->hostDir, dir_names[i]);
      if (server == NULL)
        exit(1);
      check_hostname(server);
      //printf("Hostname found! It is: %s\n", server);
    }
  }

  //Init ZMQ
  void *context = zmq_ctx_new();
  s->sender = zmq_socket(context, ZMQ_PUSH);
  zmq_setsockopt(s->sender, ZMQ_SNDHWM, &(cmd->hwm), sizeof(cmd->hwm));
  char connect[MAX_CHARS_KEY];
  sprintf(connect, "tcp://%s:%d", server, port);
  printf("MPI Rank %d/%d - ZMQ recipient: %s\n", getMyRank(), getNRanks(), connect);
  zmq_connect(s->sender, connect);

  return (context);
}

void closeZmqStuff(void *context, SimFlat *s)
{
  //printf("Terminating ZMQ context\n");
  zmq_close(s->sender);
  zmq_ctx_term(context);
}

//Write the file containg the amout of data sent through ZMQ
void logDataSizeSent(long totalBytesSent, long miliseconds)
{
  if(getMyRank() == 0) {
    printf("Total MiB sent: %lf \n",totalBytesSent*1.0/1024/1024);
    printf("Avg. rate: %lf MB/s\n\n",(totalBytesSent*1.0/1000)/miliseconds);
    const char *homedir = getpwuid(getuid())->pw_dir;
    //Get home dir
    homedir = getpwuid(getuid())->pw_dir;
    char path[200];

    sprintf(path,"%s/%s",homedir,"CoMD_BytesSent.txt");

    FILE *f = fopen(path, "w");

    if (f == NULL) {
      printf("Error opening output file!\n");
      exit(1);
    }

    fprintf(f, "%ld", totalBytesSent);
    fclose(f);
  }
}
