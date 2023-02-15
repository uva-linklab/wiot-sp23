#include <zephyr/types.h>
#include <stddef.h>
#include <errno.h>
#include <zephyr.h>
#include <sys/printk.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include <sys/byteorder.h>

#define SEARCH_UUID BT_UUID_128_ENCODE(0xBDFC9792, 0x8234, 0x405E, 0xAE02, 0x35EF3274B299)
// #define SEARCH_UUID BT_UUID_128_ENCODE(0x5253FF4B, 0xE47C, 0x4EC8, 0x9792, 0x69FDF4923B4A)

static void start_scan(void);

static struct bt_conn *default_conn;

static struct bt_gatt_discover_params discover_params;



struct discovered_gatt_descriptor {
	uint16_t handle;
	struct bt_uuid_128 uuid;
};

struct discovered_gatt_characteristic {
	uint16_t handle;
	uint16_t value_handle;
	struct bt_uuid_128 uuid;
	int num_descriptors;
	struct discovered_gatt_descriptor descriptors[10];
};

struct discovered_gatt_service {
	uint16_t handle;
	uint16_t end_handle;
	struct bt_uuid_128 uuid;
	int num_characteristics;
	struct discovered_gatt_characteristic characteristics[10];
};

int num_discovered_services = 0;
struct discovered_gatt_service services[10];

enum discovering_state {
	DISC_STATE_SERVICES = 0,
	DISC_STATE_CHARACTERISTICS = 1,
	DISC_STATE_DESCRIPTORS = 2,
	DISC_STATE_DONE = 3,
};

enum discovering_state disc_state = DISC_STATE_SERVICES;
int discovering_index_svc = 0;


static uint8_t discover_func(struct bt_conn *conn,
			                 const struct bt_gatt_attr *attr,
			                 struct bt_gatt_discover_params *params)
{
	int err;

	if (!attr) {
		// printk("Discover complete\n");
	
		if (disc_state == DISC_STATE_SERVICES) {
			disc_state = DISC_STATE_CHARACTERISTICS;
			discovering_index_svc = 0;

			if (num_discovered_services > 0) {
				// Discover characteristics for first service.
				discover_params.uuid = NULL;
				discover_params.func = discover_func;
				discover_params.start_handle = services[discovering_index_svc].handle + 1;
				discover_params.end_handle = services[discovering_index_svc].end_handle;
				discover_params.type = BT_GATT_DISCOVER_CHARACTERISTIC;
				err = bt_gatt_discover(conn, &discover_params);
			}
		}
		else if (disc_state == DISC_STATE_CHARACTERISTICS) {
			discovering_index_svc += 1;

			if (discovering_index_svc < num_discovered_services) {
				// Discover characteristics for each additional service.
				discover_params.uuid = NULL;
				discover_params.func = discover_func;
				discover_params.start_handle = services[discovering_index_svc].handle + 1;
				discover_params.end_handle = services[discovering_index_svc].end_handle;
				discover_params.type = BT_GATT_DISCOVER_CHARACTERISTIC;
				err = bt_gatt_discover(conn, &discover_params);
			} else {
				// Move to descriptors
				disc_state = DISC_STATE_DESCRIPTORS;
				discovering_index_svc = 0;

				discover_params.uuid = NULL;
				discover_params.func = discover_func;
				discover_params.start_handle = services[discovering_index_svc].handle + 1;
				discover_params.end_handle = services[discovering_index_svc].end_handle;
				discover_params.type = BT_GATT_DISCOVER_DESCRIPTOR;
				err = bt_gatt_discover(conn, &discover_params);
			}
		} else if (disc_state == DISC_STATE_DESCRIPTORS) {
			discovering_index_svc += 1;


			if (discovering_index_svc < num_discovered_services) {
				discover_params.uuid = NULL;
				discover_params.func = discover_func;
				discover_params.start_handle = services[discovering_index_svc].handle + 1;
				discover_params.end_handle = services[discovering_index_svc].end_handle;
				discover_params.type = BT_GATT_DISCOVER_DESCRIPTOR;
				err = bt_gatt_discover(conn, &discover_params);
			} else {
				disc_state = DISC_STATE_DONE;

				// Print everything we discovered.
				char str[128];
				int s, c, d;
				for (s=0; s<num_discovered_services; s++) {
					struct discovered_gatt_service* disc_serv = &services[s];

					bt_uuid_to_str((struct bt_uuid*) &disc_serv->uuid, str, 128);
					printk("\nService [%s] - handle: 0x%02x\n", str, disc_serv->handle);

					for (c=0; c<disc_serv->num_characteristics; c++) {
						struct discovered_gatt_characteristic* disc_char = &disc_serv->characteristics[c];

						bt_uuid_to_str((struct bt_uuid*) &disc_char->uuid, str, 128);
						printk("  -Characteristic [%s] - handle: 0x%02x\n", str, disc_char->handle);
						printk("  ---Value handle: 0x%02x\n", disc_char->value_handle);

						for (d=0; d<disc_char->num_descriptors; d++) {
							struct discovered_gatt_descriptor* disc_desc = &disc_char->descriptors[d];

							if (bt_uuid_cmp((struct bt_uuid*) &disc_desc->uuid, BT_UUID_GATT_CCC) == 0) {
								printk("  ---CCC - handle: 0x%02x\n", disc_desc->handle);
							} else {
								bt_uuid_to_str((struct bt_uuid*) &disc_desc->uuid, str, 128);
								printk("  ---Descriptor [%s] - handle: 0x%02x\n", str, disc_char->handle);
							}
						}
					}
				}
			}
		}

		return BT_GATT_ITER_STOP;
	}

