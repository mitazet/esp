#ifndef SENSOR_H
#define SENSOR_H

#include "driver/i2c.h"

// Software Reset 
extern esp_err_t reset_sensor();

// Init i2c communication
extern void init_sensor();

// Read distancee from sensor
extern esp_err_t read_distance(int *distance);

#endif
