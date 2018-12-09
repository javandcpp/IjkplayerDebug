typedef void (*pM)(char *str,void *fp);
pM pMethod;
void * ffp;

void setKsDebugCallback(void (*p)(char* str,void *ffp1),void *ffps);
