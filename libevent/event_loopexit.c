#include <event2/event.h>
#include <stdio.h>
#include <stdlib.h>


void run_base_with_ticks(struct event_base *base)
{
  struct timeval ten_sec;

  ten_sec.tv_sec = 10; // 设置单位秒
  ten_sec.tv_usec = 0; // 设置单位微秒

  /* Now we run the event_base for a series of 10-second intervals, printing
     "Tick" after each.  For a much better way to implement a 10-second
     timer, see the section below about persistent timer events. */
  while (1) {
     /* This schedules an exit ten seconds from now. */
     event_base_loopexit(base, &ten_sec);

     event_base_dispatch(base);
     puts("Tick");
  }
}

int main(){
    struct event_base * base = event_base_new();
    run_base_with_ticks(base);
    return 0;
}