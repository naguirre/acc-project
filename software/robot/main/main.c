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
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/ledc.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "wifi.h"
#include "websocket_server.h"

#include "driver/mcpwm.h"
#include "soc/mcpwm_reg.h"
#include "soc/mcpwm_struct.h"
#include "common.h"

#include "robot.h"

#define LED_PIN CONFIG_LED_PIN
/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t wifi_event_group;
/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
const int WIFI_CONNECTED_BIT = BIT0;

QueueHandle_t queue_actions = NULL;
QueueHandle_t queue_moves = NULL;
static QueueHandle_t client_queue;
const static int client_queue_size = 10;
static ledc_channel_config_t ledc_channel;

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

static void robot_move(int8_t mx, int8_t my)
{
    ESP_LOGI(TAG, "MOVE %d %d", mx, my);

    if (mx == 0 && my == 0)
    {
        brushed_motor_stop(MCPWM_UNIT_0, MCPWM_TIMER_0);
        brushed_motor_stop(MCPWM_UNIT_1, MCPWM_TIMER_1);
    }
        
    if (mx > 0 && (abs(my) < 40))
    {
        brushed_motor_forward(MCPWM_UNIT_0, MCPWM_TIMER_0, abs(mx));
        brushed_motor_forward(MCPWM_UNIT_1, MCPWM_TIMER_1, abs(mx));
    }
    else if (mx < 0 && (abs(my) < 40))
    {
        brushed_motor_backward(MCPWM_UNIT_0, MCPWM_TIMER_0, abs(mx));
        brushed_motor_backward(MCPWM_UNIT_1, MCPWM_TIMER_1, abs(mx));
    }
    else if (my > 0)
    {
        brushed_motor_backward(MCPWM_UNIT_0, MCPWM_TIMER_0, abs(my));
        brushed_motor_forward(MCPWM_UNIT_1, MCPWM_TIMER_1, abs(my));
    }
    else if (my < 0)
    {
        brushed_motor_forward(MCPWM_UNIT_0, MCPWM_TIMER_0, abs(my));
        brushed_motor_backward(MCPWM_UNIT_1, MCPWM_TIMER_1, abs(my));
    }


// Differential Steering Joystick Algorithm
// ========================================
//   by Calvin Hass
//   https://www.impulseadventure.com/elec/
//
// Converts a single dual-axis joystick into a differential
// drive motor control, with support for both drive, turn
// and pivot operations.
//
#if 1
// INPUTS
// int     mx;              // Joystick X input                     (-128..+127)
// int     my;              // Joystick Y input                     (-128..+127)

// OUTPUTS
int     nMotMixL;           // Motor (left)  mixed output           (-128..+127)
int     nMotMixR;           // Motor (right) mixed output           (-128..+127)

// CONFIG
// - fPivYLimt  : The threshold at which the pivot action starts
//                This threshold is measured in units on the Y-axis
//                away from the X-axis (Y=0). A greater value will assign
//                more of the joystick's range to pivot actions.
//                Allowable range: (0..+127)
float fPivYLimit = 32.0;
			
// TEMP VARIABLES
float   nMotPremixL;    // Motor (left)  premixed output        (-128..+127)
float   nMotPremixR;    // Motor (right) premixed output        (-128..+127)
int     nPivSpeed;      // Pivot Speed                          (-128..+127)
float   fPivScale;      // Balance scale b/w drive and pivot    (   0..1   )


// Calculate Drive Turn output due to Joystick X input
if (my >= 0) {
  // Forward
  nMotPremixL = (mx>=0)? 127.0 : (127.0 + mx);
  nMotPremixR = (mx>=0)? (127.0 - mx) : 127.0;
} else {
  // Reverses
  nMotPremixL = (mx>=0)? (127.0 - mx) : 127.0;
  nMotPremixR = (mx>=0)? 127.0 : (127.0 + mx);
}

// Scale Drive output due to Joystick Y input (throttle)
nMotPremixL = nMotPremixL * my/128.0;
nMotPremixR = nMotPremixR * my/128.0;

// Now calculate pivot amount
// - Strength of pivot (nPivSpeed) based on Joystick X input
// - Blending of pivot vs drive (fPivScale) based on Joystick Y input
nPivSpeed = mx;
fPivScale = (abs(my)>fPivYLimit)? 0.0 : (1.0 - abs(my)/fPivYLimit);

// Calculate final mix of Drive and Pivot
nMotMixL = (1.0-fPivScale)*nMotPremixL + fPivScale*( nPivSpeed);
nMotMixR = (1.0-fPivScale)*nMotPremixR + fPivScale*(-nPivSpeed);

printf("scale : %3.3f , MotMixL : %d MotMixR : %d", fPivScale, nMotMixL, nMotMixR);
#endif
}

