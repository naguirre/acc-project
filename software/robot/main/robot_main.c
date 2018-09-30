/* brushed dc motor control example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

/*
 * This example will show you how to use MCPWM module to control brushed dc motor.
 * This code is tested with L298 motor driver.
 * User may need to make changes according to the motor driver they use.
 */

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_attr.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "wifi.h"
#include "socket_server.h"

#include "driver/mcpwm.h"
#include "soc/mcpwm_reg.h"
#include "soc/mcpwm_struct.h"


/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t wifi_event_group;
/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
const int WIFI_CONNECTED_BIT = BIT0;

QueueHandle_t queue = NULL;


static const char *TAG = "nawak";


#define GPIO_PWM0A_OUT 12   //Set GPIO 15 as PWM0A
#define GPIO_PWM0B_OUT 13   //Set GPIO 16 as PWM0B

#define GPIO_PWM1A_OUT 17   //Set GPIO 17 as PWM1A
#define GPIO_PWM1B_OUT 18   //Set GPIO 18 as PWM1B


static void mcpwm_example_gpio_initialize()
{
    printf("initializing mcpwm gpio...\n");
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, GPIO_PWM0A_OUT);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, GPIO_PWM0B_OUT);

    mcpwm_gpio_init(MCPWM_UNIT_1, MCPWM1A, GPIO_PWM1A_OUT);
    mcpwm_gpio_init(MCPWM_UNIT_1, MCPWM1B, GPIO_PWM1B_OUT);

}

/**
 * @brief motor moves in forward direction, with duty cycle = duty %
 */
static void brushed_motor_forward(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num , float duty_cycle)
{
    mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_B);
    mcpwm_set_duty(mcpwm_num, timer_num, MCPWM_OPR_A, duty_cycle);
    mcpwm_set_duty_type(mcpwm_num, timer_num, MCPWM_OPR_A, MCPWM_DUTY_MODE_0); //call this each time, if operator was previously in low/high state
}

/**
 * @brief motor moves in backward direction, with duty cycle = duty %
 */
static void brushed_motor_backward(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num , float duty_cycle)
{
    mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_A);
    mcpwm_set_duty(mcpwm_num, timer_num, MCPWM_OPR_B, duty_cycle);
    mcpwm_set_duty_type(mcpwm_num, timer_num, MCPWM_OPR_B, MCPWM_DUTY_MODE_0);  //call this each time, if operator was previously in low/high state
}



/**
 * @brief motor stop
 */
static void brushed_motor_stop(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num)
{
    mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_A);
    mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_B);
}


static void stop(uint16_t ms)
{
    brushed_motor_stop(MCPWM_UNIT_0, MCPWM_TIMER_0);
    brushed_motor_stop(MCPWM_UNIT_1, MCPWM_TIMER_1);
    vTaskDelay(ms / portTICK_RATE_MS);

}

/**
 * @brief motor moves in backward direction, with duty cycle = duty %
 */
static void turn_right(uint16_t ms)
{
    ESP_LOGI(TAG, "RIGHT %d", ms);

    brushed_motor_stop(MCPWM_UNIT_0, MCPWM_TIMER_0);
    brushed_motor_stop(MCPWM_UNIT_1, MCPWM_TIMER_1);
    brushed_motor_forward(MCPWM_UNIT_0, MCPWM_TIMER_0, 30.0);
    vTaskDelay(ms / portTICK_RATE_MS);

}

/**
 * @brief motor moves in backward direction, with duty cycle = duty %
 */
static void turn_left(uint16_t ms)
{
    ESP_LOGI(TAG, "LEFT %d", ms);

    brushed_motor_stop(MCPWM_UNIT_0, MCPWM_TIMER_0);
    brushed_motor_stop(MCPWM_UNIT_1, MCPWM_TIMER_1);
    brushed_motor_forward(MCPWM_UNIT_1, MCPWM_TIMER_1, 30.0);
    vTaskDelay(ms / portTICK_RATE_MS);

}

