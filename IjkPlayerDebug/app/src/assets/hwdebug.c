#include "hwdebug.h"

void setKsDebugCallback(void (*p)(char* str,void *ffp1),void *ffps){
	 pMethod=p;
	 ffp=ffps;
}
