
#include <libq/config.h>

#include <time.h>
//#include <sys/time.h>
#include "time.h"


/*uint64_t q_millis() {
	struct timeval tv;
	if(gettimeofday(&tv,NULL)) return 0;
	return (uint64_t)(tv.tv_sec*1000+tv.tv_usec/1000);
}*/

/*static int timespec_get(struct timespec *ts,int base) {
	switch(base) {
		case TIME_UTC:
			if(__clock_gettime(CLOCK_REALTIME,ts)<0) return 0;
			break;
		default:
			return 0;
	}
	return base;
}*/

uint64_t q_millis() {
	struct timespec ts;
	timespec_get(&ts,TIME_UTC);
	return (uint64_t)(ts.tv_sec*1000L+ts.tv_nsec/1000000L);
}


