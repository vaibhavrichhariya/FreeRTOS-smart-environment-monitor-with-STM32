#ifndef STM32F103_H
#define STM32F103_H

#include <stdint.h>

/* ─── Base Addresses ─────────────────────────────── */
#define PERIPH_BASE       0x40000000UL
#define APB1_BASE         (PERIPH_BASE + 0x00000000UL)
#define APB2_BASE         (PERIPH_BASE + 0x00010000UL)
#define AHB_BASE          (PERIPH_BASE + 0x00018000UL)

/* ─── RCC ────────────────────────────────────────── */
#define RCC_BASE          (AHB_BASE + 0x09000UL)
typedef struct {
    volatile uint32_t CR, CFGR, CIR, APB2RSTR, APB1RSTR;
    volatile uint32_t AHBENR, APB2ENR, APB1ENR;
    volatile uint32_t BDCR, CSR;
} RCC_TypeDef;
#define RCC               ((RCC_TypeDef *)RCC_BASE)

/* ─── GPIO ────────────────────────────────────────── */
#define GPIOA_BASE        (APB2_BASE + 0x0800UL)
#define GPIOB_BASE        (APB2_BASE + 0x0C00UL)
#define GPIOC_BASE        (APB2_BASE + 0x1000UL)
typedef struct {
    volatile uint32_t CRL, CRH, IDR, ODR, BSRR, BRR, LCKR;
} GPIO_TypeDef;
#define GPIOA             ((GPIO_TypeDef *)GPIOA_BASE)
#define GPIOB             ((GPIO_TypeDef *)GPIOB_BASE)
#define GPIOC             ((GPIO_TypeDef *)GPIOC_BASE)

/* ─── ADC1 ────────────────────────────────────────── */
#define ADC1_BASE         (APB2_BASE + 0x2400UL)
typedef struct {
    volatile uint32_t SR, CR1, CR2, SMPR1, SMPR2;
    volatile uint32_t JOFR1, JOFR2, JOFR3, JOFR4;
    volatile uint32_t HTR, LTR, SQR1, SQR2, SQR3;
    volatile uint32_t JSQR, JDR1, JDR2, JDR3, JDR4, DR;
} ADC_TypeDef;
#define ADC1              ((ADC_TypeDef *)ADC1_BASE)

/* ─── TIM2 (microsecond timer) ───────────────────── */
#define TIM2_BASE         (APB1_BASE + 0x0000UL)
typedef struct {
    volatile uint32_t CR1, CR2, SMCR, DIER, SR, EGR;
    volatile uint32_t CCMR1, CCMR2, CCER, CNT, PSC, ARR;
    volatile uint32_t RCR, CCR1, CCR2, CCR3, CCR4, DCR, DMAR;
} TIM_TypeDef;
#define TIM2              ((TIM_TypeDef *)TIM2_BASE)

/* ─── SysTick ─────────────────────────────────────── */
#define SYSTICK_BASE      0xE000E010UL
typedef struct {
    volatile uint32_t CTRL, LOAD, VAL, CALIB;
} SysTick_TypeDef;
#define SYSTICK           ((SysTick_TypeDef *)SYSTICK_BASE)

/* ─── Bit helpers ─────────────────────────────────── */
#define SET_BIT(REG, BIT)    ((REG) |=  (BIT))
#define CLR_BIT(REG, BIT)    ((REG) &= ~(BIT))
#define READ_BIT(REG, BIT)   ((REG) &   (BIT))

/* ─── RCC Enable bits ─────────────────────────────── */
#define RCC_APB2ENR_IOPAEN   (1U << 2)
#define RCC_APB2ENR_IOPBEN   (1U << 3)
#define RCC_APB2ENR_IOPCEN   (1U << 4)
#define RCC_APB2ENR_ADC1EN   (1U << 9)
#define RCC_APB1ENR_TIM2EN   (1U << 0)

/* ─── GPIO Mode values for CRL/CRH ───────────────── */
/* CNF[1:0] MODE[1:0] per 4-bit nibble                */
#define GPIO_OUT_PP_2MHZ     0x2U   /* 0010 push-pull out 2MHz */
#define GPIO_OUT_PP_50MHZ    0x3U   /* 0011 push-pull out 50MHz*/
#define GPIO_IN_FLOAT        0x4U   /* 0100 floating input     */
#define GPIO_IN_ANALOG       0x0U   /* 0000 analog input       */

/* ─── ADC SR bits ─────────────────────────────────── */
#define ADC_SR_EOC           (1U << 1)

/* ─── ADC CR2 bits ────────────────────────────────── */
#define ADC_CR2_ADON         (1U << 0)
#define ADC_CR2_CAL          (1U << 2)
#define ADC_CR2_RSTCAL       (1U << 3)
#define ADC_CR2_SWSTART      (1U << 22)
#define ADC_CR2_EXTTRIG      (1U << 20)
#define ADC_CR2_EXTSEL       (7U << 17)   /* 111 = SWSTART */

/* ─── SysTick bits ────────────────────────────────── */
#define SYSTICK_CTRL_ENABLE  (1U << 0)
#define SYSTICK_CTRL_TICKINT (1U << 1)
#define SYSTICK_CTRL_CLKSRC  (1U << 2)

/* ─── Pin shortcuts ───────────────────────────────── */
/* PA0 = LDR, PA4 = GAS(POT), PA1 = DHT11             */
#define DHT11_PIN            (1U << 1)   /* PA1 */
#define LDR_PIN              (1U << 0)   /* PA0 */
#define GAS_PIN              (1U << 4)   /* PA4 */

/* GPIOB: LCD */
#define LCD_RS               (1U << 0)   /* PB0 */
#define LCD_EN               (1U << 1)   /* PB1 */
#define LCD_D4               (1U << 4)   /* PB4 */
#define LCD_D5               (1U << 5)   /* PB5 */
#define LCD_D6               (1U << 6)   /* PB6 */
#define LCD_D7               (1U << 7)   /* PB7 */

/* GPIOC: Actuators */
#define RELAY_PIN            (1U << 13)  /* PC13 - Fan */
#define LED_PIN              (1U << 14)  /* PC14 - Light */
#define BUZZER_PIN           (1U << 15)  /* PC15 - Gas alert */

#endif /* STM32F103_H */
