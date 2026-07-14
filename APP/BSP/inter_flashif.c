#include "inter_flashif.h"
#include "stm32f1xx_hal_flash_ex.h"
#include "common.h"


#ifndef FLASH_PAGE_SIZE
#define FLASH_PAGE_SIZE  2048U
#endif
/**
 * @brief 计算累加校验和
 * 
 * @param data 需要校验的数据缓冲区
 * @param len 数据长度
 * @return uint8_t 校验和结果
 */
uint8_t inter_flash_checksum(uint8_t *data, uint32_t len)
{
    uint8_t checksum = 0;
    for(uint32_t i = 0; i < len; i++)
    {
        checksum += data[i];
    }
    return checksum & 0xFF;
}


/**
 * @brief 片内flash分页读取
 * 
 * @param addr 读取数据起始地址
 * @param buf 读取数据存放缓冲区
 * @param len 读取字节长度
 * @return uint8_t 0成功 1长度超限失败
 */
uint8_t inter_flashif_read_page(uint32_t addr, uint8_t *buf, uint32_t len)
{
    uint32_t flash_addr = addr;

    if(len > FLASH_PAGE_SIZE)          //长度超过单页大小
    {
        return 1;   
    }

    for(uint32_t i = 0; i < len; i++)
    {
        buf[i] = *(volatile uint8_t *)(flash_addr + i);
    }

    return 0;
}

/**
 * @brief 片内flash智能分页写入（无返回错误判断）
 * 
 * @param addr 写入起始地址(4字节对齐)
 * @param buf 待写入32位数据缓存
 * @param len 待写入word数量
 * @return uint8_t 固定返回0
 */
uint8_t inter_flashif_smart_write_page(uint32_t addr, uint32_t *buf, uint32_t len)
{
    FLASH_EraseInitTypeDef user_flash = {0};
    
    /* 解锁Flash */
    HAL_FLASH_Unlock();
	
    /* 配置擦除单页参数 */
    user_flash.TypeErase = FLASH_TYPEERASE_PAGES;
    user_flash.PageAddress = addr;
    user_flash.NbPages = 1;
	
    //说明：仅擦除当前一页，页数范围最小1页，最大受Flash总页限制

    uint32_t Error = 0;
    HAL_FLASHEx_Erase(&user_flash, &Error);

    for(uint32_t i = 0; i < len; i++)
    {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr+i*4, buf[i]); 
    }

    /* 上锁Flash */
    HAL_FLASH_Lock();
	return 0;
}

/**
 * @brief 片内flash标准分页写入（带擦除、写入错误检测）
 * 
 * @param addr 写入起始地址(4字节对齐)
 * @param buf 待写入32位数据缓存
 * @param len 待写入word数量
 * @return uint8_t 0成功 1擦除失败 2编程写入失败
 */
uint8_t inter_flashif_write_page(uint32_t addr, uint32_t *buf, uint32_t len)
{   
    FLASH_EraseInitTypeDef user_flash = {0};
    uint32_t Error = 0;

    HAL_FLASH_Unlock();

    user_flash.TypeErase = FLASH_TYPEERASE_PAGES;
    user_flash.PageAddress = addr;
    user_flash.NbPages = 1;
    if (HAL_FLASHEx_Erase(&user_flash, &Error) != HAL_OK) {
        HAL_FLASH_Lock();
        return 1;
    }

    for(uint32_t i = 0; i < len; i++)
    {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr+i*4, buf[i]) != HAL_OK) {
            HAL_FLASH_Lock();
            return 2;
        }
    }
    /* 上锁Flash */
    HAL_FLASH_Lock();
    return 0;
}



#define TEST_ADDR       0x8007400

void inter_flash_test(void)
{
    uint8_t write[] = "1234FFO222-?";
    
    uint8_t read[32] = {0};
    
    inter_flashif_write_page(TEST_ADDR, (uint32_t *)write, 12);

    inter_flashif_read_page(TEST_ADDR, read, 12);

    dump_hex(read, 12, 16);
}