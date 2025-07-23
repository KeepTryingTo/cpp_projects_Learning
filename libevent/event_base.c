#include <event.h>
#include <stdio.h>
#include <stdlib.h>

struct event_base *base;
enum event_method_feature f;

int main(){
    int i;
    const char **methods = event_get_supported_methods();
    printf("Starting Libevent %s.  Available methods are:\n",
        event_get_version());
    for (i = 0; methods[i] != NULL; ++i) {
        printf("    %s\n", methods[i]);
    }

    base = event_base_new();
    if (!base) {
        puts("Couldn't get an event_base!");
    } else {
        printf("Using Libevent with backend method %s.",
            event_base_get_method(base));
        f = event_base_get_features(base);
        if ((f & EV_FEATURE_ET))
            printf("  Edge-triggered events are supported.");
        if ((f & EV_FEATURE_O1))
            printf("  O(1) event notification is supported.");
        if ((f & EV_FEATURE_FDS))
            printf("  All FD types are supported.");
        puts("");
    } 

    // 释放资源
    event_base_free(base);
    return 0;
}