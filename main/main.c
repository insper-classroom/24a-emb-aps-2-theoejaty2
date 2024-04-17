#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>

#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/adc.h"

#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "mpu6050.h"
#include "pico/stdlib.h"

#include <stdlib.h>
#include <Fusion.h>
#include <math.h>  // Inclua esta biblioteca para usar fabs
#include "hc06.h"

// batida - Z
// smash - X

const int MPU_ADDRESS = 0x68;
const int I2C_SDA_GPIO = 12;
const int I2C_SCL_GPIO = 13;
const uint ADC_PIN_X = 26; // GPIO 26, que é o canal ADC 0
const uint ADC_PIN_Y = 27; // GPIO 27, que é o canal ADC 1

#define SAMPLE_PERIOD (0.01f) // Definindo o período de amostra

QueueHandle_t xQueueAdc;  // Agora vai armazenar dados do tipo char

static void mpu6050_reset() {
    uint8_t buf[] = {0x6B, 0x00}; // Sair do modo de dormir
    i2c_write_blocking(i2c_default, MPU_ADDRESS, buf, 2, false);
}

static void mpu6050_read_raw(int16_t accel[3], int16_t gyro[3]) {
    uint8_t buffer[14];
    uint8_t val = 0x3B;
    i2c_write_blocking(i2c_default, MPU_ADDRESS, &val, 1, true);
    i2c_read_blocking(i2c_default, MPU_ADDRESS, buffer, 14, false);

    for (int i = 0; i < 3; i++) {
        accel[i] = (buffer[i * 2] << 8 | buffer[(i * 2) + 1]);
        gyro[i] = (buffer[(i * 2) + 8] << 8 | buffer[(i * 2) + 9]);
    }
}

void mpu6050_task(void *p) {
    i2c_init(i2c_default, 400 * 1000);
    gpio_set_function(I2C_SDA_GPIO, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_GPIO, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_GPIO);
    gpio_pull_up(I2C_SCL_GPIO);

    mpu6050_reset();
    FusionAhrs ahrs;
    FusionAhrsInitialise(&ahrs);

    int16_t acceleration[3], gyro[3];

    while(1) {
        mpu6050_read_raw(acceleration, gyro);

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

        FusionEuler euler = FusionQuaternionToEuler(FusionAhrsGetQuaternion(&ahrs));

        // printf("Accel x: %0.2f g, y: %0.2f g, z: %0.2f g\n", accelerometer.axis.x, accelerometer.axis.y, accelerometer.axis.z);
        // printf("Gyro x: %0.2f deg/s, y: %0.2f deg/s, z: %0.2f deg/s\n", gyroscope.axis.x, gyroscope.axis.y, gyroscope.axis.z);


        if ((gyroscope.axis.x) < -190.0f) {
            // printf("SMASH\n");
            char smash = 'v';  // player 1daada
            xQueueSend(xQueueAdc, &smash, portMAX_DELAY);
        }
        if (fabs(gyroscope.axis.z) > 225.0f) {
            // printf("BATIDA\n");
            char batida = 'b'; // player 1
            xQueueSend(xQueueAdc, &batida, portMAX_DELAY);
        }

        vTaskDelay(pdMS_TO_TICKS(50)); // Simulação do período de amostra
    }
}

void x_task(void *params) {
    while (1) {
        adc_select_input(0);  // Seleciona o canal ADC para o eixo X (GP26 = ADC0)
        int adc_value = adc_read();  // Lê o valor do ADC
        int processed_value = (adc_value - 2047) / 8;  // Processa o valor para o formato desejado
        
        // Lógica para decidir qual tecla pressionar
        if (processed_value > 30) {
            char key = 'd';  // Enviar 'd' para a fila
            xQueueSend(xQueueAdc, &key, portMAX_DELAY);
        } else if (processed_value < -30) {
            char key = 'a';  // Enviar 'a' para a fila
            xQueueSend(xQueueAdc, &key, portMAX_DELAY);
        }
        else {
            char key = 'n';  // Enviar 'a' para a fila
            xQueueSend(xQueueAdc, &key, portMAX_DELAY);
        }


        // printf("X ADC Value: %d\n", processed_value);  // Imprime o valor processado
        vTaskDelay(pdMS_TO_TICKS(50));  // Adiciona um delay para não sobrecarregar a saída
    }
}



void y_task(void *params) {
    while (1) {
        adc_select_input(1);  // Seleciona o canal ADC para o eixo Y (GP27 = ADC1)
        int adc_value = adc_read();  // Lê o valor do ADC
        int processed_value = (adc_value - 2047) / 8;  // Processa o valor para o formato desejado
        
        if (processed_value < -100) {
            char key = 'w';  // Enviar 'w' para a fila se o valor do ADC for maior que 30
            xQueueSend(xQueueAdc, &key, portMAX_DELAY);
        }

        vTaskDelay(pdMS_TO_TICKS(50));  // Delay para evitar sobrecarga
    }
}

void write_package(char data) {
    uart_putc_raw(uart0, data);  // Envia o caractere recebido
    uart_putc_raw(uart0, '\n');  // Nova linha para separar comandos (opcional)
}

void uart_task(void *params) {
    char command;

    while (1) {
        if (xQueueReceive(xQueueAdc, &command, portMAX_DELAY)) {
            write_package(command);  // Envia o caractere usando write_package
        }
    }
}
void hc06_task(void *p) {
    uart_init(HC06_UART_ID, HC06_BAUD_RATE);
    gpio_set_function(HC06_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(HC06_RX_PIN, GPIO_FUNC_UART);
    hc06_init("tennis_legends", "1111");

    char command;
    while (true) {
        if (xQueueReceive(xQueueAdc, &command, portMAX_DELAY)) {
            char buffer[2] = {command, '\n'};
            uart_puts(HC06_UART_ID, buffer);  // Sending the command over Bluetooth
        }
    }
}


int main() {
    stdio_init_all();
    adc_init(); 
    adc_gpio_init(ADC_PIN_X);
    adc_gpio_init(ADC_PIN_Y);

    xTaskCreate(hc06_task, "Bluetooth_Task", 9600, NULL, 1, NULL);
    xTaskCreate(x_task, "x_task", 256, NULL, 1, NULL);
    xTaskCreate(y_task, "y_task", 256, NULL, 1, NULL);
    xTaskCreate(mpu6050_task, "mpu6050_Task", 8192, NULL, 1, NULL);
    //xTaskCreate(uart_task, "uart_task", 8192, NULL, 1, NULL);

    xQueueAdc = xQueueCreate(10, sizeof(char));  // Cria a fila com espaço para 10 caracteres


    vTaskStartScheduler();

    while (true)
        ;
}