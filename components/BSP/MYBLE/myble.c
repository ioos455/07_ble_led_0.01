#include "myble.h"

#include "driver/gpio.h"
#include "esp_log.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "led.h"

#define TAG "MYBLE-Demo"
static bool ble_initialized = false;                                        /*蓝牙初始化标志*/
static uint16_t ble_gatt_receive_handle;                                    /*接受特征值句柄*/
static uint16_t ble_gatt_send_handle;                                       /*发送特征值句柄*/

/*蓝牙GAP事件处理函数*/
static int gap_event_handler(struct ble_gap_event *event, void *arg)
{
    if (event->type == BLE_GAP_EVENT_CONNECT) 
    {
        if (event->connect.status == 0) 
        {
            ESP_LOGI(TAG, "蓝牙连接成功");
            ble_initialized = false; //连接成功后，停止广播
        } 
        else 
        {
            ESP_LOGE(TAG, "蓝牙连接失败，错误码：%d", event->connect.status);
            if (!ble_initialized) 
            {
                start_advertising(); //重新开始广播
            }
        }
    } 
    else if (event->type == BLE_GAP_EVENT_DISCONNECT) 
    {
        ESP_LOGI(TAG, "蓝牙断开连接，原因：%d", event->disconnect.reason);
        if (!ble_initialized) 
        {
            start_advertising(); //重新开始广播
        }
    }
    return 0;
}

/*蓝牙GATT事件处理函数*/
static int ble_gatt_event_handler(uint16_t conn_handle, uint16_t attr_handle,
                               struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    if(ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR)
    {
        if(attr_handle == ble_gatt_send_handle)
        {
            //读取发送特征值时，返回一个示例数据
            const char *response = "Hello from ESP32!";
            os_mbuf_append(ctxt->om, response, strlen(response));
        }
    }
    else if(ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR)
    {
        if(attr_handle == ble_gatt_receive_handle)
        {
            if(ctxt->om->om_data[0] == 0x00)
            {
                LED0_TOGGLE();
            }
            else if(ctxt->om->om_data[0] == 0x01)
            {
                LED1_TOGGLE();
            }
        }
    }
    else
    {
        ESP_LOGW(TAG, "未知的GATT操作类型：%d", ctxt->op);
    }
    return 0;
}

/*蓝牙开始广播函数*/
void start_advertising(void)
{
    struct ble_hs_adv_fields adv_fields;
    memset(&adv_fields, 0, sizeof(adv_fields));
    adv_fields.name = (uint8_t *)MYBULE_NAME;
    adv_fields.name_len = strlen(MYBULE_NAME);
    adv_fields.name_is_complete = 1;
    adv_fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
    adv_fields.tx_pwr_lvl_is_present = 1;
    adv_fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;
    
    int rc = ble_gap_adv_set_fields(&adv_fields);
    if (rc != 0) 
    {
        ESP_LOGE(TAG, "设置广播数据失败，错误码：%d", rc);
        return;
    }

    //广播内容结构体
    const struct ble_gap_adv_params adv_params = {
        .conn_mode = BLE_GAP_CONN_MODE_UND,                             //设置为可连接模式
        .disc_mode = BLE_GAP_DISC_MODE_GEN,                             //设置为通用可发现模式
        .itvl_max = BLE_GAP_ADV_ITVL_MS(500),                           //设置最大广播间隔为500ms
        .itvl_min = BLE_GAP_ADV_ITVL_MS(200),                           //设置最小广播间隔为200ms
    };

    rc = ble_gap_adv_start(BLE_OWN_ADDR_PUBLIC, NULL, BLE_HS_FOREVER, &adv_params, gap_event_handler, NULL);          //开始广播                 

    if (rc != 0) {
        ESP_LOGE(TAG, "广播启动失败");
        return;
    }
    else
    {
        ESP_LOGI(TAG, "广播启动成功");
        ble_initialized = true;
    }
}

/*蓝牙初始化回调函数*/
static void ble_on_sync(void)
{
    start_advertising();
    ESP_LOGI(TAG, "蓝牙初始化完成，开始广播");
}


/*蓝牙GATT服务定义*/
static const struct ble_gatt_svc_def gatt_svcs[] = 
{
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(0x00FF),
        .includes = NULL,
        .characteristics = (struct ble_gatt_chr_def[])
        {
             
            
            //ESP32接受手机数据
            {
                .uuid = BLE_UUID16_DECLARE(0xFF01),
                .access_cb = ble_gatt_event_handler,
                .flags = BLE_GATT_CHR_F_WRITE,
                .val_handle = &ble_gatt_receive_handle,
                .arg = NULL,
            },
            //ESP32发送数据给手机
            {
                .uuid = BLE_UUID16_DECLARE(0xFF02),
                .access_cb = ble_gatt_event_handler,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
                .val_handle = &ble_gatt_send_handle,
                .arg = NULL,
            },
            {
               0, /* No more characteristics in this service. */
            },
        },
    },
    {
        0, /* No more services. */
    },
};

/*蓝牙监听任务*/
void host_task( void * )
{
    nimble_port_run();                                              /*循环监听NimBLE协议栈任务函数*/

}

/*蓝牙初始化函数*/
void ble_init(void)
{
    nimble_port_init();                                             /*初始化NimBLE蓝牙协议栈*/
    ble_svc_gap_init();                                             /*初始化GAP服务*/
    ble_svc_gatt_init();                                            /*初始化GATT服务*/
    ble_svc_gap_device_name_set(MYBULE_NAME);                       /*设置设备名称*/
    ble_gatts_count_cfg(gatt_svcs);                                 /*计算服务数量*/
    ble_gatts_add_svcs(gatt_svcs);                                  /*添加服务*/ 

    ble_hs_cfg.sync_cb = ble_on_sync;                               /*设置蓝牙初始化回调函数*/

    nimble_port_freertos_init(host_task);                          /*初始化NimBLE协议栈的FreeRTOS任务*/
}
