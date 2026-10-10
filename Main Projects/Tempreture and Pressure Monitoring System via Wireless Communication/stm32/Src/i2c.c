/*
 * i2c.c - bare-metal, polled I2C1 master driver for STM32F401/F411
 *
 * Follows the master transmitter/receiver sequences of RM0368 section 18.3.3.
 */
#include "i2c.h"
#include "gpio.h"
#include "stm32f4_reg.h"

#define I2C_SCL_PIN     6U
#define I2C_SDA_PIN     7U
#define I2C_SPEED_HZ    400000UL
#define I2C_TIMEOUT     60000UL     /* loop iterations (~ several ms @16 MHz) */

/* ------------------------------------------------------------------ */
/* Helpers                                                             */
/* ------------------------------------------------------------------ */

static void short_delay(volatile uint32_t n)
{
    while (n--) { __asm volatile ("nop"); }
}

/* If a slave was interrupted mid-transfer (e.g. MCU reset) it can hold SDA
 * low forever. Clocking SCL up to 9 times and issuing a STOP frees it. */
static void i2c_bus_recover(void)
{
    gpio_config(GPIOB, I2C_SCL_PIN, GPIO_MODE_OUTPUT, GPIO_OTYPE_OPENDRAIN,
                GPIO_SPEED_HIGH, GPIO_PULL_UP, 0);
    gpio_config(GPIOB, I2C_SDA_PIN, GPIO_MODE_OUTPUT, GPIO_OTYPE_OPENDRAIN,
                GPIO_SPEED_HIGH, GPIO_PULL_UP, 0);
    gpio_write(GPIOB, I2C_SDA_PIN, 1);
    gpio_write(GPIOB, I2C_SCL_PIN, 1);
    short_delay(200);

    for (uint8_t i = 0; i < 9U && !gpio_read(GPIOB, I2C_SDA_PIN); i++) {
        gpio_write(GPIOB, I2C_SCL_PIN, 0);
        short_delay(200);
        gpio_write(GPIOB, I2C_SCL_PIN, 1);
        short_delay(200);
    }

    /* STOP condition: SDA low -> high while SCL high */
    gpio_write(GPIOB, I2C_SDA_PIN, 0);
    short_delay(200);
    gpio_write(GPIOB, I2C_SCL_PIN, 1);
    short_delay(200);
    gpio_write(GPIOB, I2C_SDA_PIN, 1);
    short_delay(200);
}

static void i2c_stop(void)
{
    I2C1->CR1 |= I2C_CR1_STOP;
}

/* Wait for a flag in SR1. Detects NACK and bus errors while waiting. */
static i2c_status_t wait_sr1(uint32_t flag)
{
    uint32_t t = I2C_TIMEOUT;
    while (!(I2C1->SR1 & flag)) {
        const uint32_t sr1 = I2C1->SR1;
        if (sr1 & I2C_SR1_AF) {
            I2C1->SR1 = ~I2C_SR1_AF;            /* rc_w0: write 0 to clear */
            i2c_stop();
            return I2C_ERR_NACK;
        }
        if (sr1 & (I2C_SR1_BERR | I2C_SR1_ARLO)) {
            I2C1->SR1 = ~(I2C_SR1_BERR | I2C_SR1_ARLO);
            i2c_stop();
            return I2C_ERR_BUS;
        }
        if (--t == 0U) {
            i2c_stop();
            return I2C_ERR_TIMEOUT;
        }
    }
    return I2C_OK;
}

static i2c_status_t wait_not_busy(void)
{
    uint32_t t = I2C_TIMEOUT;
    while (I2C1->SR2 & I2C_SR2_BUSY) {
        if (--t == 0U) return I2C_ERR_TIMEOUT;
    }
    return I2C_OK;
}

/* START + address phase. dir: 0 = write, 1 = read.
 * ADDR is NOT cleared here - the caller does it (needed for 1-byte reads). */
static i2c_status_t start_and_address(uint8_t addr7, uint8_t dir)
{
    i2c_status_t st;

    I2C1->CR1 |= I2C_CR1_START;
    st = wait_sr1(I2C_SR1_SB);
    if (st != I2C_OK) return st;

    I2C1->DR = (uint32_t)((addr7 << 1) | (dir & 1U));   /* reading SR1 + writing DR clears SB */
    return wait_sr1(I2C_SR1_ADDR);
}

static void clear_addr(void)
{
    volatile uint32_t tmp;
    tmp = I2C1->SR1;
    tmp = I2C1->SR2;
    (void)tmp;
}

static i2c_status_t send_byte(uint8_t b)
{
    i2c_status_t st = wait_sr1(I2C_SR1_TXE);
    if (st != I2C_OK) return st;
    I2C1->DR = b;
    return I2C_OK;
}

/* ------------------------------------------------------------------ */
/* Public API                                                          */
/* ------------------------------------------------------------------ */