	if (params->type == BT_GATT_DISCOVER_PRIMARY) {
		// We are looking for the services

		struct bt_gatt_service_val* gatt_service = (struct bt_gatt_service_val*) attr->user_data;

		services[num_discovered_services] = (struct discovered_gatt_service) {
			.handle = attr->handle,
			.end_handle = gatt_service->end_handle,
			.num_characteristics = 0
		};
		memcpy(&services[num_discovered_services].uuid, BT_UUID_128(gatt_service->uuid),
			       sizeof(services[num_discovered_services].uuid));

		num_discovered_services += 1;
	}
	else if (params->type == BT_GATT_DISCOVER_CHARACTERISTIC) {

		struct bt_gatt_chrc* gatt_characteristic = (struct bt_gatt_chrc*) attr->user_data;

		struct discovered_gatt_service* disc_serv_loc = &services[discovering_index_svc];
		struct discovered_gatt_characteristic* disc_char_loc = &disc_serv_loc->characteristics[disc_serv_loc->num_characteristics];

		*disc_char_loc = (struct discovered_gatt_characteristic) {
			.handle = attr->handle,
			.value_handle = gatt_characteristic->value_handle,
			.num_descriptors = 0,
		};
		memcpy(&disc_char_loc->uuid, BT_UUID_128(gatt_characteristic->uuid), sizeof(disc_char_loc->uuid));

		services[discovering_index_svc].num_characteristics += 1;
	}
	else if (params->type == BT_GATT_DISCOVER_DESCRIPTOR) {
		uint16_t handle = attr->handle;
		int i;

		int matched_characteristic = 0;
		for (i=1; i<services[discovering_index_svc].num_characteristics; i++) {
			if (handle < services[discovering_index_svc].characteristics[i].handle) {
				break;
			}
			matched_characteristic = i;
		}

		struct discovered_gatt_service* disc_serv_loc = &services[discovering_index_svc];
		struct discovered_gatt_characteristic* disc_char_loc = &disc_serv_loc->characteristics[matched_characteristic];
		struct discovered_gatt_descriptor* disc_desc_loc = &disc_char_loc->descriptors[disc_char_loc->num_descriptors];

		*disc_desc_loc = (struct discovered_gatt_descriptor) {
			.handle = handle,
		};
		memcpy(&disc_desc_loc->uuid, BT_UUID_128(attr->uuid), sizeof(disc_desc_loc->uuid));

		disc_char_loc->num_descriptors += 1;
	}

