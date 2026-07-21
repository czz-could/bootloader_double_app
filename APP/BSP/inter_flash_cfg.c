#include "inter_flash_cfg.h"
#include <string.h>
#include <stdio.h>
#include "inter_flashif.h"
#include "usart.h"
static inter_flash_cfg_param_typeDef flash_cfg_param;

/**
 * @brief 读取参数分区配置
 * 
 * @return uint8_t 
 */
uint8_t inter_flash_cfg_get_param_sector()
{
    // 读取系统配置参数
    inter_flashif_read_page(INTER_FLASH_PARAM_ADDR, (uint8_t *)&flash_cfg_param, sizeof(flash_cfg_param));
    // 校验魔数标志
    if((flash_cfg_param.magic[0] != 0xAA) || (flash_cfg_param.magic[1] != 0xBB) ||
        (flash_cfg_param.magic[2] != 0xCC) || (flash_cfg_param.magic[3] != 0xDD))
    {
        printf("flash magic error\r\n");
        return 1;           // 魔数校验失败
    }

    // 计算校验和校验
    uint8_t checksum = inter_flash_checksum((uint8_t *)&flash_cfg_param, sizeof(flash_cfg_param) - 5);
		printf("checksum:%d\r\n",checksum);
    if(checksum != flash_cfg_param.checksum)
    {
        printf("flash checksum error\r\n");
        return 2;           // 校验和错误
    }

    return 0;
}


/**
 * @brief 获取Flash升级标志位
 * 
 * @return uint8_t 
 */
int8_t inter_flash_cfg_get_app_update_flag(void)
{
    uint8_t ret = inter_flash_cfg_get_param_sector();
    if(ret != 0)            // 读取参数失败
    {
        return -1;
    }

    // 返回升级标志
	return flash_cfg_param.ota_flag;
}

/**
 * @brief 设置升级标志位
 * @param flag 升级标志
 * @return uint8_t 
 */
int8_t inter_flash_cfg_set_app_update_flag(uint8_t flag)
{
    uint8_t ret = inter_flash_cfg_get_param_sector();
    if(ret != 0)            // 读取参数失败
    {
        printf("flash get update flag error\r\n");
        return -1;
    }
    flash_cfg_param.ota_flag = flag;

    // 重新计算校验和
    flash_cfg_param.checksum = inter_flash_checksum((uint8_t *)&flash_cfg_param, sizeof(flash_cfg_param) - 5);

    // 写入Flash页
    inter_flashif_smart_write_page(INTER_FLASH_PARAM_ADDR, (uint32_t *)&flash_cfg_param, sizeof(flash_cfg_param)/sizeof(uint32_t));

	return 0;
}

/**
 * @brief 获取当前运行APP分区编号
 * 
 * @return uint8_t 
 */
int8_t inter_flash_cfg_get_app_running_flash_number(void)
{
    uint8_t ret = inter_flash_cfg_get_param_sector();
    if(ret != 0)            // 读取参数失败
    {
        return -1;
    }

    // 返回运行分区编号
	return flash_cfg_param.ota_running_flash_number;
}

/**
 * @brief 设置当前运行APP分区编号
 * @param flag 分区编号
 * @return uint8_t 
 */
int8_t inter_flash_cfg_set_app_set_running_flash_number(uint8_t flag)
{
    uint8_t ret = inter_flash_cfg_get_param_sector();
    if(ret != 0)            // 读取参数失败
    {
        printf("flash get running flash number error\r\n");
        return -1;
    }
    flash_cfg_param.ota_running_flash_number = flag;

    // 重新计算校验和
    flash_cfg_param.checksum = inter_flash_checksum((uint8_t *)&flash_cfg_param, sizeof(flash_cfg_param) - 5);

    // 写入Flash页
    inter_flashif_smart_write_page(INTER_FLASH_PARAM_ADDR, (uint32_t *)&flash_cfg_param, sizeof(flash_cfg_param)/sizeof(uint32_t));

	return 0;
}


/**
 * @brief 获取下次待运行APP分区编号
 * 
 * @return uint8_t 
 */
int8_t inter_flash_cfg_get_app_next_running_flash_number(void)
{
    uint8_t ret = inter_flash_cfg_get_param_sector();
    if(ret != 0)            // 读取参数失败
    {
        return -1;
    }

    // 返回下次启动分区编号
	return flash_cfg_param.next_ota_need_flash_number;
}

/**
 * @brief 设置下次待运行APP分区编号
 * @param flag 分区编号
 * @return uint8_t 
 */
int8_t inter_flash_cfg_set_app_set_next_running_flash_number(uint8_t flag)
{
    uint8_t ret = inter_flash_cfg_get_param_sector();
    if(ret != 0)            // 读取参数失败
    {
        printf("flash get running flash number error\r\n");
        return -1;
    }
    flash_cfg_param.next_ota_need_flash_number = flag;

    // 重新计算校验和
    flash_cfg_param.checksum = inter_flash_checksum((uint8_t *)&flash_cfg_param, sizeof(flash_cfg_param) - 5);

    // 写入Flash页
    inter_flashif_smart_write_page(INTER_FLASH_PARAM_ADDR, (uint32_t *)&flash_cfg_param, sizeof(flash_cfg_param)/sizeof(uint32_t));

	return 0;
}



static void inter_flash_cfg_write_default(void)
{
    inter_flash_cfg_param_typeDef default_param;

    // 填充默认配置参数
    default_param.magic[0] = 0xAA;
    default_param.magic[1] = 0xBB;
    default_param.magic[2] = 0xCC;
    default_param.magic[3] = 0xDD;
    default_param.ota_bin_version = 0;
    default_param.ota_flag = 0;          // 默认无需升级
	default_param.ota_running_flash_number = 1; // 默认运行APP1分区
	default_param.next_ota_need_flash_number = 1;// 下次启动使用分区1
    default_param.checksum = 0;          // 先清空校验和，后续重新计算
    default_param.format[0] = 0;
    default_param.format[1] = 0;
	default_param.format[2] = 0;
	default_param.format[3] = 0;

    // 计算校验和：总长度减去末尾5字节（1字节checksum + 4字节format）
    default_param.checksum = inter_flash_checksum((uint8_t*)&default_param, sizeof(default_param) - 5);

    // 写入Flash参数分区整页
    inter_flashif_smart_write_page(INTER_FLASH_PARAM_ADDR, (uint32_t*)&default_param, sizeof(default_param) / 4);
}



/**
 * @brief Flash参数分区初始化
 * 
 * @return uint8_t 
 */
uint8_t inter_flash_cfg_init(void)
{
    // 检查结构体对齐，Flash按4字节写入，长度必须4整数倍
    if (sizeof(inter_flash_cfg_param_typeDef) % 4 != 0) {
        while(1) {
            printf("flash cfg param size error\r\n");
            HAL_Delay(1000);
        }
    }

    // 读取分区参数校验有效性
    uint8_t ret = inter_flash_cfg_get_param_sector();
    if (ret != 0) {
        // 魔数/校验和异常，写入默认参数
        printf("Param sector invalid, writing default...\r\n");
        inter_flash_cfg_write_default();

        // 重读校验初始化结果
        ret = inter_flash_cfg_get_param_sector();
        if (ret != 0) {
            // 写入默认参数失败，Flash硬件/地址故障卡死
            while(1) {
                printf("Failed to init param sector!\r\n");
                HAL_Delay(1000);
            }
        } else {
            printf("Param sector initialized successfully.\r\n");
        }
    }
    return 0;
}