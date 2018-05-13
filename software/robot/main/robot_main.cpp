#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

#include <app/robot.h>

#define PERIODE_ORDONNANCEUR        (0.01)

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

extern "C" {
	void app_main(void);
}

void app_main()
{
     int command_index = 0;
     printf("ACC Controller!\n");

     /* Print chip information */
     esp_chip_info_t chip_info;
     esp_chip_info(&chip_info);
     printf("This is ESP32 chip with %d CPU cores, WiFi%s%s, ",
            chip_info.cores,
            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
            (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

     printf("silicon revision %d, ", chip_info.revision);

     printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");


     Robot * robot = new Robot(PERIODE_ORDONNANCEUR, false);
#if 1
     robot->GoThrough(0.8, 0.5, -1.0);

     while (robot->CheckMovementAlmostDone() == false)
     {
	  vTaskDelay(10/portTICK_PERIOD_MS);	 
	  robot->Run();
     }

     robot->GoTo(1.2, 1.2, -0.5);

     while (robot->CheckMovementDone() == false)
     {
	  vTaskDelay(10/portTICK_PERIOD_MS);	 
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
	       vTaskDelay(10/portTICK_PERIOD_MS);
	       robot->Run();
	  }

	  command_index++;
     }

     robot->SplittedGoTo(0.0, 0.0, 1.0);

     while (robot->CheckMovementDone() == false)
     {
	  vTaskDelay(10/portTICK_PERIOD_MS);
	  robot->Run();
     }

     robot->PointTowards(0.0, 1.0);

     while (robot->CheckMovementDone() == false)
     {
	  vTaskDelay(10/portTICK_PERIOD_MS);
	  robot->Run();
     }
    
#endif

}
