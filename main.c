#include "pico/stdlib.h"
#include "src/connection.h"

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#define WIFI_SSID "SSID"
#define WIFI_PASSWORD "PASSWORD"

// CONNECTION TASKS
void http_task(void *pvParameters);


// PROCESS CARDS TASKS



int main()
{
    stdio_init_all();
    sleep_ms(2500);
    
    connect_wifi(WIFI_SSID, WIFI_PASSWORD);

    get("192.168.18.115", "/get");

    post("192.168.18.115", "/post", "{\"device\": \"pico\", \"value\": 42}");

    put("192.168.18.115", "/put", "{\"device\": \"pico\", \"value\": 42}");
    

    xTaskCreate(http_task, "HTTP Task", 1024, NULL, 1, NULL);
    vTaskStartScheduler();

    while (true) {}
}


void http_task(void *pvParameters)
{
    for(;;)
    {
        cyw43_arch_poll();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}