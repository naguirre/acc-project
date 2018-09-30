#ifndef __COMMON_H__
#define __COMMON_H__

typedef enum _ROBOT_ACTION_TYPE
{
    ROBOT_ACTION_FORWARD,
    ROBOT_ACTION_BACKWARD,
    ROBOT_ACTION_LEFT,
    ROBOT_ACTION_RIGHT,
    ROBOT_ACTION_UNKNOWN
} ROBOT_ACTION_TYPE;

typedef struct _Robot_Action {
    ROBOT_ACTION_TYPE type;
    uint16_t duration;
} Robot_Action;

extern QueueHandle_t queue;


#endif /* __COMMON_H__ */