// sets up the led for pwm
static void led_duty(uint16_t duty) {
  static uint16_t val;
  static uint16_t max = (1L<<10)-1;
  if(duty > 100) return;
  val = (duty * max) / 100;
  ledc_set_duty(ledc_channel.speed_mode,ledc_channel.channel,val);
  ledc_update_duty(ledc_channel.speed_mode,ledc_channel.channel);
}

static void led_setup() {
  const static char* TAG = "led_setup";

  ledc_timer_config_t ledc_timer = {
    .duty_resolution = LEDC_TIMER_10_BIT,
    .freq_hz         = 5000,
    .speed_mode      = LEDC_HIGH_SPEED_MODE,
    .timer_num       = LEDC_TIMER_0
  };

  ledc_channel.channel = LEDC_CHANNEL_0;
  ledc_channel.duty = 0;
  ledc_channel.gpio_num = LED_PIN,
  ledc_channel.speed_mode = LEDC_HIGH_SPEED_MODE;
  ledc_channel.timer_sel = LEDC_TIMER_0;

  ledc_timer_config(&ledc_timer);
  ledc_channel_config(&ledc_channel);
  led_duty(0);
  ESP_LOGI(TAG,"led is off and ready, 10 bits");
}

// handles websocket events
void websocket_callback(uint8_t num,WEBSOCKET_TYPE_t type,char* msg,uint64_t len) {
  const static char* TAG = "websocket_callback";
  int value;

  switch(type) {
    case WEBSOCKET_CONNECT:
      ESP_LOGI(TAG,"client %i connected!",num);
      break;
    case WEBSOCKET_DISCONNECT_EXTERNAL:
      ESP_LOGI(TAG,"client %i sent a disconnect message",num);
      led_duty(0);
      break;
    case WEBSOCKET_DISCONNECT_INTERNAL:
      ESP_LOGI(TAG,"client %i was disconnected",num);
      break;
    case WEBSOCKET_DISCONNECT_ERROR:
      ESP_LOGI(TAG,"client %i was disconnected due to an error",num);
      led_duty(0);
      break;
    case WEBSOCKET_TEXT:
      if(len) { // if the message length was greater than zero
        int mx, my;
        Robot_Move m;
        if(sscanf(msg,"%hhd %hhd",&m.mx, &m.my)) 
        {
          xQueueSendToBack(queue_moves, &m, WEBSOCKET_SERVER_QUEUE_TIMEOUT);
        }
        //       ESP_LOGI(TAG,"LED value: %i",value);
        //       led_duty(value);
        //       ws_server_send_text_all_from_callback(msg,len); // broadcast it!
        //     }
       

        // switch(msg[0]) {
        //   case 'L':
        //     if(sscanf(msg,"L%i",&value)) {
        //       ESP_LOGI(TAG,"LED value: %i",value);
        //       led_duty(value);
        //       ws_server_send_text_all_from_callback(msg,len); // broadcast it!
        //     }
        //     break;
        //   case 'M':
        //     ESP_LOGI(TAG, "got message length %i: %s", (int)len-1, &(msg[1]));
        //     break;
        //   default:
	      //     ESP_LOGI(TAG, "got an unknown message with length %i", (int)len);
	      //     break;
        // }
      }
      break;
    case WEBSOCKET_BIN:
      ESP_LOGI(TAG,"client %i sent binary message of size %i:\n%s",num,(uint32_t)len,msg);
      break;
    case WEBSOCKET_PING:
      ESP_LOGI(TAG,"client %i pinged us with message of size %i:\n%s",num,(uint32_t)len,msg);
      break;
    case WEBSOCKET_PONG:
      ESP_LOGI(TAG,"client %i responded to the ping",num);
      break;
  }
}

