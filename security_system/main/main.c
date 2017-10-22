#include "http_request.h"
#include "sensor.h"
#include "nvs_flash.h"

void security_loop()
{
    esp_err_t ret;

    ret = reset_sensor();
    if(ret != ESP_OK){
        printf("Sensor reset failed!");
        return;
    }

    // polling every 100ms
    while(1){
        int distance;
        ret = read_distance(&distance);
        if (ret != ESP_OK) {
            printf("data_l:No ack, sensor not connected...skip...\n");
            continue;
        }

        if(distance < 40){
            http_post_task();
        }        
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void app_main()
{
    ESP_ERROR_CHECK( nvs_flash_init() );
    init_sensor();
    initialise_wifi();
    xTaskCreate(&security_loop, "security_loop", 4096, NULL, 5, NULL);
}
