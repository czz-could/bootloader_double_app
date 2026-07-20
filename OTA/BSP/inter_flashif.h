#include "main.h"
#include "header.h"

#ifndef __INTER_FLASHIF_H__
#define __INTER_FLASHIF_H__


#define INTER_FLASH_PARAM_ADDR      												(0x8006000U)            //参数存储区地址起始地址
#define INTER_FLASH_APP_ADDR        												(0x8008000U)            //APP程序地址

//#define INTER_FLASH_OTAFLAG_ADDR      								 			(0x8006000U)            //参数存储区OTA升级标志起始地址
//#define INTER_FLASH_SET_RUNNING_FLASH_NUMBER_ADDR      			(0x8006001U)            //参数存储区正在运行的分区起始地址
//#define INTER_FLASH_SET_NEXT_RUNNING_FLASH_NEMBER_ADDR      (0x8006002U)       //参数存储区下一次运行的分区起始地址

void inter_flash_test(void);
uint8_t inter_flashif_smart_write_page(uint32_t addr, uint32_t *buf, uint32_t len);
HAL_StatusTypeDef inter_flashif_write_page(uint32_t addr, uint32_t *buf, uint32_t len);
uint8_t inter_flashif_read_page(uint32_t addr, uint8_t *buf, uint32_t len);
uint8_t inter_flash_checksum(uint8_t *data, uint32_t len);
#endif /* __INTER_FLASHIF_H__ */
