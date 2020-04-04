#include "robot.h"
#include <app/robot.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

typedef enum
{
    COMMAND_NONE = 0,
    COMMAND_LEFT,
    COMMAND_RIGHT,
    COMMAND_FORWARD,
    COMMAND_BACKWARD
}RobotCommand;

#define ROBOT_COMMAND_NUM           10
#define ROBOT_COMMAND_DISTANCE      0.2

RobotCommand commands[ROBOT_COMMAND_NUM] = {
    COMMAND_FORWARD,
    COMMAND_LEFT,
    COMMAND_FORWARD,
    COMMAND_RIGHT,
    COMMAND_RIGHT,
    COMMAND_BACKWARD,
    COMMAND_LEFT,
    COMMAND_FORWARD,
    COMMAND_FORWARD,
    COMMAND_RIGHT
};

void robot_task(void* pvParameters) 
{
     int command_index = 0;
    printf("Creating robot object\n");
    Robot * robot = new Robot(0.25, true);

    robot->GoThrough(0.8, 0.5, -1.0);

    while (robot->CheckMovementAlmostDone() == false)
    {
        usleep(10000);
        robot->Run();
    }

    robot->GoTo(1.2, 1.2, -0.5);

    while (robot->CheckMovementDone() == false)
    {
        usleep(10000);
        robot->Run();
    }
 
    while ((command_index < ROBOT_COMMAND_NUM) && (commands[command_index] != COMMAND_NONE))
    {
        switch (commands[command_index])
        {
            case COMMAND_LEFT:
                robot->Rotate(PI_2);
                break;

            case COMMAND_RIGHT:
                robot->Rotate(-PI_2);
                break;

            case COMMAND_FORWARD:
                robot->Translate(ROBOT_COMMAND_DISTANCE);
                break;

            case COMMAND_BACKWARD:
                robot->Translate(-ROBOT_COMMAND_DISTANCE);
                break;

            default:
                exit(1);
                break;
        }
        
        while (robot->CheckMovementDone() == false)
        {
            usleep(10000);
            robot->Run();
        }

        command_index++;
    }

    robot->SplittedGoTo(0.0, 0.0, 1.0);

    while (robot->CheckMovementDone() == false)
    {
        usleep(10000);
        robot->Run();
    }

    robot->PointTowards(0.0, 1.0);

    while (robot->CheckMovementDone() == false)
    {
        usleep(10000);
        robot->Run();
    }
}
