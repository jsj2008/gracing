#ifndef GMLOG_H
#define GMLOG_H
#include <stdio.h>

#ifndef LOG_FILE_NAME
#define LOG_FILE_NAME "/tmp/log.txt"
#endif

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

//fprintf(stdout,"%s:%d -| ",__FILE__,__LINE__);\

#ifdef LOG_TO_STDOUT
#define GM_LOG(fmt,...) GM_LOG_STDOUT(fmt,## __VA_ARGS__)
#else
#define GM_LOG(fmt,...) GM_LOG_FILE(fmt,## __VA_ARGS__)
#endif

#endif // GMLOG_H
