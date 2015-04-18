

/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Jeroen Domburg <jeroen@spritesmods.com> wrote this file. As long as you retain
 * this notice you can do whatever you want with this stuff. If we meet some day,
 * and you think this stuff is worth it, you can buy me a beer in return.
 * ----------------------------------------------------------------------------
 */


#include "espmissingincludes.h"
#include "ets_sys.h"
#include "osapi.h"
#include "httpd.h"
#include "io.h"
#include "httpdespfs.h"
#include "cgi.h"
#include "cgiwifi.h"
#include "cgiflash.h"
#include "stdout.h"
#include "auth.h"
#include "config.h"
#include "driver/uart.h"
#include "mqtt.h"
#include "wifi.h"
#include "debug.h"
#include "gpio.h"
#include "user_interface.h"
#include "mem.h"
#include "os_type.h"
#include "jsmn.h"

#define user_procTaskPrio        0
#define user_procTaskQueueLen    1

#define settings_name "orange"

MQTT_Client mqttClient;

int g_temperature = 0;
int g_thresholdTemperature = 30;
bool g_settingsUpdated = false;

os_event_t    user_procTaskQueue[user_procTaskQueueLen];

static volatile os_timer_t some_timer;

char* itoa(int value, char* result, int base);
int atoi(const char *s);

bool parse(char *json);

void publishData(MQTT_Client* client);

void wifiConnectCb(uint8_t status)
{
	if(status == STATION_GOT_IP){
		INFO("MQTT: wifiConnectCb\r\n");
		MQTT_Connect(&mqttClient);
	} else {
		MQTT_Disconnect(&mqttClient);
	}
}
void mqttConnectedCb(uint32_t *args)
{
	connected_mqtt_cloud = true;

	MQTT_Client* client = (MQTT_Client*)args;
	INFO("MQTT: Connected\r\n");
	if (FALSE == MQTT_Subscribe(client, "/settings/temperature", 0))
	{
		INFO("MQTT: Unable to subscribe to /settings/temperature\r\n");
	}
	if (FALSE == MQTT_Subscribe(client, "/ping", 0))
	{
		INFO("MQTT: Unable to subscribe to /ping\r\n");
	}
}

void mqttDisconnectedCb(uint32_t *args)
{
	INFO("MQTT: Disconnected\r\n");
}

void mqttPublishedCb(uint32_t *args)
{
	INFO("MQTT: Published\r\n");
}

void mqttDataCb(uint32_t *args, const char* topic, uint32_t topic_len, const char *data, uint32_t data_len)
{
	char *topicBuf = (char*)os_zalloc(topic_len+1),
			*dataBuf = (char*)os_zalloc(data_len+1);

	MQTT_Client* client = (MQTT_Client*)args;

	os_memcpy(topicBuf, topic, topic_len);
	topicBuf[topic_len] = 0;

	os_memcpy(dataBuf, data, data_len);
	dataBuf[data_len] = 0;

	if (0 == strcmp(topicBuf, "/settings/temperature"))
	{
		if (false == parse(dataBuf))
		{
			INFO("\r\nUnable to parse JSON.\r\n");
		}
	}
	else if (0 == strcmp(topicBuf, "/ping"))
	{
		publishData(client);
	}

	INFO("Receive topic: %s, data: %s \r\n", topicBuf, dataBuf);
	os_free(topicBuf);
	os_free(dataBuf);
}

void publishData(MQTT_Client* client)
{
	char *tempStr = "00";
	itoa(g_temperature, tempStr, 10);
	char str[255];
	ets_sprintf(str, "{ \"name\": \"%s\", \"temperature\": \"%s\" }",
	settings_name, tempStr);

	MQTT_Publish(client, "/sensors/temperature", str, strlen(str), 0, 1);
}


char* itoa(int value, char* result, int base) {
	// check that the base if valid
	if (base < 2 || base > 36) { *result = (char) 0; return result; }

		char* ptr = result, *ptr1 = result, tmp_char;
		int tmp_value;

		do {
			tmp_value = value;
			value /= base;
			*ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
		} while ( value );

		// Apply negative sign
		if (tmp_value < 0) *ptr++ = '-';
		*ptr-- = (char) 0;
		while(ptr1 < ptr) {
			tmp_char = *ptr;
			*ptr--= *ptr1;
			*ptr1++ = tmp_char;
		}
		return result;
	}

	int atoi (const char *s)
{
	return (int) strtol (s, NULL, 10);
}

static int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
	if (tok->type == JSMN_STRING && (int) strlen(s) == tok->end - tok->start &&
		strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
			return 0;
		}
		return -1;
}