void i2c1_init(void)
{
    gpio_clock_enable(GPIOB);
    i2c_bus_recover();

    /* PB6/PB7 -> AF4, open-drain. Internal pull-ups are weak (~40k);
     * most BMP280/OLED breakout boards already carry 4.7k-10k pull-ups. */
    gpio_config(GPIOB, I2C_SCL_PIN, GPIO_MODE_AF, GPIO_OTYPE_OPENDRAIN,
                GPIO_SPEED_HIGH, GPIO_PULL_UP, GPIO_AF4_I2C1);
    gpio_config(GPIOB, I2C_SDA_PIN, GPIO_MODE_AF, GPIO_OTYPE_OPENDRAIN,
                GPIO_SPEED_HIGH, GPIO_PULL_UP, GPIO_AF4_I2C1);

    RCC->APB1ENR  |= RCC_APB1ENR_I2C1EN;
    RCC->APB1RSTR |= RCC_APB1RSTR_I2C1RST;      /* clean peripheral reset */
    RCC->APB1RSTR &= ~RCC_APB1RSTR_I2C1RST;

    I2C1->CR1 = I2C_CR1_SWRST;
    I2C1->CR1 = 0;

    const uint32_t pclk_mhz = PCLK1_HZ / 1000000UL;
    I2C1->CR2 = pclk_mhz;                        /* FREQ[5:0] = APB1 clock in MHz */

    /* Fast mode, duty 2:1 -> f_scl = PCLK1 / (3 * CCR) */
    uint32_t ccr = PCLK1_HZ / (3UL * I2C_SPEED_HZ);
    if (PCLK1_HZ % (3UL * I2C_SPEED_HZ)) ccr++;  /* round up -> never exceed 400 kHz */
    if (ccr < 1U) ccr = 1U;
    I2C1->CCR = I2C_CCR_FS | ccr;

    /* Max rise time 300 ns in fast mode: TRISE = 300ns * f_pclk + 1 */
    I2C1->TRISE = (pclk_mhz * 300UL) / 1000UL + 1UL;

    I2C1->CR1 = I2C_CR1_PE;
}

i2c_status_t i2c_probe(uint8_t addr7)
{
    i2c_status_t st = wait_not_busy();
    if (st != I2C_OK) return st;

    st = start_and_address(addr7, 0);
    if (st != I2C_OK) return st;
    clear_addr();
    i2c_stop();
    return I2C_OK;
}

i2c_status_t i2c_write(uint8_t addr7, const uint8_t *data, uint16_t len)
{
    i2c_status_t st = wait_not_busy();
    if (st != I2C_OK) return st;

    st = start_and_address(addr7, 0);
    if (st != I2C_OK) return st;
    clear_addr();

    for (uint16_t i = 0; i < len; i++) {
        st = send_byte(data[i]);
        if (st != I2C_OK) return st;
    }

    st = wait_sr1(I2C_SR1_BTF);                 /* last byte fully shifted out */
    if (st != I2C_OK) return st;
    i2c_stop();
    return I2C_OK;
}

i2c_status_t i2c_write_reg(uint8_t addr7, uint8_t reg, const uint8_t *data, uint16_t len)
{
    i2c_status_t st = wait_not_busy();
    if (st != I2C_OK) return st;

    st = start_and_address(addr7, 0);
    if (st != I2C_OK) return st;
    clear_addr();

    st = send_byte(reg);
    if (st != I2C_OK) return st;

    for (uint16_t i = 0; i < len; i++) {
        st = send_byte(data[i]);
        if (st != I2C_OK) return st;
    }

    st = wait_sr1(I2C_SR1_BTF);
    if (st != I2C_OK) return st;
    i2c_stop();
    return I2C_OK;
}

i2c_status_t i2c_read_reg(uint8_t addr7, uint8_t reg, uint8_t *data, uint16_t len)
{
    if (len == 0U) return I2C_OK;

    i2c_status_t st = wait_not_busy();
    if (st != I2C_OK) return st;

    /* --- Phase 1: write the register address --- */
    st = start_and_address(addr7, 0);
    if (st != I2C_OK) return st;
    clear_addr();

    st = send_byte(reg);
    if (st != I2C_OK) return st;
    st = wait_sr1(I2C_SR1_BTF);
    if (st != I2C_OK) return st;

    /* --- Phase 2: repeated START, read --- */
    st = start_and_address(addr7, 1);
    if (st != I2C_OK) return st;

    if (len == 1U) {
        /* Single byte: NACK must be armed BEFORE ADDR is cleared */
        I2C1->CR1 &= ~I2C_CR1_ACK;
        clear_addr();
        i2c_stop();
        st = wait_sr1(I2C_SR1_RXNE);
        if (st != I2C_OK) return st;
        data[0] = (uint8_t)I2C1->DR;
    } else {
        I2C1->CR1 |= I2C_CR1_ACK;
        clear_addr();

        while (len) {
            if (len == 1U) {
                /* Last byte is being received: answer it with NACK + STOP */
                I2C1->CR1 &= ~I2C_CR1_ACK;
                i2c_stop();
            }
            st = wait_sr1(I2C_SR1_RXNE);
            if (st != I2C_OK) return st;
            *data++ = (uint8_t)I2C1->DR;
            len--;
        }
    }

    I2C1->CR1 |= I2C_CR1_ACK;                   /* restore default */
    return I2C_OK;
}
