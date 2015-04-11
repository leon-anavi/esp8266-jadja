#ifndef USER_IO_H_
#define USER_IO_H_

bool connected_mqtt_cloud;

void ICACHE_FLASH_ATTR ioLed(int ena);
void ioInit(void);
void blink(void *arg);

#endif /* USER_IO_H_ */
