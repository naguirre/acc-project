#ifndef __SOCKET_SERVER_H__
#define __SOCKET_SERVER_H__

#include "common.h"

void socket_server(void *ignore);
ROBOT_ACTION_TYPE str_to_action_type(char *action);
const char *action_type_to_str(ROBOT_ACTION_TYPE type);

#endif /* __SOCKET_SERVER_H__ */
