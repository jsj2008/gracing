#ifndef GMLOG_H
#define GMLOG_H
#include <stdio.h>
#define GM_LOG_FILE(fmt,...) do {\
  FILE * f=fopen("/tmp/log.txt","a");\
  if(f) {\
    fprintf(f,fmt,## __VA_ARGS__);\
    fclose(f);\
  }\
} while(0)

#define GM_LOG_STDOUT(fmt,...) do {\
   fprintf(stdout,fmt,## __VA_ARGS__);\
} while(0)

#define GM_LOG(fmt,...) GM_LOG_FILE(fmt,## __VA_ARGS__)

#endif // GMLOG_H
