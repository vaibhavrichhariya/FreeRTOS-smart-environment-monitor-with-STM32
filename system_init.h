#ifndef SYSTEM_INIT_H
#define SYSTEM_INIT_H

#include "stm32f103.h"

/* ─── SysTick ms counter ──────────────────────────── */
volatile uint32_t sys_tick_ms = 0;

/* SysTick IRQ Handler */
void SysTick_Handler(void) {
    sys_tick_ms++;
}

/* ─── Delay functions ─────────────────────────────── */
void delay_ms(uint32_t ms) {
    uint32_t start = sys_tick_ms;
    while ((sys_tick_ms - start) < ms);
}

/* Microsecond delay using TIM2 counter */
void delay_us(uint32_t us) {
    TIM2->CNT = 0;
    while (TIM2->CNT < us);
}

/* ─── Clock Init (72MHz via PLL + HSE 8MHz) ──────── */
void clock_init(void) {
    /* Enable HSE */
    SET_BIT(RCC->CR, (1U << 16));
    while (!READ_BIT(RCC->CR, (1U << 17)));  /* wait HSERDY */

    /* Flash latency = 2 wait states for 72MHz */
    /* FLASH->ACR: set LATENCY=010 */
    *((volatile uint32_t *)0x40022000) = 0x32;

    /* PLL: HSE/1 * 9 = 72MHz */
    /* PLLSRC=1(HSE), PLLMUL=0111(x9), APB1=/2, APB2=/1 */
    RCC->CFGR = (1U << 16) |   /* PLLSRC = HSE */
                (7U << 18) |   /* PLLMUL = x9  */
                (4U << 8)  |   /* APB1 = /2    */
                (0U << 11);    /* APB2 = /1    */

    /* Enable PLL */
    SET_BIT(RCC->CR, (1U << 24));
    while (!READ_BIT(RCC->CR, (1U << 25)));  /* wait PLLRDY */

    /* Switch to PLL */
    RCC->CFGR |= (2U << 0);
    while ((RCC->CFGR & (3U << 2)) != (2U << 2));  /* SWS=PLL */
}

/* ─── SysTick Init (1ms tick at 72MHz) ───────────── */
void systick_init(void) {
    SYSTICK->LOAD = 72000 - 1;   /* 72MHz / 72000 = 1kHz = 1ms */
    SYSTICK->VAL  = 0;
    SYSTICK->CTRL = SYSTICK_CTRL_ENABLE |
                    SYSTICK_CTRL_TICKINT |
                    SYSTICK_CTRL_CLKSRC;
}

/* ─── TIM2 Init (microsecond counter) ────────────── */
void tim2_init(void) {
    SET_BIT(RCC->APB1ENR, RCC_APB1ENR_TIM2EN);
    TIM2->PSC = 71;          /* 72MHz / (71+1) = 1MHz → 1us tick */
    TIM2->ARR = 0xFFFF;
    TIM2->CR1 = (1U << 0);  /* CEN */
}

/* ─── GPIO Pin Config helpers ─────────────────────── */
/* Set pin as output push-pull 50MHz in CRL (pins 0-7) */
static inline void gpio_set_output(GPIO_TypeDef *port, uint8_t pin) {
    if (pin < 8) {
        port->CRL &= ~(0xFU << (pin * 4));
        port->CRL |=  (0x3U << (pin * 4));  /* OUT PP 50MHz */
    } else {
        pin -= 8;
        port->CRH &= ~(0xFU << (pin * 4));
        port->CRH |=  (0x3U << (pin * 4));
    }
}

/* Set pin as floating input in CRL (pins 0-7) */
static inline void gpio_set_input(GPIO_TypeDef *port, uint8_t pin) {
    if (pin < 8) {
        port->CRL &= ~(0xFU << (pin * 4));
        port->CRL |=  (0x4U << (pin * 4));  /* IN FLOAT */
    } else {
        pin -= 8;
        port->CRH &= ~(0xFU << (pin * 4));
        port->CRH |=  (0x4U << (pin * 4));
    }
}

