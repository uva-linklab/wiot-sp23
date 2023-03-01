#include <stdio.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>

#include <zephyr/logging/log.h>
	LOG_MODULE_REGISTER(send);

#include "nrf_802154.h"
#include <inttypes.h>

#define PSDU_MAX_SIZE (127u)
#define FCS_LENGTH (2u) /* Length of the Frame Control Sequence */

static int rf_setup(const struct device *dev)
{
	LOG_INF("RF setup started");
	ARG_UNUSED(dev);

	/* nrf radio driver initialization */
	nrf_802154_init();

	return 0;
}

// callback fn when tx starts
void nrf_802154_tx_started(const uint8_t *p_frame) {
	LOG_INF("tx started");
}

// callback fn when tx fails
void nrf_802154_transmit_failed(uint8_t *frame, nrf_802154_tx_error_t error,
	const nrf_802154_transmit_done_metadata_t *p_metadata)
{
	LOG_INF("tx failed error %u!", error);
}

// callback fn for successful tx
void nrf_802154_transmitted_raw(uint8_t *p_frame, 
	const nrf_802154_transmit_done_metadata_t *p_metadata) {
	LOG_INF("frame was transmitted!");
}

int main(int argc, char **argv) {
	nrf_802154_channel_set(11u);
	LOG_DBG("channel: %u", nrf_802154_channel_get());

	// set the pan_id (2 bytes, little-endian)
	uint8_t src_pan_id[] = {0xcd, 0xab}; 
	nrf_802154_pan_id_set(src_pan_id);

	// set the extended address (8 bytes, little-endian)
	uint8_t src_extended_addr[] = {0xdc, 0xa9, 0x35, 0x7b, 0x73, 0x36, 0xce, 0xf4};
	nrf_802154_extended_address_set(src_extended_addr);

	uint8_t dst_extended_addr[] = {0x50, 0xbe, 0xca, 0xc3, 0x3c, 0x36, 0xce, 0xf4};

	// send a tx pkt
	uint8_t pkt[PSDU_MAX_SIZE];
	pkt[0] = 26u + FCS_LENGTH; /* Length for nrf_transmit (length of pkt + FCS) */
	pkt[1] = 0x01; /* Frame Control Field */
	pkt[2] = 0xcc; /* Frame Control Field */
	pkt[3] = 0x00; /* Sequence number */
	pkt[4] = 0xff; /* Destination PAN ID 0xffff */
	pkt[5] = 0xff; /* Destination PAN ID */
	memcpy(&pkt[6], dst_extended_addr, 8); /* Destination extended address */
	memcpy(&pkt[14], src_pan_id, 2); /* Source PAN ID */
	memcpy(&pkt[16], src_extended_addr, 8);/* Source extended address */ 
	pkt[24] = 0xAA; /* Payload */
	pkt[25] = 0xBB; /* */
	pkt[26] = 0xCC; /* */

	const nrf_802154_transmit_metadata_t metadata = {
			.frame_props = NRF_802154_TRANSMITTED_FRAME_PROPS_DEFAULT_INIT,
			.cca = true
		};
	
	while(1) {
		// send the packet
		if(!nrf_802154_transmit_raw(pkt, &metadata)) {
			LOG_ERR("driver could not schedule the transmission procedure.");
		}
		// sleep for 10 seconds
		k_sleep(K_MSEC(10000)); 
	}
	
	return 0;
}

SYS_INIT(rf_setup, POST_KERNEL, CONFIG_PTT_RF_INIT_PRIORITY);