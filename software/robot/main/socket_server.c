#include <lwip/sockets.h>
#include <esp_log.h>
#include <string.h>
#include <errno.h>
#include "sdkconfig.h"
#include "common.h"
#include "utils.h"

#define PORT_NUMBER 8001

static char tag[] = "socket_server";

static const char* actions_str[]= {
    "forward",
    "backward",
    "left",
    "right",
    NULL
};

ROBOT_ACTION_TYPE str_to_action_type(char *action)
{
    int i;
    for (i = 0; actions_str[i]; i++)
    {
        if (!strcmp(action, actions_str[i]))
        {
            return i;
        }
    }
    return ROBOT_ACTION_UNKNOWN;
};

const char *action_type_to_str(ROBOT_ACTION_TYPE type)
{
    return actions_str[type];
}

void socket_server(void *ignore) {
	struct sockaddr_in clientAddress;
	struct sockaddr_in serverAddress;

    ESP_LOGI(tag, "Start socket server");

	// Create a socket that we will listen upon.
	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock < 0) {
		ESP_LOGE(tag, "socket: %d %s", sock, strerror(errno));
		goto END;
	}

	// Bind our server socket to a port.
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port = htons(PORT_NUMBER);
	int rc  = bind(sock, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
	if (rc < 0) {
		ESP_LOGE(tag, "bind: %d %s", rc, strerror(errno));
		goto END;
	}

	// Flag the socket as listening for new connections.
	rc = listen(sock, 5);
	if (rc < 0) {
		ESP_LOGE(tag, "listen: %d %s", rc, strerror(errno));
		goto END;
	}

	while (1) {
		// Listen for a new client connection.
		socklen_t clientAddressLength = sizeof(clientAddress);
		int clientSock = accept(sock, (struct sockaddr *)&clientAddress, &clientAddressLength);
		if (clientSock < 0) {
			ESP_LOGE(tag, "accept: %d %s", clientSock, strerror(errno));
			goto END;
		}

		// We now have a new client ...
		int total =	2*1024;
		int sizeUsed = 0;
		char *data = malloc(total);
        ESP_LOGI(tag, "New client");
		// Loop reading data.

        while(1)
          {
            ssize_t sizeRead = recv(clientSock, data, total, 0);
            if (sizeRead < 0) {
              ESP_LOGE(tag, "recv: %d %s", sizeRead, strerror(errno));
              break;
            }

            Robot_Action action;
            char **arr;
            arr = str_split(data, " ", 0);

            action.type = str_to_action_type(arr[0]);
            action.duration = atoi(arr[1]);
            xQueueSend(queue,(void *)&action,(TickType_t )0); // add the counter value to the queue
            free(arr[0]);
            free(arr);
            // Finished reading data.
            ESP_LOGI(tag, "Data read (size: %d) was: %.*s", sizeRead, sizeRead, data);
          }
        free(data);
		close(clientSock);
	}
	END:
	vTaskDelete(NULL);
}
