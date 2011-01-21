#ifndef TESTEVENT_H
#define TESTEVENT_H

#include "testdefinitiondatatypes.h"

int init_event_system();
void cleanup_event_system();
int wait_for_event(td_event *event);
int send_event(td_event *event);

#endif
