
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Jeroen Domburg <jeroen@spritesmods.com> wrote this file. As long as you retain
 * this notice you can do whatever you want with this stuff. If we meet some day,
 * and you think this stuff is worth it, you can buy me a beer in return.
 * ----------------------------------------------------------------------------
 */


#include "c_types.h"
#include "user_interface.h"
#include "espconn.h"
#include "mem.h"
#include "osapi.h"
#include "gpio.h"
#include "espmissingincludes.h"
#include "config.h"
#include "io.h"

#define LEDGPIO 2
#define BTNGPIO 0

static ETSTimer resetBtntimer;

static os_timer_t blink_timer;
static int blink_counter = 1;

void ICACHE_FLASH_ATTR ioLed(int ena) {
	//gpio_output_set is overkill. ToDo: use better mactos
	if (ena) {
		gpio_output_set((1<<LEDGPIO), 0, (1<<LEDGPIO), 0);
	} else {
		gpio_output_set(0, (1<<LEDGPIO), (1<<LEDGPIO), 0);
	}
}

static void ICACHE_FLASH_ATTR resetBtnTimerCb(void *arg) {
	static int resetCnt=0;
	if (!GPIO_INPUT_GET(BTNGPIO)) {
		resetCnt++;
	} else {
		if (resetCnt>=6) { //3 sec pressed
			//reset WiFi configurations
			CFG_Update("", "", "", 1883, "", "");

			wifi_station_disconnect();
			wifi_set_opmode(0x3); //reset to AP+STA mode
			os_printf("Reset to AP+STA mode. Restarting system...\n");
			system_restart();
		}
		resetCnt=0;
	}
}

void blink(void *arg)
{
	if (true == connected_mqtt_cloud) {
		//Set GPIO4 to HIGH
		gpio_output_set(BIT4, 0, BIT4, 0);
	}
	else {
		if (1 == blink_counter) {
			//Set GPIO4 to HIGH
			gpio_output_set(BIT4, 0, BIT4, 0);
			blink_counter = 0;
		}
		else {
			//Set GPIO4 to LOW
			gpio_output_set(0, BIT4, BIT4, 0);
			blink_counter = 1;
		}
	}

	if (0 == blink_counter_lamp)
	{
		//Set GPIO5 to LOW
		gpio_output_set(0, BIT5, BIT5, 0);
	}
	else
	{
		//Decrease counter
		if (0 < blink_counter_lamp) {
			blink_counter_lamp -= 1;
		}
		//Set GPIO5 to HIGH
		gpio_output_set(BIT5, 0, BIT5, 0);
	}
}

void ioInit() {
	//Set GPIO4 to output mode
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4);

	//Set GPIO5 to output mode
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5);

	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0);
	gpio_output_set(0, 0, (1<<LEDGPIO), (1<<BTNGPIO));
	os_timer_disarm(&resetBtntimer);
	os_timer_setfn(&resetBtntimer, resetBtnTimerCb, NULL);
	os_timer_arm(&resetBtntimer, 500, 1);

	//Disarm timer
	os_timer_disarm(&blink_timer);

	//Setup timer
	os_timer_setfn(&blink_timer, (os_timer_func_t *)blink, NULL);

	//Arm the timer
	//&some_timer is the pointer
	//1000 is the fire time in ms
	//0 for once and 1 for repeating
	os_timer_arm(&blink_timer, 1000, 1);
}
