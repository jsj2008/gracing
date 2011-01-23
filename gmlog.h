#ifndef GMLOG_H
#define GMLOG_H
#define GM_LOG(fmt,...) do {\
  FILE * f=fopen("/tmp/data-change.txt","a");\
  if(f) {\
    fprintf(f,fmt,## __VA_ARGS__);\
    fclose(f);\
  }\
} while(0)

#endif // GMLOG_H