/* Set pin as analog input */
static inline void gpio_set_analog(GPIO_TypeDef *port, uint8_t pin) {
    if (pin < 8) {
        port->CRL &= ~(0xFU << (pin * 4));  /* 0000 = analog */
    } else {
        pin -= 8;
        port->CRH &= ~(0xFU << (pin * 4));
    }
}

/* ─── GPIO Init ────────────────────────────────────── */
void gpio_init(void) {
    /* Enable clocks */
    SET_BIT(RCC->APB2ENR, RCC_APB2ENR_IOPAEN |
                           RCC_APB2ENR_IOPBEN |
                           RCC_APB2ENR_IOPCEN);

    /* PA0 = LDR (analog), PA4 = GAS/POT (analog), PA1 = DHT11 (input start) */
    gpio_set_analog(GPIOA, 0);   /* PA0 - LDR */
    gpio_set_analog(GPIOA, 4);   /* PA4 - GAS */
    gpio_set_input(GPIOA,  1);   /* PA1 - DHT11 (toggled in driver) */

    /* PB0=RS, PB1=EN, PB4-7=D4-D7 → LCD outputs */
    gpio_set_output(GPIOB, 0);
    gpio_set_output(GPIOB, 1);
    gpio_set_output(GPIOB, 4);
    gpio_set_output(GPIOB, 5);
    gpio_set_output(GPIOB, 6);
    gpio_set_output(GPIOB, 7);

    /* PC13=Relay, PC14=LED, PC15=Buzzer */
    gpio_set_output(GPIOC, 13);
    gpio_set_output(GPIOC, 14);
    gpio_set_output(GPIOC, 15);

    /* All actuators OFF initially */
    GPIOC->ODR &= ~(RELAY_PIN | LED_PIN | BUZZER_PIN);
}

/* ─── ADC1 Init ───────────────────────────────────── */
void adc_init(void) {
    SET_BIT(RCC->APB2ENR, RCC_APB2ENR_ADC1EN);

    /* Software trigger, right align */
    ADC1->CR2 = ADC_CR2_EXTSEL | ADC_CR2_EXTTRIG;

    /* Sample time = 239.5 cycles for both channels (SMPR2) */
    /* CH0: bits [2:0], CH4: bits [14:12] */
    ADC1->SMPR2 = (7U << 0) | (7U << 12);  /* 111 = 239.5 cycles */

    /* 1 conversion */
    ADC1->SQR1 = 0;

    /* Power ON ADC */
    SET_BIT(ADC1->CR2, ADC_CR2_ADON);
    delay_ms(2);  /* stabilize */

    /* Reset calibration */
    SET_BIT(ADC1->CR2, ADC_CR2_RSTCAL);
    while (READ_BIT(ADC1->CR2, ADC_CR2_RSTCAL));

    /* Calibrate */
    SET_BIT(ADC1->CR2, ADC_CR2_CAL);
    while (READ_BIT(ADC1->CR2, ADC_CR2_CAL));
}

/* ─── ADC Read single channel ─────────────────────── */
uint16_t adc_read(uint8_t channel) {
    /* Select channel */
    ADC1->SQR3 = channel & 0x1F;

    /* Start conversion */
    SET_BIT(ADC1->CR2, ADC_CR2_ADON);
    SET_BIT(ADC1->CR2, ADC_CR2_SWSTART);

    /* Wait for EOC */
    while (!READ_BIT(ADC1->SR, ADC_SR_EOC));

    return (uint16_t)(ADC1->DR & 0x0FFF);
}

/* ─── GPIO pin write ───────────────────────────────── */
static inline void pin_high(GPIO_TypeDef *port, uint32_t pin_mask) {
    port->BSRR = pin_mask;
}
static inline void pin_low(GPIO_TypeDef *port, uint32_t pin_mask) {
    port->BRR = pin_mask;
}
static inline uint8_t pin_read(GPIO_TypeDef *port, uint32_t pin_mask) {
    return (port->IDR & pin_mask) ? 1 : 0;
}

#endif /* SYSTEM_INIT_H */
