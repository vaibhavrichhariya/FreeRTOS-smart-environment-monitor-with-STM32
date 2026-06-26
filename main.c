#include "FreeRTOS.h"
#include "task.h"

#include "stm32f103.h"
#include "system_init.h"
#include "tasks.h"

/* ─── FreeRTOS Hook Functions ─────────────────────── */
void vApplicationIdleHook(void) {
    /* MCU sleeps when idle — saves power */
    __asm volatile ("wfi");
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    (void)xTask;
    (void)pcTaskName;
    /* Hang here on stack overflow — debug via Proteus */
    while (1);
}

/* ─── Main ────────────────────────────────────────── */
int main(void) {

    /* 1. System clock → 72MHz */
    clock_init();

    /* 2. SysTick → 1ms */
    systick_init();

    /* 3. TIM2 → 1us counter for DHT11 */
    tim2_init();

    /* 4. GPIO → All pins configured */
    gpio_init();

    /* 5. ADC1 → Calibrated, ready */
    adc_init();

    /* 6. Create FreeRTOS tasks + queue */
    tasks_create();

    /* 7. Start FreeRTOS scheduler (never returns) */
    vTaskStartScheduler();

    /* Should never reach here */
    while (1);

    return 0;
}