bool parse(char *json) {

	jsmn_parser parser;
	// Max number of tokens: 32
	jsmntok_t tokens[32];

	jsmn_init(&parser);
	// All tokens retrieved from the JSON
	int tokensCount = jsmn_parse(&parser, json, strlen(json), tokens,
	sizeof(tokens)/sizeof(tokens[0]));
	if (tokensCount < 0) {
		return false;
	}

	// Assume the top-level element is an object
	if ( (tokensCount < 1) || (tokens[0].type != JSMN_OBJECT) ) {
		return false;
	}

	// Loop over all keys of the root object
	int index = 1;
	for (index = 1; index < tokensCount; index++) {
		if (jsoneq(json, &tokens[index], "temperature") == 0) {

			unsigned int length = tokens[index+1].end - tokens[index+1].start;
			char keyString[length + 1];
			memcpy(keyString, &json[tokens[index+1].start], length);
			keyString[length] = '\0';

			INFO("Temperature settings detected %s\n", keyString);

			int settingsTemperature = atoi(keyString);
			if (g_thresholdTemperature != settingsTemperature) {
				g_thresholdTemperature = settingsTemperature;
				g_settingsUpdated = true;
			}

			INFO("Threshold Temperature %d\n", g_thresholdTemperature);

			index++;
		}
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////////

//Function that tells the authentication system what users/passwords live on the system.
//This is disabled in the default build; if you want to try it, enable the authBasic line in
//the builtInUrls below.
int myPassFn(HttpdConnData *connData, int no, char *user, int userLen, char *pass, int passLen) {
	if (no==0) {
		os_strcpy(user, "admin");
		os_strcpy(pass, "s3cr3t");
		return 1;
//Add more users this way. Check against incrementing no for each user added.
//	} else if (no==1) {
//		os_strcpy(user, "user1");
//		os_strcpy(pass, "something");
//		return 1;
	}
	return 0;
}


/*
This is the main url->function dispatching data struct.
In short, it's a struct with various URLs plus their handlers. The handlers can
be 'standard' CGI functions you wrote, or 'special' CGIs requiring an argument.
They can also be auth-functions. An asterisk will match any url starting with
everything before the asterisks; "*" matches everything. The list will be
handled top-down, so make sure to put more specific rules above the more
general ones. Authorization things (like authBasic) act as a 'barrier' and
should be placed above the URLs they protect.
*/
HttpdBuiltInUrl builtInUrls[]={
	{"/", cgiRedirect, "/wifi.tpl"},
	//Routines to make the /wifi URL and everything beneath it work.

//Enable the line below to protect the WiFi configuration with an username/password combo.
//	{"/*", authBasic, myPassFn},

	{"/wifi", cgiRedirect, "/wifi.tpl"},
	{"/wifi/", cgiRedirect, "/wifi.tpl"},
	{"/wifiscan.cgi", cgiWiFiScan, NULL},
	{"/wifi.tpl", cgiEspFsTemplate, tplWlan},
	{"/connect.cgi", cgiWiFiConnect, NULL},
	{"/setmode.cgi", cgiWifiSetMode, NULL},

	{"*", cgiEspFsHook, NULL}, //Catch-all cgi function for the filesystem
	{NULL, NULL, NULL}
};

//Main routine. Initialize stdout, the I/O and the webserver and we're done.
void user_init(void) {
	connected_mqtt_cloud = false;

	uart_init(BIT_RATE_115200, BIT_RATE_115200);
	os_delay_us(1000000);

	CFG_Load();

	ioInit();
	stdoutInit();
	os_printf("\nSSID: %s\n", sysCfg.sta_ssid);
	if (0 == strlen((const char *)sysCfg.sta_ssid)) {
		wifi_set_opmode(0x3); //reset to AP+STA mode

		//Check WiFi mode (required on the first launch of the device)
		if (3 != wifi_get_opmode()){
			//Reset to AP+STA mode and restart
			wifi_set_opmode(0x3);
			os_printf("Reset to AP+STA mode. Restarting system...\n");
			system_restart();
		}

		httpdInit(builtInUrls, 80);
		os_printf("\nHTTP server daemon started...\n");
	}
	else {
		os_printf("\r\nMQTT ...\r\n");

		os_printf("\nMQTT host: %s\n", sysCfg.mqtt_host);
		MQTT_InitConnection(&mqttClient, sysCfg.mqtt_host, sysCfg.mqtt_port, sysCfg.security);
		os_printf("\r\nInit ...\r\n");

		MQTT_InitClient(&mqttClient, sysCfg.device_id, sysCfg.mqtt_user, sysCfg.mqtt_pass, sysCfg.mqtt_keepalive, 1);

		MQTT_InitLWT(&mqttClient, (uint8_t *)"/lwt", (uint8_t *)"offline", 0, 0);
		MQTT_OnConnected(&mqttClient, mqttConnectedCb);
		MQTT_OnDisconnected(&mqttClient, mqttDisconnectedCb);
		MQTT_OnPublished(&mqttClient, mqttPublishedCb);
		MQTT_OnData(&mqttClient, mqttDataCb);

		WIFI_Connect(sysCfg.sta_ssid, sysCfg.sta_pwd, wifiConnectCb);
	}
}
