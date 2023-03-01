#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <inttypes.h>

#define SW0_NODE	DT_ALIAS(sw0)

static const struct gpio_dt_spec button1 = GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios, {0});

static struct gpio_callback button1_cb_data;



void button1_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	int val = gpio_pin_get_dt(&button1);
	if (val == 1) {
		printk("Button1 pressed.\n");
	}
}


void init_button(const struct gpio_dt_spec* button,
                 struct gpio_callback* button_cb_data,
				 gpio_callback_handler_t cb) {
	int ret;

	if (!device_is_ready(button->port)) {
		printk("Error: button device %s is not ready\n",
		       button->port->name);
		return;
	}

	ret = gpio_pin_configure_dt(button, GPIO_INPUT);
	if (ret != 0) {
		printk("Error %d: failed to configure %s pin %d\n",
		       ret, button->port->name, button->pin);
		return;
	}

	ret = gpio_pin_interrupt_configure_dt(button, GPIO_INT_EDGE_BOTH);
	if (ret != 0) {
		printk("Error %d: failed to configure interrupt on %s pin %d\n",
			ret, button->port->name, button->pin);
		return;
	}

	gpio_init_callback(button_cb_data, cb, BIT(button->pin));
	gpio_add_callback(button->port, button_cb_data);
	printk("Set up button at %s pin %d\n", button->port->name, button->pin);
}

void main(void)
{
	init_button(&button1, &button1_cb_data, button1_pressed);

	printk("Press button 1\n");
}
