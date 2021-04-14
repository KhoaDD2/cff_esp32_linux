#include <stdint.h>
#include <string.h>


#include "esp32_at.h"
#include "bt.h"

BTCmdCbFn bt_data_cb;
BT_ConnectedCbFn bt_conn_cb;

#define BT_NAME "OBDII"

uint32_t BT_Startup(void)
{
	uint32_t result = 1;
	do {
		if (!esp32_at_cmdSync(AT_CMD_BT_INIT, "1", strlen("1"), NULL, NULL, 0)) {
			break;
		}

		// Init BT SPP as slave mode
		if (!esp32_at_cmdSync(AT_CMD_BT_SPP_INIT, "2", strlen("2"), NULL, NULL, 0)) {
			break;
		}

		// Start BT SPP
		if (!esp32_at_cmdSync(AT_CMD_BT_SPP_START, 0, 0, NULL, NULL, 0)) {
			break;
		}

		// Setup BT name
		if (!esp32_at_cmdSync(AT_CMD_BT_NAME, BT_NAME, strlen(BT_NAME), NULL, NULL, 0)) {
			break;
		}

		// Setup BT scan mode
		if (!esp32_at_cmdSync(AT_CMD_BT_SCAN_MODE, "2", strlen("2"), NULL, NULL, 0)) {
			break;
		}

		// // Setup BT scan mode
		// bt_sec_param_t sec = {
		// 	.input = 3,
		// 	.output = 1,
		// 	.pin = "0000",
		// };

		// if (!esp32_at_cmdSync(AT_CMD_BT_SEC_PARAM, (uint8_t *) &sec, sizeof sec, NULL, NULL, 0)) {
		// 	break;
		// }

		result = 1;
	} while (0);

	return result;
}

int BT_SPP_SendMessage(uint8_t* data, uint32_t length)
{
  int result = 0;
  if (esp32_at_cmdSync(AT_CMD_BT_SPP_SEND_INIT, NULL, length, NULL, NULL, 0))
  {
    // printf("Sending %lu bytes\r\n", length);
    result = (int) esp32_at_cmdSync(AT_CMD_BT_SPP_SEND_DATA, (uint8_t*) data, length, NULL, NULL, 0);
  }

  return result;
}

void BT_Echo(uint8_t* data, uint32_t len)
{
	BT_SPP_SendMessage(data, len);
}

int BT_SPP_RegisterConnectedCallback(BT_ConnectedCbFn fn)
{
	bt_conn_cb = fn;
	return 0;
}

int BT_SPP_RegsiterDataCallback(BTCmdCbFn fn)
{
	bt_data_cb = fn;
    return 0;
}
