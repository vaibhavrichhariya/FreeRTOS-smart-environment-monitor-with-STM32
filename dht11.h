#ifndef DHT11_H
#define DHT11_H

#include "stm32f103.h"
#include "system_init.h"

/* ─── DHT11 on PA1 ─────────────────────────────────── */

static void dht11_output_mode(void) {
    /* PA1 → output push-pull 50MHz */
    GPIOA->CRL &= ~(0xFU << 4);
    GPIOA->CRL |=  (0x3U << 4);
}

static void dht11_input_mode(void) {
    /* PA1 → floating input */
    GPIOA->CRL &= ~(0xFU << 4);
    GPIOA->CRL |=  (0x4U << 4);
}

/* ─── DHT11 Read ───────────────────────────────────── */
/*  Returns 1 on success, 0 on error                    */
/*  temp and hum are integer values (DHT11 no decimal)  */
uint8_t dht11_read(uint8_t *temperature, uint8_t *humidity) {
    uint8_t data[5] = {0, 0, 0, 0, 0};
    uint32_t timeout;

    /* ── START SIGNAL ─────────────────────────────── */
    dht11_output_mode();
    pin_low(GPIOA, DHT11_PIN);
    delay_ms(18);                    /* pull low min 18ms */
    pin_high(GPIOA, DHT11_PIN);
    delay_us(30);                    /* pull high 20-40us */

    /* ── SWITCH TO INPUT ──────────────────────────── */
    dht11_input_mode();

    /* ── WAIT FOR DHT11 RESPONSE LOW ─────────────── */
    timeout = 10000;
    while (pin_read(GPIOA, DHT11_PIN) && timeout--);
    if (timeout == 0) return 0;

    /* ── WAIT FOR DHT11 RESPONSE HIGH ────────────── */
    timeout = 10000;
    while (!pin_read(GPIOA, DHT11_PIN) && timeout--);
    if (timeout == 0) return 0;

    /* ── WAIT FOR DATA START ──────────────────────── */
    timeout = 10000;
    while (pin_read(GPIOA, DHT11_PIN) && timeout--);
    if (timeout == 0) return 0;

    /* ── READ 40 BITS ─────────────────────────────── */
    for (int i = 0; i < 5; i++) {
        for (int j = 7; j >= 0; j--) {
            /* wait for bit HIGH */
            timeout = 10000;
            while (!pin_read(GPIOA, DHT11_PIN) && timeout--);
            if (timeout == 0) return 0;

            /* measure HIGH duration */
            delay_us(40);

            /* if still HIGH after 40us → bit is 1 */
            if (pin_read(GPIOA, DHT11_PIN)) {
                data[i] |= (1U << j);
                /* wait for LOW */
                timeout = 10000;
                while (pin_read(GPIOA, DHT11_PIN) && timeout--);
            }
        }
    }

    /* ── CHECKSUM ─────────────────────────────────── */
    if (data[4] != ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) {
        return 0;  /* checksum fail */
    }

    *humidity    = data[0];   /* integer part */
    *temperature = data[2];   /* integer part */
    return 1;
}

#endif /* DHT11_H */
