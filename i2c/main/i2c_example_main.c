/* i2c - Example

   For other examples please check:
   https://github.com/espressif/esp-idf/tree/master/examples

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include "driver/i2c.h"

#define I2C_MASTER_SCL_IO           19  /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO           18  /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM I2C_NUM_1        /*!< I2C port number for master dev */
#define I2C_MASTER_TX_BUF_DISABLE   0   /*!< I2C master do not need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0   /*!< I2C master do not need buffer */
#define I2C_MASTER_FREQ_HZ          100000  /*!< I2C master clock frequency */

#define GP2Y0E03_SLAVE_ADDR         0x80/*!< slave address for GP2Y0E03 sensor */
#define GP2Y0E03_REG_SHIFT_BIT      0x35/*!< Command to set Maximum Display */
#define GP2Y0E03_REG_DISTANCE_HIGH  0x5E/*!< Command to read Distance Value */
#define GP2Y0E03_REG_DISTANCE_LOW   0x5F/*!< Command to read Distance Value */
#define GP2Y0E03_REG_SOFTWARE_RESET 0xEE/*!< Command to execute software reset */

#define GP2Y0E03_VAL_SOFTWARE_RESET 0x06/*!< Vaule of software reset*/

#define WRITE_BIT  I2C_MASTER_WRITE     /*!< I2C master write */
#define READ_BIT   I2C_MASTER_READ      /*!< I2C master read */
#define ACK_CHECK_EN                0x1 /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS               0x0 /*!< I2C master will not check ack from slave */
#define ACK_VAL                     0x0 /*!< I2C ack value */
#define NACK_VAL                    0x1 /*!< I2C nack value */

/**
 * @brief code to read esp-i2c-slave
 * _______________________________________________________________________________________
 * | start | slave_addr + rd_bit +ack | read n-1 bytes + ack | read 1 byte + nack | stop |
 * --------|--------------------------|----------------------|--------------------|------|
 */
static esp_err_t i2c_master_read_slave(i2c_port_t i2c_num, uint8_t* data_rd, size_t size)
{
    if(size == 0){
        return ESP_OK;
    }
    
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, GP2Y0E03_SLAVE_ADDR | READ_BIT, ACK_CHECK_EN);
    if(size > 1){
        i2c_master_read(cmd, data_rd, size - 1, ACK_VAL);
    }
    i2c_master_read_byte(cmd, data_rd + size - 1, NACK_VAL);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    
    return ret;
}
     
/**
 * @brief code to write esp-i2c-slave
 * ___________________________________________________________________
 * | start | slave_addr + wr_bit + ack | write n bytes + ack  | stop |
 * --------|---------------------------|----------------------|------|
 */
static esp_err_t i2c_master_write_slave(i2c_port_t i2c_num, uint8_t* data_wr, size_t size)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, GP2Y0E03_SLAVE_ADDR | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write(cmd, data_wr, size, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    return ret;
}
          
static esp_err_t reset_gp2y0e03(void)
{
    esp_err_t   ret;
    uint8_t     data_soft_reset[] = {GP2Y0E03_REG_SOFTWARE_RESET, GP2Y0E03_VAL_SOFTWARE_RESET};
                    
    ret = i2c_master_write_slave(I2C_MASTER_NUM, data_soft_reset, sizeof(data_soft_reset));
                        
    return ret;
}
          
/**
 * @brief code to write esp-i2c-slave
 *
 * 1. set mode
 * _________________________________________________________________
 * | start | slave_addr + wr_bit + ack | write 1 byte + ack  | stop |
 * --------|---------------------------|---------------------|------|
 * 2. wait more than 24 ms
 * 3. read data
 * ______________________________________________________________________________________
 * | start | slave_addr + rd_bit + ack | read 1 byte + ack  | read 1 byte + nack | stop |
 * --------|---------------------------|--------------------|--------------------|------|
 */
static esp_err_t read_shift_bit(uint8_t* data)
{
    esp_err_t   ret;
    uint8_t     reg = GP2Y0E03_REG_SHIFT_BIT;

    ret = i2c_master_write_slave(I2C_MASTER_NUM, &reg,  sizeof(reg));
    if(ret == ESP_FAIL){
        return ret;
    }
    vTaskDelay(30 / portTICK_RATE_MS);
                                                                    
    ret = i2c_master_read_slave(I2C_MASTER_NUM, data, sizeof(uint8_t));

    return ret;
}

static esp_err_t read_distance_high(uint8_t* data)
{
    esp_err_t   ret;
    uint8_t     reg = GP2Y0E03_REG_DISTANCE_HIGH;

    ret = i2c_master_write_slave(I2C_MASTER_NUM, &reg,  sizeof(reg));
    if(ret == ESP_FAIL){
        return ret;
    }
    vTaskDelay(30 / portTICK_RATE_MS);
                                                                    
    ret = i2c_master_read_slave(I2C_MASTER_NUM, data, sizeof(uint8_t));
                                                                        
    return ret;
}
                      
static esp_err_t read_distance_low(uint8_t* data)
{
    esp_err_t   ret;
    uint8_t     reg = GP2Y0E03_REG_DISTANCE_LOW;

    ret = i2c_master_write_slave(I2C_MASTER_NUM, &reg,  sizeof(reg));
    if (ret == ESP_FAIL) {
        return ret;
    }
    vTaskDelay(30 / portTICK_RATE_MS);

    ret = i2c_master_read_slave(I2C_MASTER_NUM, data, sizeof(uint8_t));

    return ret;
}

/**
 * @brief i2c master initialization
 */
static void i2c_master_init()
{
    int i2c_master_port = I2C_MASTER_NUM;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = I2C_MASTER_SCL_IO;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
    i2c_param_config(i2c_master_port, &conf);
    i2c_driver_install(i2c_master_port, conf.mode,
                       I2C_MASTER_RX_BUF_DISABLE,
                       I2C_MASTER_TX_BUF_DISABLE, 0);
}
                        
static void i2c_test_task()
{
    int ret;
    uint8_t shift_bit, data_h, data_l;

    /* Software Rest */
    ret = reset_gp2y0e03();

    while (1) {
        ret = read_shift_bit(&shift_bit);
        if (ret != ESP_OK) {
            printf("shift_bit:No ack, sensor not connected...skip...\n");
            continue;
        }

        ret = read_distance_high(&data_h);
        if (ret != ESP_OK) {
            printf("data_h:No ack, sensor not connected...skip...\n");
            continue;
        }

        ret = read_distance_low(&data_l);
        if (ret != ESP_OK) {
            printf("data_l:No ack, sensor not connected...skip...\n");
            continue;
        }
        printf("distance: %d\n",( ( data_h << 4 | data_l ) / 16 ) >> shift_bit);
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}
                        
void app_main()
{
    i2c_master_init();

    xTaskCreate(i2c_test_task, "i2c_test_task_0", 1024 * 2, (void* ) 0, 10, NULL);
}
