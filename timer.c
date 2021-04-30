/*
 * A introduction to simple timer service based on Libevent 
 */
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <event2/event.h>

/* Definitions */
typedef void (*callback_handler_t) (void *cb_arg);

typedef struct timer_s {
	struct event       *ev;
	callback_handler_t cb_func;
	void               *cb_func_args;
} tmr_t;

/* Timer event handler will call corresponding handler */
void timer_event_handler(int s, short evtype, void *cb_arg) {
	if (cb_arg == NULL) {
		return;
	}
	tmr_t *timer = (tmr_t *)cb_arg;
	if (timer->cb_func != NULL) {
		timer->cb_func(timer);
	}
	return;	
}

/* Libevent timer initialization */
void timer_init(struct event_base *evbase, unsigned int msec, callback_handler_t func, void *cb_func_args) {
	struct timeval tv;
	tmr_t *timer = malloc(sizeof(tmr_t));
	assert(timer);

	tv.tv_sec           = msec/1000;
	tv.tv_usec          = (msec%1000)*1000;
	timer->cb_func      = func;
	timer->cb_func_args = cb_func_args;
	timer->ev           = event_new(evbase, -1, EV_PERSIST, timer_event_handler, timer);
	evtimer_add(timer->ev, &tv);
	return;
}

/* Disable timer */
void timer_stop(tmr_t *timer) {
	if (timer != NULL) {
		event_del(timer->ev);
		event_free(timer->ev);
		free(timer);
		timer = NULL;
	} else {
		printf("No timer available\n");
	}
	return;
}

/* Test code */

int cnt = 0;

void handler_func(void *args) {
	time_t now;
	time(&now);
	tmr_t *timer = (tmr_t *)args;	
	printf("Timer expired, arg: %s %s\n", (char *)timer->cb_func_args, ctime(&now));
	if (cnt++ >= 5) {
		timer_stop(timer);
	}
	return;
}

int main(int argc, char **argv) {
	if (argc != 2) {
		printf("execute cmd: ./prog_name <time in seconds>\n");
		return;
	}
	/* initialize libevent */
	struct event_base *evbase = event_base_new();
	assert(evbase);

	timer_init(evbase, atoi(argv[1])*1000, handler_func, (void *)"hello");
	event_base_dispatch(evbase);	
	return 0;
}