// serves any clients
static void http_serve(struct netconn *conn) {
  const static char* TAG = "http_server";
  const static char HTML_HEADER[] = "HTTP/1.1 200 OK\nContent-type: text/html\n\n";
  const static char ERROR_HEADER[] = "HTTP/1.1 404 Not Found\nContent-type: text/html\n\n";
  const static char JS_HEADER[] = "HTTP/1.1 200 OK\nContent-type: text/javascript\n\n";
  const static char CSS_HEADER[] = "HTTP/1.1 200 OK\nContent-type: text/css\n\n";
  //const static char PNG_HEADER[] = "HTTP/1.1 200 OK\nContent-type: image/png\n\n";
  const static char ICO_HEADER[] = "HTTP/1.1 200 OK\nContent-type: image/x-icon\n\n";
  //const static char PDF_HEADER[] = "HTTP/1.1 200 OK\nContent-type: application/pdf\n\n";
  //const static char EVENT_HEADER[] = "HTTP/1.1 200 OK\nContent-Type: text/event-stream\nCache-Control: no-cache\nretry: 3000\n\n";
  struct netbuf* inbuf;
  static char* buf;
  static uint16_t buflen;
  static err_t err;

  // default page
  extern const uint8_t root_html_start[] asm("_binary_root_html_start");
  extern const uint8_t root_html_end[] asm("_binary_root_html_end");
  const uint32_t root_html_len = root_html_end - root_html_start;

  // test.js
  extern const uint8_t test_js_start[] asm("_binary_test_js_start");
  extern const uint8_t test_js_end[] asm("_binary_test_js_end");
  const uint32_t test_js_len = test_js_end - test_js_start;

  // test.css
  extern const uint8_t test_css_start[] asm("_binary_test_css_start");
  extern const uint8_t test_css_end[] asm("_binary_test_css_end");
  const uint32_t test_css_len = test_css_end - test_css_start;

  // favicon.ico
  extern const uint8_t favicon_ico_start[] asm("_binary_favicon_ico_start");
  extern const uint8_t favicon_ico_end[] asm("_binary_favicon_ico_end");
  const uint32_t favicon_ico_len = favicon_ico_end - favicon_ico_start;

  // error page
  extern const uint8_t error_html_start[] asm("_binary_error_html_start");
  extern const uint8_t error_html_end[] asm("_binary_error_html_end");
  const uint32_t error_html_len = error_html_end - error_html_start;

  netconn_set_recvtimeout(conn,1000); // allow a connection timeout of 1 second
  ESP_LOGI(TAG,"reading from client...");
  err = netconn_recv(conn, &inbuf);
  ESP_LOGI(TAG,"read from client");
  if(err==ERR_OK) {
    netbuf_data(inbuf, (void**)&buf, &buflen);
    if(buf) {

      // default page
      if     (strcasestr(buf,"GET / ")
          && !strcasestr(buf,"upgrade: websocket")) {
        ESP_LOGI(TAG,"Sending /");
        netconn_write(conn, HTML_HEADER, sizeof(HTML_HEADER)-1,NETCONN_NOCOPY);
        netconn_write(conn, root_html_start,root_html_len,NETCONN_NOCOPY);
        netconn_close(conn);
        netconn_delete(conn);
        netbuf_delete(inbuf);
      }

      // default page websocket
      else if(strcasestr(buf,"GET / ")
           && strcasestr(buf,"upgrade: websocket")) {
        ESP_LOGI(TAG,"Requesting websocket on /");
        ws_server_add_client(conn,buf,buflen,"/",websocket_callback);
        netbuf_delete(inbuf);
      }

      else if(strstr(buf,"GET /test.js ")) {
        ESP_LOGI(TAG,"Sending /test.js");
        netconn_write(conn, JS_HEADER, sizeof(JS_HEADER)-1,NETCONN_NOCOPY);
        netconn_write(conn, test_js_start, test_js_len,NETCONN_NOCOPY);
        netconn_close(conn);
        netconn_delete(conn);
        netbuf_delete(inbuf);
      }

      else if(strstr(buf,"GET /test.css ")) {
        ESP_LOGI(TAG,"Sending /test.css");
        netconn_write(conn, CSS_HEADER, sizeof(CSS_HEADER)-1,NETCONN_NOCOPY);
        netconn_write(conn, test_css_start, test_css_len,NETCONN_NOCOPY);
        netconn_close(conn);
        netconn_delete(conn);
        netbuf_delete(inbuf);
      }

      else if(strstr(buf,"GET /favicon.ico ")) {
        ESP_LOGI(TAG,"Sending favicon.ico");
        netconn_write(conn,ICO_HEADER,sizeof(ICO_HEADER)-1,NETCONN_NOCOPY);
        netconn_write(conn,favicon_ico_start,favicon_ico_len,NETCONN_NOCOPY);
        netconn_close(conn);
        netconn_delete(conn);
        netbuf_delete(inbuf);
      }

      else if(strstr(buf,"GET /")) {
        ESP_LOGI(TAG,"Unknown request, sending error page: %s",buf);
        netconn_write(conn, ERROR_HEADER, sizeof(ERROR_HEADER)-1,NETCONN_NOCOPY);
        netconn_write(conn, error_html_start, error_html_len,NETCONN_NOCOPY);
        netconn_close(conn);
        netconn_delete(conn);
        netbuf_delete(inbuf);
      }

      else {
        ESP_LOGI(TAG,"Unknown request");
        netconn_close(conn);
        netconn_delete(conn);
        netbuf_delete(inbuf);
      }
    }
    else {
      ESP_LOGI(TAG,"Unknown request (empty?...)");
      netconn_close(conn);
      netconn_delete(conn);
      netbuf_delete(inbuf);
    }
  }
  else { // if err==ERR_OK
    ESP_LOGI(TAG,"error on read, closing connection");
    netconn_close(conn);
    netconn_delete(conn);
    netbuf_delete(inbuf);
  }
}

