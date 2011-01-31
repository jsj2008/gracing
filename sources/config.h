#ifndef CONFIG_H
#define CONFIG_H

#define CFG_PARAM_D(p)  double p
#define CFG_INIT_D(p,v) p=v;

#define CFG_PARAM_V3(p)         double p[3]
#define CFG_INIT_V3(p,v1,v2,v3) p[0]=v1; p[1]=v2; p[2]=v3

#endif
