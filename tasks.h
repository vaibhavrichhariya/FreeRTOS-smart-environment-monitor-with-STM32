#ifndef TASKS_H
#define TASKS_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "stm32f103.h"
#include "system_init.h"
#include "dht11.h"
#include "lcd.h"

/* ─── Shared Sensor Data Structure ───────────────── */
typedef struct {
    uint8_t  temperature;    /* °C  from DHT11 */
    uint8_t  humidity;       /* %RH from DHT11 */
    uint16_t ldr_value;      /* 0-4095 ADC raw  */
    uint16_t gas_value;      /* 0-4095 ADC raw  */
    uint8_t  dht_ok;         /* 1=valid reading */
} SensorData_t;

/* ─── Global Queue & Mutex ────────────────────────── */
QueueHandle_t  xSensorQueue;
SemaphoreHandle_t xLCDMutex;

/* ─── Thresholds (tune as needed) ─────────────────── */
#define TEMP_THRESHOLD    30U    /* °C  → relay ON  */
#define LDR_THRESHOLD     1000U  /* raw → LED ON (dark) */
#define GAS_THRESHOLD     2000U  /* raw → buzzer ON */

/* ══════════════════════════════════════════════════
   TASK 1 — DHT11 Sensor Task
   Priority: 2  |  Period: 2000ms
   ══════════════════════════════════════════════════ */
void Task_DHT11(void *pvParameters) {
    SensorData_t data;
    (void)pvParameters;

    for (;;) {
        uint8_t temp = 0, hum = 0;
        data.dht_ok = dht11_read(&temp, &hum);

        if (data.dht_ok) {
            data.temperature = temp;
            data.humidity    = hum;
        } else {
            data.temperature = 0;
            data.humidity    = 0;
        }

        /* Keep ldr/gas from previous reading using peek */
        SensorData_t prev;
        if (xQueuePeek(xSensorQueue, &prev, 0) == pdTRUE) {
            data.ldr_value = prev.ldr_value;
            data.gas_value = prev.gas_value;
        }

        /* Overwrite queue with latest */
        xQueueOverwrite(xSensorQueue, &data);

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

/* ══════════════════════════════════════════════════
   TASK 2 — ADC Task (LDR + GAS/POT)
   Priority: 2  |  Period: 300ms
   ══════════════════════════════════════════════════ */
void Task_ADC(void *pvParameters) {
    SensorData_t data;
    (void)pvParameters;

    for (;;) {
        /* Read LDR → ADC CH0 (PA0) */
        uint16_t ldr = adc_read(0);

        /* Read GAS/POT → ADC CH4 (PA4) */
        uint16_t gas = adc_read(4);

        /* Merge with existing DHT11 data */
        if (xQueuePeek(xSensorQueue, &data, pdMS_TO_TICKS(10)) == pdTRUE) {
            data.ldr_value = ldr;
            data.gas_value = gas;
            xQueueOverwrite(xSensorQueue, &data);
        } else {
            /* Queue empty — init struct */
            data.ldr_value   = ldr;
            data.gas_value   = gas;
            data.temperature = 0;
            data.humidity    = 0;
            data.dht_ok      = 0;
            xQueueOverwrite(xSensorQueue, &data);
        }

        vTaskDelay(pdMS_TO_TICKS(300));
    }
}

/* ══════════════════════════════════════════════════
   TASK 3 — LCD Display Task
   Priority: 1  |  Period: 1000ms
   ══════════════════════════════════════════════════ */
void Task_Display(void *pvParameters) {
    SensorData_t data;
    (void)pvParameters;

    lcd_init();
    lcd_clear();

    /* Startup message */
    lcd_set_cursor(0, 0); lcd_print("Smart Env Mon");
    lcd_set_cursor(1, 0); lcd_print("Initializing..");
    vTaskDelay(pdMS_TO_TICKS(2000));
    lcd_clear();

    for (;;) {
        if (xQueuePeek(xSensorQueue, &data, pdMS_TO_TICKS(100)) == pdTRUE) {

            if (xSemaphoreTake(xLCDMutex, pdMS_TO_TICKS(50)) == pdTRUE) {

                /* ── Row 0: Temperature & Humidity ───── */
                lcd_set_cursor(0, 0);
                lcd_print("T:");
                if (data.dht_ok) {
                    lcd_print_int(data.temperature);
                    lcd_print("C H:");
                    lcd_print_int(data.humidity);
                    lcd_print("%  ");
                } else {
                    lcd_print("--C H:--%");
                }

                /* ── Row 1: LDR & Gas ─────────────────── */
                lcd_set_cursor(1, 0);
                lcd_print("L:");
                lcd_print_int(data.ldr_value);
                lcd_print(" G:");
                lcd_print_int(data.gas_value);
                lcd_print("   ");

                xSemaphoreGive(xLCDMutex);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/* ══════════════════════════════════════════════════
   TASK 4 — Control Task (Relay, LED, Buzzer)
   Priority: 3 (highest)  |  Period: 200ms
   ══════════════════════════════════════════════════ */
void Task_Control(void *pvParameters) {
    SensorData_t data;
    (void)pvParameters;

    for (;;) {
        if (xQueuePeek(xSensorQueue, &data, pdMS_TO_TICKS(50)) == pdTRUE) {

            /* ── Fan/Relay: ON if temp > threshold ──── */
            if (data.dht_ok && data.temperature > TEMP_THRESHOLD) {
                pin_high(GPIOC, RELAY_PIN);
            } else {
                pin_low(GPIOC, RELAY_PIN);
            }

            /* ── LED: ON if dark (LDR < threshold) ──── */
            if (data.ldr_value < LDR_THRESHOLD) {
                pin_high(GPIOC, LED_PIN);
            } else {
                pin_low(GPIOC, LED_PIN);
            }

            /* ── Buzzer: ON if gas > threshold ──────── */
            if (data.gas_value > GAS_THRESHOLD) {
                pin_high(GPIOC, BUZZER_PIN);
            } else {
                pin_low(GPIOC, BUZZER_PIN);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

/* ─── Create all tasks and queue ─────────────────── */
void tasks_create(void) {
    /* Queue: 1 item (always overwritten with latest data) */
    xSensorQueue = xQueueCreate(1, sizeof(SensorData_t));

    /* LCD Mutex */
    xLCDMutex = xSemaphoreCreateMutex();

    /* Create tasks */
    xTaskCreate(Task_DHT11,   "DHT11",   128, NULL, 2, NULL);
    xTaskCreate(Task_ADC,     "ADC",     128, NULL, 2, NULL);
    xTaskCreate(Task_Display, "Display", 256, NULL, 1, NULL);
    xTaskCreate(Task_Control, "Control", 128, NULL, 3, NULL);
}

#endif /* TASKS_H */
