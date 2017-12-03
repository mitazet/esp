#include "http_request.h"
#include "sensor.h"
#include "nvs_flash.h"
#include <time.h>
#include <sys/time.h>
#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_wifi.h"
#include "ntptime.h"

#define EMPTY_DISTANCE  30
#define CHECK_HOUR      20

static const char *TAG = "main";

esp_err_t monitor(bool *need_check)
{
    esp_err_t ret;

    ret = reset_sensor();
    if(ret != ESP_OK){
        ESP_LOGI(TAG, "Sensor reset failed!");
        return ret;
    }

    int distance;
    ret = read_distance(&distance);
    if (ret != ESP_OK) {
        ESP_LOGI(TAG, "data_l:No ack, sensor not connected...skip...");
        return ret;
    }

    if(distance >  EMPTY_DISTANCE){
        ESP_LOGI(TAG, "not declutter...");
        http_post_task();
    }else{
        ESP_LOGI(TAG, "decluttered");
        *need_check = false;
    }

    return ESP_OK;
}

void app_main()
{
    ESP_ERROR_CHECK( nvs_flash_init() );
    init_sensor();
    initialise_wifi();

    time_t now;
    struct tm timeinfo;
    char strftime_buf[64];

    ESP_LOGI(TAG, "Time is not set yet. Connecting to WiFi and getting time over NTP.");
    obtain_time();
    // update 'now' variable with current time
    time(&now);
    
    time_t t = now + (3600 * 9);
    localtime_r(&t, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG, "The current date/time in JST-2 is: %s", strftime_buf);

    bool need_check = false;
    int check_start_hour = CHECK_HOUR;
    int check_start_day = timeinfo.tm_mday;
    
    while(true){
         //get time
        time(&now);
        t = now + (3600 * 9);
        localtime_r(&t, &timeinfo);
        strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
        ESP_LOGI(TAG, "The current date/time in JST-2 is: %s", strftime_buf);

        if(timeinfo.tm_mday == check_start_day && timeinfo.tm_hour >= check_start_hour){ 
            need_check = true;
            check_start_day++;
        }

        if(need_check==true){
            esp_err_t ret = monitor(&need_check);
            if(ret != ESP_OK){
                ESP_LOGI(TAG, "Device error occured!!");
                return;
            }
        }

        //sleep 1 minute
        esp_sleep_enable_timer_wakeup(1000000LL * 60);
        esp_light_sleep_start();
    }
}
