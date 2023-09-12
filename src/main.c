#include <stdio.h>
#include "hardware/pio.h"
#include "pico/stdlib.h"

#include "can2040.h"

#define CONFIG_CANBUS_FREQUENCY 1000000
#define CONFIG_RP2040_CANBUS_GPIO_RX    19  // GPIO 26 <=> CAN TX
#define CONFIG_RP2040_CANBUS_GPIO_TX    27  // GPIO 27 <=> CAN RX

#define RESETS_RESET_PIO0_RESET  _u(0x1)
#define FREQ_SYS 125000000

static struct can2040 cbus;

// can2040 callback function - handle rx and tx notifications
static void
can2040_cb(struct can2040 *cd, uint32_t notify, struct can2040_msg *msg)
{
    if (notify == CAN2040_NOTIFY_RX)
    {
        printf("xfguo: recv msg: (id: %0x, size: %0x, data: %0x, %0x)\n", msg->id & 0x7ff, msg->dlc, msg->data32[0], msg->data32[1]);
    } else if (notify == CAN2040_NOTIFY_TX) {
        printf("xfguo: confirmed tx msg: (id: %0x, size: %0x, data: %0x, %0x)\n", msg->id & 0x7ff, msg->dlc, msg->data32[0], msg->data32[1]);
    } else if (notify & CAN2040_NOTIFY_ERROR) {
        printf("xfguo: error(%d) on msg: (id: %0x, size: %0x, data: %0x, %0x)\n", notify & 0xff, msg->id & 0x7ff, msg->dlc, msg->data32[0], msg->data32[1]);
    }
}

uint32_t
get_pclock_frequency(uint32_t reset_bit)
{
    return FREQ_SYS;
}

// Main PIO irq handler
void
PIOx_IRQHandler(void)
{
    can2040_pio_irq_handler(&cbus);
}

int main() {
    stdio_init_all();
    printf("xfguo: can_main start\n");

    // Setup canbus
    can2040_setup(&cbus, 0);
    can2040_callback_config(&cbus, can2040_cb);

    // Start canbus
    uint32_t pclk = get_pclock_frequency(RESETS_RESET_PIO0_RESET);
    can2040_start(&cbus, pclk, CONFIG_CANBUS_FREQUENCY
            , CONFIG_RP2040_CANBUS_GPIO_RX, CONFIG_RP2040_CANBUS_GPIO_TX);

    // 设置中断处理函数 / 优先级，并启用中断
    irq_set_exclusive_handler(PIO0_IRQ_0, PIOx_IRQHandler);
    irq_set_priority(PIO0_IRQ_0, 1);
    irq_set_enabled(PIO0_IRQ_0, true);

    printf("xfguo: CAN pio is set up\n");

    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    struct can2040_msg msg = {
            .id = 2 | CAN2040_ID_EFF,
            .dlc = 8,
            .data32 = {
                    0xefbeadde,
                    0x12345678
            }
    };

    while (true) {
        gpio_put(LED_PIN, 1);
        sleep_ms(250);

        if (can2040_check_transmit(&cbus)) {
            can2040_transmit(&cbus, &msg);
            msg.data32[1]++;
            printf("Transmit %0x.%0x\n", msg.data32[0], msg.data32[1]);
        }

        gpio_put(LED_PIN, 0);
        sleep_ms(250);
    }
}
