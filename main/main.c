#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "QMI8658.h"

// Define I2C configuration macros
#define I2C_MASTER_SCL_IO    7      // GPIO number for I2C SCL
#define I2C_MASTER_SDA_IO    6      // GPIO number for I2C SDA
#define I2C_MASTER_NUM       I2C_NUM_0  // I2C port number
#define I2C_MASTER_FREQ_HZ   400000  // I2C master clock frequency

static const char *TAG = "QMI8658";

// Task to read and print sensor data
void sensor_task(void *pvParameters) {
    int count = 0;
    while (1) {
        float acc[3], gyro[3];
        
        // Read accelerometer and gyroscope data
        QMI8658_read_xyz(acc, gyro, NULL);

        // Print accelerometer values (in mg)
        ESP_LOGI(TAG, "Accelerometer (mg): X: %.2f, Y: %.2f, Z: %.2f", 
                acc[0], acc[1], acc[2]);

        // Print gyroscope values (in dps)
        ESP_LOGI(TAG, "Gyroscope (dps): X: %.2f, Y: %.2f, Z: %.2f", 
                gyro[0], gyro[1], gyro[2]);

        ESP_LOGI(TAG, "Count= %d", ++count);

        // Delay for 100ms 
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

extern "C" void app_main(void) {
    // Initialize I2C
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA_IO;
    conf.scl_io_num = I2C_MASTER_SCL_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;

    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);

    // Initialize sensor
    if (QMI8658_init()) {
        ESP_LOGI(TAG, "Sensor initialized successfully");
    } else {
        ESP_LOGE(TAG, "Sensor initialization failed");
        return;
    }

    // Create a FreeRTOS task to read and print sensor data
    xTaskCreate(sensor_task, "sensor_task", 4096, NULL, 5, NULL);
}
