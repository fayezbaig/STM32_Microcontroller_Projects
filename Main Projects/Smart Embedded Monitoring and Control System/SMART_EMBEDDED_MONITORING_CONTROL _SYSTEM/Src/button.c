/*
 * button.c - push button on PA7 with EXTI interrupt + debounce
 *
 * A mechanical button "bounces": one press produces several edges within
 * a few milliseconds. The original code toggled the mode on every edge,
 * so a press often toggled twice (no visible change). Here an edge is
 * only accepted if the previous accepted one is >= BUTTON_DEBOUNCE_MS old.
 */
#include "button.h"
#include "gpio.h"
#include "systick.h"
#include "stm32f4_reg.h"

#define BUTTON_PIN   7U

static volatile uint8_t  s_pressed;
static volatile uint32_t s_last_ms;

void button_init(void)
{
    gpio_clock_enable(GPIOA);
    gpio_config(GPIOA, BUTTON_PIN, GPIO_MODE_INPUT, GPIO_OTYPE_PUSHPULL,
                GPIO_SPEED_LOW, GPIO_PULL_UP, 0);

    /* Route PA7 to EXTI line 7: EXTICR2 (index 1), bits [15:12], port A = 0 */
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    SYSCFG->EXTICR[1] &= ~(0xFUL << 12);

    EXTI->FTSR |=  (1UL << BUTTON_PIN);   /* falling edge = button pressed */
    EXTI->RTSR &= ~(1UL << BUTTON_PIN);
    EXTI->PR    =  (1UL << BUTTON_PIN);   /* clear anything pending */
    EXTI->IMR  |=  (1UL << BUTTON_PIN);   /* unmask */

    s_last_ms = millis();
    nvic_enable_irq(EXTI9_5_IRQn);
}

uint8_t button_take_press(void)
{
    if (s_pressed) {
        s_pressed = 0;
        return 1U;
    }
    return 0U;
}

/* Overrides the weak handler in startup_stm32f4.c.
 * EXTI lines 5..9 share this IRQ, so check line 7 explicitly. */
void EXTI9_5_IRQHandler(void)
{
    if (EXTI->PR & (1UL << BUTTON_PIN)) {
        EXTI->PR = (1UL << BUTTON_PIN);            /* write 1 to clear */

        const uint32_t now = millis();
        if ((uint32_t)(now - s_last_ms) >= BUTTON_DEBOUNCE_MS) {
            s_last_ms = now;
            s_pressed = 1;
        }
    }
}