static void forward(uint16_t ms)
{
    ESP_LOGI(TAG, "FORWARD %d", ms);
    brushed_motor_stop(MCPWM_UNIT_0, MCPWM_TIMER_0);
    brushed_motor_stop(MCPWM_UNIT_1, MCPWM_TIMER_1);

    brushed_motor_forward(MCPWM_UNIT_0, MCPWM_TIMER_0, 30.0);
    brushed_motor_forward(MCPWM_UNIT_1, MCPWM_TIMER_1, 30.0);
    vTaskDelay(ms / portTICK_RATE_MS);

}


static void backward(uint16_t ms)
{
    ESP_LOGI(TAG, "BACKWARD %d", ms);
    brushed_motor_stop(MCPWM_UNIT_0, MCPWM_TIMER_0);
    brushed_motor_stop(MCPWM_UNIT_1, MCPWM_TIMER_1);

    brushed_motor_backward(MCPWM_UNIT_0, MCPWM_TIMER_0, 30.0);
    brushed_motor_backward(MCPWM_UNIT_1, MCPWM_TIMER_1, 30.0);
    vTaskDelay(ms / portTICK_RATE_MS);
}


static void start_wifi()
{
    ESP_LOGI(TAG, "starting network");

    /* FreeRTOS event group to signal when we are connected & ready to make a request */
    EventGroupHandle_t wifi_event_group = xEventGroupCreate();

    /* init wifi */
    initialise_wifi(wifi_event_group);
    ESP_LOGI(TAG, "Wait for connexion");
    /* Wait for the callback to set the CONNECTED_BIT in the event group. */
    xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT,
                        false, true, portMAX_DELAY);
    ESP_LOGI(TAG, "Connected");
}


/**
 * @brief Configure MCPWM module for brushed dc motor
 */
static void mcpwm_example_brushed_motor_control(void *arg)
{
    //1. mcpwm gpio initialization
    mcpwm_example_gpio_initialize();

    //2. initial mcpwm configuration
    printf("Configuring Initial Parameters of mcpwm...\n");
    mcpwm_config_t pwm_config;
    pwm_config.frequency = 1000;    //frequency = 500Hz,
    pwm_config.cmpr_a = 0;    //duty cycle of PWMxA = 0
    pwm_config.cmpr_b = 0;    //duty cycle of PWMxb = 0
    pwm_config.counter_mode = MCPWM_UP_COUNTER;
    pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);    //Configure PWM0A & PWM0B with above settings
    mcpwm_init(MCPWM_UNIT_1, MCPWM_TIMER_1, &pwm_config);    //Configure PWM1A & PWM1B with above settings

    Robot_Action action;
    int ret;
    while(1)
    {
        ret = xQueueReceive(queue ,&action,(TickType_t )(1000 / portTICK_PERIOD_MS));
        if (ret)
        {
            ESP_LOGI(TAG, "Action received : %s %d", action_type_to_str(action.type), action.duration);
            switch(action.type)
            {
            case ROBOT_ACTION_FORWARD:
                forward(action.duration);
                stop(1000);
                break;
            case ROBOT_ACTION_BACKWARD:
                backward(action.duration);
                stop(1000);
                break;
            case ROBOT_ACTION_LEFT:
                turn_left(action.duration);
                stop(1000);
                break;
            case ROBOT_ACTION_RIGHT:
                turn_right(action.duration);
                stop(1000);
                break;
            default:
                break;
            }
        }
    }

}


void app_main()
{

    queue = xQueueCreate(1, sizeof(Robot_Action));

    nvs_flash_init();
    start_wifi();
    ESP_LOGI(TAG, "Launch motor thread");
    xTaskCreate(mcpwm_example_brushed_motor_control, "motor", 4096, NULL, 5, NULL);
    ESP_LOGI(TAG, "Launch socket server");
    xTaskCreate(socket_server, "motor", 4096, NULL, 6, NULL);

    while(1)
    {
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}
