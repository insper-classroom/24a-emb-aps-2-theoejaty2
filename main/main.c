/*
 * LED blink with FreeRTOS
 */
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>

#include <string.h>

#include "pico/stdlib.h"
#include <stdio.h>

// #include "hc06.h"
#include "mpu6050.h"
#include <Fusion.h>

// roll y
// yaw x

#define SAMPLE_PERIOD (0.01f) // Definindo o período de amostra

const int MPU_ADDRESS = 0x68;
const int I2C_SDA_GPIO = 4;
const int I2C_SCL_GPIO = 5;

QueueHandle_t xQueueAdc;

typedef struct {
    int axis;
    int val;
} adc_reading_t;

// void hc06_task(void *p) {
//     uart_init(HC06_UART_ID, HC06_BAUD_RATE);
//     gpio_set_function(HC06_TX_PIN, GPIO_FUNC_UART);
//     gpio_set_function(HC06_RX_PIN, GPIO_FUNC_UART);
//     hc06_init("tennis_legends", "1111");

//     while (true) {
//         uart_puts(HC06_UART_ID, "OLAAA ");
//         vTaskDelay(pdMS_TO_TICKS(100));
//     }
// }

static void mpu6050_setup() {
    uint8_t buf[] = {0x6B, 0x00}; // Configuração para tirar o sensor do modo sleep
    i2c_write_blocking(i2c_default, MPU_ADDRESS, buf, 2, false);

    // Adicione aqui outras configurações necessárias após o reset, como configuração da escala
    // de giroscópio e acelerômetro se necessário.
}

static void mpu6050_read_raw(int16_t accel[3], int16_t gyro[3], int16_t *temp) {
    // For this particular device, we send the device the register we want to read
    // first, then subsequently read from the device. The register is auto incrementing
    // so we don't need to keep sending the register we want, just the first.

    uint8_t buffer[6];

    // Start reading acceleration registers from register 0x3B for 6 bytes
    uint8_t val = 0x3B;
    i2c_write_blocking(i2c_default, MPU_ADDRESS, &val, 1, true); // true to keep master control of bus
    i2c_read_blocking(i2c_default, MPU_ADDRESS, buffer, 6, false);

    for (int i = 0; i < 3; i++) {
        accel[i] = (buffer[i * 2] << 8 | buffer[(i * 2) + 1]);
    }

    // Now gyro data from reg 0x43 for 6 bytes
    // The register is auto incrementing on each read
    val = 0x43;
    i2c_write_blocking(i2c_default, MPU_ADDRESS, &val, 1, true);
    i2c_read_blocking(i2c_default, MPU_ADDRESS, buffer, 6, false);  // False - finished with bus

    for (int i = 0; i < 3; i++) {
        gyro[i] = (buffer[i * 2] << 8 | buffer[(i * 2) + 1]);;
    }

    // Now temperature from reg 0x41 for 2 bytes
    // The register is auto incrementing on each read
    val = 0x41;
    i2c_write_blocking(i2c_default, MPU_ADDRESS, &val, 1, true);
    i2c_read_blocking(i2c_default, MPU_ADDRESS, buffer, 2, false);  // False - finished with bus

    *temp = buffer[0] << 8 | buffer[1];
}

void mpu6050_task(void *p) {
    i2c_init(i2c_default, 400 * 1000);
    gpio_set_function(I2C_SDA_GPIO, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_GPIO, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_GPIO);
    gpio_pull_up(I2C_SCL_GPIO);

    mpu6050_setup(); // Substituído pelo método de setup que inclui tirar do modo sleep
    FusionAhrs ahrs;
    FusionAhrsInitialise(&ahrs);

    int16_t acceleration[3], gyro[3], temp;

    while(1) {
        
     mpu6050_read_raw(acceleration, gyro, &temp);

        FusionVector gyroscope = {
        .axis.x = gyro[0] / 131.0f, // Conversão para graus/s
        .axis.y = gyro[1] / 131.0f,
        .axis.z = gyro[2] / 131.0f,
        };

        FusionVector accelerometer = {
            .axis.x = acceleration[0] / 16384.0f, // Conversão para g
            .axis.y = acceleration[1] / 16384.0f,
            .axis.z = acceleration[2] / 16384.0f,
        }; 

      
        FusionAhrsUpdateNoMagnetometer(&ahrs, gyroscope, accelerometer, SAMPLE_PERIOD);

        const FusionEuler euler = FusionQuaternionToEuler(FusionAhrsGetQuaternion(&ahrs));

        // printf("Roll %0.1f, Pitch %0.1f, Yaw %0.1f\n", euler.angle.roll, euler.angle.pitch, (euler.angle.yaw));

        adc_reading_t x_reading = {.axis = 0, .val = -(int)(euler.angle.yaw)};

        xQueueSend(xQueueAdc, &x_reading, portMAX_DELAY);  // Envia a leitura para a fila
        vTaskDelay(pdMS_TO_TICKS(10));  // Atraso para desacoplamento das tarefas

        adc_reading_t y_reading = {.axis = 1, .val = -(int)euler.angle.roll};

        xQueueSend(xQueueAdc, &y_reading, portMAX_DELAY);  // Envia a leitura para a fila
        vTaskDelay(pdMS_TO_TICKS(10));  // Atraso para desacoplamento das tarefas

    vTaskDelay(pdMS_TO_TICKS(10));
    }
}


void uart_task(void *params) {
    adc_reading_t reading;
    
    while (1) {
        if (xQueueReceive(xQueueAdc, &reading, portMAX_DELAY)) {
            write_package(reading);
        }
    }
}

void write_package(adc_reading_t data) {
    int val = data.val;
    int msb = val >> 8;
    int lsb = val & 0xFF ;

    uart_putc_raw(uart0, data.axis);
    uart_putc_raw(uart0, lsb);
    uart_putc_raw(uart0, msb);
    uart_putc_raw(uart0, -1);
}


int main() {
    stdio_init_all();

    printf("Start bluetooth task\n");

    xTaskCreate(mpu6050_task, "mpu6050_Task", 8192, NULL, 1, NULL);
    xTaskCreate(uart_task, "uart_task", 256, NULL, 1, NULL);
    // xTaskCreate(hc06_task, "UART_Task 1", 4096, NULL, 1, NULL);

    vTaskStartScheduler();

    while (true)
        ;
}
