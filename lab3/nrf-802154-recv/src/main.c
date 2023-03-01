/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <stdio.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>

#include <zephyr/logging/log.h>
	LOG_MODULE_REGISTER(recv);

#include "nrf_802154.h"

#define PSDU_MAX_SIZE (127u)

static void pkt_hexdump(uint8_t *pkt, uint8_t length) {
  int i;
  printk(" -> Packet content:");
  for (i = 0; i < length;) {
    int j;
    printk("\t");

	// groups every 10 bytes
    for (j = 0; j < 10 && i < length; j++, i++) {
      printk("%02x ", *pkt);
      pkt++;
    }
    printk("");
  }
  printk("\n");
}

static int rf_setup(const struct device *dev)
{
	LOG_INF("RF setup started");
	ARG_UNUSED(dev);

	/* nrf radio driver initialization */
	nrf_802154_init();
	return 0;
}

void nrf_802154_received_raw(uint8_t *data, int8_t power, uint8_t lqi) {
	pkt_hexdump(data+1, *data - 2U); /* print packet from byte [1, len-2] */
	nrf_802154_buffer_free_raw(data);
}

// void nrf_802154_receive_failed(nrf_802154_rx_error_t error, uint32_t id) {
// 	LOG_INF("rx failed error %u!", error);
// }

int main(void) {
	nrf_802154_channel_set(11u);
	nrf_802154_auto_ack_set(false);
	LOG_DBG("channel: %u", nrf_802154_channel_get());
	
	// set the pan_id (2 bytes, little-endian)
	uint8_t pan_id[] = {0xcd, 0xab};
	nrf_802154_pan_id_set(pan_id);

	// set the extended address (8 bytes, little-endian)
	uint8_t extended_addr[] = {0x50, 0xbe, 0xca, 0xc3, 0x3c, 0x36, 0xce, 0xf4};
	nrf_802154_extended_address_set(extended_addr);

	if(nrf_802154_receive()) {
		LOG_DBG("radio entered rx state");
	} else {
		LOG_ERR("driver could not enter the receive state");
	}

	return 0;
}

SYS_INIT(rf_setup, POST_KERNEL, CONFIG_PTT_RF_INIT_PRIORITY);