	return BT_GATT_ITER_CONTINUE;
}

static void connected(struct bt_conn *conn, uint8_t conn_err)
{
	char addr[BT_ADDR_LE_STR_LEN];
	int err;

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	if (conn_err) {
		printk("Failed to connect to %s (%u)\n", addr, conn_err);

		bt_conn_unref(default_conn);
		default_conn = NULL;

		start_scan();
		return;
	}

	printk("Connected: %s\n", addr);

	if (conn == default_conn) {
		discover_params.uuid = NULL;
		discover_params.func = discover_func;
		discover_params.start_handle = BT_ATT_FIRST_ATTTRIBUTE_HANDLE;
		discover_params.end_handle = BT_ATT_LAST_ATTTRIBUTE_HANDLE;
		discover_params.type = BT_GATT_DISCOVER_PRIMARY;

		disc_state = DISC_STATE_SERVICES;

		err = bt_gatt_discover(default_conn, &discover_params);
		if (err) {
			printk("Discover failed(err %d)\n", err);
			return;
		}
	}
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	printk("Disconnected: %s (reason 0x%02x)\n", addr, reason);

	if (default_conn != conn) {
		return;
	}

	bt_conn_unref(default_conn);
	default_conn = NULL;

	start_scan();
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
	.connected = connected,
	.disconnected = disconnected,
};

// Called for each advertising data element in the advertising data.
static bool ad_found(struct bt_data *data, void *user_data)
{
	bt_addr_le_t *addr = user_data;

	printk("[AD]: %u data_len %u\n", data->type, data->data_len);

	switch (data->type) {
	case BT_DATA_UUID128_ALL:
		if (data->data_len != 16) {
			printk("AD malformed\n");
			return true;
		}

		struct bt_le_conn_param *param;
		struct bt_uuid uuid;
		int err;

		bt_uuid_create(&uuid, data->data, 16);
		if (bt_uuid_cmp(&uuid, BT_UUID_DECLARE_128(SEARCH_UUID)) == 0) {
			printk("Found matching advertisement\n");

			err = bt_le_scan_stop();
			if (err) {
				printk("Stop LE scan failed (err %d)\n", err);
				return false;
			}

			param = BT_LE_CONN_PARAM_DEFAULT;
			err = bt_conn_le_create(addr, BT_CONN_LE_CREATE_CONN, param, &default_conn);
			if (err) {
				printk("Create conn failed (err %d)\n", err);
				start_scan();
			}
		}

		return false;
		
	}

	return true;
}

static void device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type,
			 struct net_buf_simple *ad)
{
	char dev[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(addr, dev, sizeof(dev));
	printk("[DEVICE]: %s, AD evt type %u, AD data len %u, RSSI %i\n",
	       dev, type, ad->len, rssi);

	// We're only interested in connectable devices.
	if (type == BT_GAP_ADV_TYPE_ADV_IND ||
	    type == BT_GAP_ADV_TYPE_ADV_DIRECT_IND) {
		// Helper function to parse the advertising data (AD) elements
		// from the advertisement. This will call `ad_found()` for
		// each element.
		bt_data_parse(ad, ad_found, (void*) addr);
	}
}

static void start_scan(void)
{
	int err;

	struct bt_le_scan_param scan_param = {
		.type       = BT_LE_SCAN_TYPE_PASSIVE,
		.options    = BT_LE_SCAN_OPT_NONE,
		.interval   = BT_GAP_SCAN_FAST_INTERVAL,
		.window     = BT_GAP_SCAN_FAST_WINDOW,
	};

	err = bt_le_scan_start(&scan_param, device_found);
	if (err) {
		printk("Scanning failed to start (err %d)\n", err);
		return;
	}

	printk("Scanning successfully started\n");
}

static void bt_ready(int err)
{
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	printk("Bluetooth initialized\n");

	start_scan();
}

void main(void)
{
	int err;

	err = bt_enable(bt_ready);

	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}
}
