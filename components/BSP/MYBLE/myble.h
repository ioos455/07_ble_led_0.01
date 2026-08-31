#ifndef __MYBLE_H_
#define __MYBLE_H_

#define MYBULE_NAME "ESP32-BLE"                         /*蓝牙设备名称*/

void start_advertising(void);                                              /*蓝牙开始广播函数*/
void ble_init(void);                                                       /*蓝牙初始化函数*/


#endif // __MYBLE_H_