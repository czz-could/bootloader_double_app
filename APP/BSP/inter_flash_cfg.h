#include "main.h"

#ifndef __INTER_FLASH_H__
#define __INTER_FLASH_H__

#pragma pack(1)        //设置字节对齐方式为1字节
#define USER_APP1_STRAT_ADDR 0x08008000
#define USER_APP2_STRAT_ADDR 0x08044000
#define USER_APP1_STRAT_SIZE 240
#define USER_APP2_STRAT_SIZE 240



#pragma pack(1)        //设置字节对齐方式为1字节
typedef struct {
    uint8_t magic[4];                   //magic number 0xAABBCCDD
    uint32_t ota_bin_version;           //将要升级的文件版本
    uint8_t ota_flag;   //升级标志
		uint8_t ota_running_flash_number;   // 正在运行的分区
		uint8_t next_ota_need_flash_number; // 需要升级的分区
    
    uint8_t checksum; //参数校验
    //占位
    uint8_t format[4]; //占位符

}inter_flash_cfg_param_typeDef;


#pragma pack()        //恢复默认字节对齐方式


uint8_t inter_flash_cfg_init(void);
int8_t inter_flash_cfg_get_app_update_flag(void);
int8_t inter_flash_cfg_set_app_update_flag(uint8_t flag);
int8_t inter_flash_cfg_get_app_running_flash_number(void);
int8_t inter_flash_cfg_set_app_set_running_flash_number(uint8_t flag);
int8_t inter_flash_cfg_get_app_next_running_flash_number(void);
int8_t inter_flash_cfg_set_app_set_next_running_flash_number(uint8_t flag);

#endif /* __INTER_FLASH_H__ */
