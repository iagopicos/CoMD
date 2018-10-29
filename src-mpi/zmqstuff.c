#include "zmqstuff.h"

void *initZmqStuff(Command *cmd, SimFlat *s)
{
  //Print 0MQ version
  if (getMyRank() == 0) {
    int major, minor, patch;
    zmq_version(&major, &minor, &patch);
    printf("Current ØMQ version is %d.%d.%d\n", major, minor, patch);
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
      printf("Hostname found! It is: %s\n", server);
    }
  }

  //Init ZMQ
  void *context = zmq_ctx_new();
  s->sender = zmq_socket(context, ZMQ_PUSH);
  zmq_setsockopt(s->sender, ZMQ_SNDHWM, &(cmd->hwm), sizeof(cmd->hwm));
  char connect[MAX_CHARS_KEY];
  sprintf(connect, "tcp://%s:%d", server, port);
  printf("%s\n", connect);
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
void logDataSizeSent(long totalBytesSent)
{
  if(getMyRank() == 0) {
    printf("Total MB sent: %lf \n",totalBytesSent*1.0/1024/1024);
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
