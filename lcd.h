#ifndef LCD_H
#define LCD_H

#include "stm32f103.h"
#include "system_init.h"

/*
 * LCD 16x2 — 4-bit mode
 * RS  → PB0
 * EN  → PB1
 * D4  → PB4
 * D5  → PB5
 * D6  → PB6
 * D7  → PB7
 */

/* ─── Send nibble (high 4 bits of data) ──────────── */
static void lcd_send_nibble(uint8_t nibble) {
    /* Write D4-D7 bits */
    if (nibble & 0x01) pin_high(GPIOB, LCD_D4); else pin_low(GPIOB, LCD_D4);
    if (nibble & 0x02) pin_high(GPIOB, LCD_D5); else pin_low(GPIOB, LCD_D5);
    if (nibble & 0x04) pin_high(GPIOB, LCD_D6); else pin_low(GPIOB, LCD_D6);
    if (nibble & 0x08) pin_high(GPIOB, LCD_D7); else pin_low(GPIOB, LCD_D7);

    /* Pulse EN high → low */
    pin_high(GPIOB, LCD_EN);
    delay_us(1);
    pin_low(GPIOB, LCD_EN);
    delay_us(50);
}

/* ─── Send command byte ───────────────────────────── */
static void lcd_cmd(uint8_t cmd) {
    pin_low(GPIOB, LCD_RS);        /* RS=0 for command */
    lcd_send_nibble(cmd >> 4);     /* send high nibble */
    lcd_send_nibble(cmd & 0x0F);   /* send low nibble  */
    delay_us(50);
}

/* ─── Send data byte (character) ──────────────────── */
static void lcd_data(uint8_t data) {
    pin_high(GPIOB, LCD_RS);       /* RS=1 for data */
    lcd_send_nibble(data >> 4);
    lcd_send_nibble(data & 0x0F);
    delay_us(50);
}

/* ─── Initialize LCD ──────────────────────────────── */
void lcd_init(void) {
    /* All LCD pins LOW initially */
    pin_low(GPIOB, LCD_RS | LCD_EN | LCD_D4 | LCD_D5 | LCD_D6 | LCD_D7);
    delay_ms(50);

    /* Init sequence: force 8-bit mode 3 times */
    pin_low(GPIOB, LCD_RS);
    lcd_send_nibble(0x03); delay_ms(5);
    lcd_send_nibble(0x03); delay_ms(1);
    lcd_send_nibble(0x03); delay_us(150);

    /* Switch to 4-bit mode */
    lcd_send_nibble(0x02);
    delay_us(150);

    /* Function set: 4-bit, 2 lines, 5x8 dots */
    lcd_cmd(0x28);
    /* Display ON, cursor OFF, blink OFF */
    lcd_cmd(0x0C);
    /* Entry mode: increment, no shift */
    lcd_cmd(0x06);
    /* Clear display */
    lcd_cmd(0x01);
    delay_ms(2);
}

/* ─── Set cursor position ─────────────────────────── */
void lcd_set_cursor(uint8_t row, uint8_t col) {
    uint8_t pos = (row == 0) ? (0x80 + col) : (0xC0 + col);
    lcd_cmd(pos);
}

/* ─── Print string ────────────────────────────────── */
void lcd_print(const char *str) {
    while (*str) {
        lcd_data((uint8_t)(*str));
        str++;
    }
}

/* ─── Clear display ───────────────────────────────── */
void lcd_clear(void) {
    lcd_cmd(0x01);
    delay_ms(2);
}

/* ─── Print integer ───────────────────────────────── */
void lcd_print_int(int32_t num) {
    char buf[12];
    int i = 0;
    uint8_t neg = 0;

    if (num < 0) { neg = 1; num = -num; }
    if (num == 0) { lcd_data('0'); return; }

    while (num > 0) {
        buf[i++] = '0' + (num % 10);
        num /= 10;
    }
    if (neg) buf[i++] = '-';

    /* reverse */
    for (int j = i - 1; j >= 0; j--) {
        lcd_data((uint8_t)buf[j]);
    }
}

#endif /* LCD_H */