// handles clients when they first connect. passes to a queue
static void server_task(void* pvParameters) {
  const static char* TAG = "server_task";
  struct netconn *conn, *newconn;
  static err_t err;
  client_queue = xQueueCreate(client_queue_size,sizeof(struct netconn*));

  conn = netconn_new(NETCONN_TCP);
  netconn_bind(conn,NULL,80);
  netconn_listen(conn);
  ESP_LOGI(TAG,"server listening");
  do {
    err = netconn_accept(conn, &newconn);
    ESP_LOGI(TAG,"new client");
    if(err == ERR_OK) {
      xQueueSendToBack(client_queue,&newconn,portMAX_DELAY);
      //http_serve(newconn);
    }
  } while(err == ERR_OK);
  netconn_close(conn);
  netconn_delete(conn);
  ESP_LOGE(TAG,"task ending, rebooting board");
  esp_restart();
}

// receives clients from queue, handles them
static void server_handle_task(void* pvParameters) {
  const static char* TAG = "server_handle_task";
  struct netconn* conn;
  ESP_LOGI(TAG,"task starting");
  for(;;) {
    xQueueReceive(client_queue,&conn,portMAX_DELAY);
    if(!conn) continue;
    http_serve(conn);
  }
  vTaskDelete(NULL);
}

static void count_task(void* pvParameters) {
  const static char* TAG = "count_task";
  char out[20];
  int len;
  int clients;
  const static char* word = "%i";
  uint8_t n = 0;
  const int DELAY = 1000 / portTICK_PERIOD_MS; // 1 second

  ESP_LOGI(TAG,"starting task");
  for(;;) {
    len = sprintf(out,word,n);
    clients = ws_server_send_text_all(out,len);
    if(clients > 0) {
      ESP_LOGI(TAG,"sent: \"%s\" to %i clients",out,clients);
    }
    n++;
    vTaskDelay(DELAY);
  }
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

    //Robot_Action action;
    Robot_Move move;
    int ret;
    while(1)
    {
        // ret = xQueueReceive(queue_actions ,&action,(TickType_t )(1000 / portTICK_PERIOD_MS));
        // if (ret)
        // {
        //     ESP_LOGI(TAG, "Action received : %s %d", action_type_to_str(action.type), action.duration);
        //     switch(action.type)
        //     {
        //     case ROBOT_ACTION_FORWARD:
        //         forward(action.duration);
        //         stop(1000);
        //         break;
        //     case ROBOT_ACTION_BACKWARD:
        //         backward(action.duration);
        //         stop(1000);
        //         break;
        //     case ROBOT_ACTION_LEFT:
        //         turn_left(action.duration);
        //         stop(1000);
        //         break;
        //     case ROBOT_ACTION_RIGHT:
        //         turn_right(action.duration);
        //         stop(1000);
        //         break;
        //     default:
        //         break;
        //     }
        // }
        ret = xQueueReceive(queue_moves ,&move,(TickType_t )(1000 / portTICK_PERIOD_MS));
        if (ret)
        {
            ESP_LOGI(TAG, "MOVE received : %d %d", move.mx, move.my);
            robot_move(move.mx, move.my);
        }
    }

}


void app_main()
{
    queue_actions = xQueueCreate(1, sizeof(Robot_Action));
    queue_moves = xQueueCreate(8, sizeof(Robot_Move));
    
    nvs_flash_init();
    start_wifi();
	
    led_setup();
  	ws_server_start();
	 
    ESP_LOGI(TAG, "Launch motor thread");
    xTaskCreate(mcpwm_example_brushed_motor_control, "motor", 8192, NULL, 5, NULL);
    ESP_LOGI(TAG, "Launch socket server");
 	  xTaskCreate(&server_task,"server_task",3000,NULL,9,NULL);
  	xTaskCreate(&server_handle_task,"server_handle_task",4000,NULL,6,NULL);
    xTaskCreate(&robot_task, "robot", 8192, NULL, 5, NULL);

}
