#include "pico/stdlib.h"
#include "include/connection.h"
#include "include/mfrc522_handler.h"
#include "include/mfrc522.h"
#include "include/led.h"
#include "include/button.h"

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"

#define WIFI_SSID "Moto Edge 50 Neo"
#define WIFI_PASSWORD "ebd82500"

#define BTNB 6
#define BTNA 5
#define GPIO_DEBOUNCE 200
#define QTN_BUTTONS 2


QueueHandle_t gpio_queue;
QueueHandle_t mfrc_queue;
MFRC522Ptr_t mfrc;
button buttons[2];


void http_task(void *pvParameters)
{
    for(;;)
    {
        cyw43_arch_poll();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}


void gpio_callback(uint gpio, uint32_t events)
{
    printf("GPIO CALLBACK - gpio: %d\n", gpio);
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xQueueSendFromISR(gpio_queue, &gpio, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}


void read_card_task(void* pvParameters) {
    MFRC522Ptr_t mfrc = (MFRC522Ptr_t) pvParameters;
    // card_event card;
    // card.card_uid[0] = '\0';
    char* card = NULL;

    for (;;)
    {
        if (check_flag(FLAG_REPORT_TRAVEL) || check_flag(FLAG_REPORT_STOP))
        {
            int response = read_card(&card, mfrc);
            printf("Cartão lido - card_uid: %s\n", card);
            switch (response)
            {
                case NOTHING:
                    printf("NOTHING\n");
                    led_set_color(AMARELO);
                    vTaskDelay(pdMS_TO_TICKS(50));
                    break;

                case SUCCESS:
                    printf("SUCCESS");
                    led_set_color(VERDE);
                    vTaskDelay(pdMS_TO_TICKS(1000));
                    xQueueSend(mfrc_queue, &card, 0);
                    break;

                case FAILED:
                    printf("FAILED\n");
                    led_set_color(VERMELHO);            
                    vTaskDelay(pdMS_TO_TICKS(1000));   
                    break;
                
                default:
                    break;
            }
        }
        else
        {
            led_set_color(DESLIGADO);
            vTaskDelay(pdMS_TO_TICKS(50));
        }
    }
}


void send_card_http(void* pvParameters)
{
    card_event card;

    for (;;)
    {
        if (xQueueReceive(mfrc_queue, &card, portMAX_DELAY))
        {
            printf("Cartão capturado da fila. Fazendo requisição\n");
            int res = post("api.meuservidor.com", "/data","{\"temp\": 23.5}");
            

            if (res == HTTP_SUCCESS) printf("Requisição efetuada.\n");
            else printf("Falha na requisição.\n");
        }
    }
}


void button_task(void* pvParameters)
{
    uint32_t gpio;
    
    for (;;)
    {
        if (xQueueReceive(gpio_queue, &gpio, portMAX_DELAY) == pdPASS)
        {
            TickType_t current_time = xTaskGetTickCount();

            for (int i = 0; i < QTN_BUTTONS; i++)
            {
                if (buttons[i].gpio == gpio)
                {
                    if ((current_time - buttons[i].last_tick) > pdMS_TO_TICKS(GPIO_DEBOUNCE))
                    {
                        printf("Botão %d pressionado\n", gpio);
                        buttons[i].last_tick = current_time;
                        buttons[i].intrr_func();    
                    }
                }
            }
        }
    }
}


int main() {
    stdio_init_all();
    
    // while (!connect_wifi(WIFI_SSID, WIFI_PASSWORD));
    printf("iniciando\n");
    sleep_ms(2000);
    
    gpio_queue = xQueueCreate(2, sizeof(uint32_t));
    mfrc_queue = xQueueCreate(31, sizeof(card_event));
    

    init_led(11);
    init_led(12);
    init_led(13);
    
    buttons[0] = init_button(BTNA, GPIO_DEBOUNCE, toggle_read_report_stop);
    buttons[1] = init_button(BTNB, GPIO_DEBOUNCE, toggle_read_report_travel);
    

    mfrc = MFRC522_Init();
    PCD_Init(mfrc, spi0);


    gpio_set_irq_enabled_with_callback(BTNA, GPIO_IRQ_EDGE_RISE, true, gpio_callback);
    gpio_set_irq_enabled_with_callback(BTNB, GPIO_IRQ_EDGE_RISE, true, gpio_callback);

    led_set_color(DESLIGADO);

    xTaskCreate(button_task, "button_task", 256, NULL, tskIDLE_PRIORITY + 2, NULL);
    xTaskCreate(read_card_task, "read_card_task", 2048, mfrc, tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(send_card_http, "send_card_http", 1024, NULL, tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(http_task, "http_task", 256, NULL, tskIDLE_PRIORITY + 0, NULL);

    printf("iniciado\n");

    vTaskStartScheduler();

    while (true) {};
}