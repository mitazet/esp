#include "driver/i2c.h"

extern esp_err_t reset_sensor();
extern void init_sensor();
extern esp_err_t read_distance(int *distance);
