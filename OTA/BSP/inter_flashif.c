#include "inter_flashif.h"
#include "stm32f1xx_hal_flash_ex.h"
#include "common.h"


#ifndef FLASH_PAGE_SIZE
#define FLASH_PAGE_SIZE  2048U
#endif
/**
 * @brief 数据校验和计算
 * 
 * @param data 需要校验的数据缓冲区
 * @param len 数据长度
 * @return uint8_t 8位校验和结果
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
 * @brief 片内Flash读取一页数据
 * 
 * @param addr 读取数据起始地址
 * @param buf 读取数据存放缓冲区
 * @param len 需要读取的数据字节长度
 * @return uint8_t 0成功 1长度超限失败
 */
uint8_t inter_flashif_read_page(uint32_t addr, uint8_t *buf, uint32_t len)
{
    uint32_t flash_addr = addr;

    if(len > FLASH_PAGE_SIZE)          //数据长度超过单页大小
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
 * @brief 片内Flash简易写入一页（无写入错误返回判断）
 * 
 * @param addr 写入起始地址(地址必须4字节对齐)
 * @param buf 待写入数据缓冲区(32位字数组)
 * @param len 待写入数据字数(1个字=4字节)
 * @return uint8_t 固定返回0，不做错误检测
 */
uint8_t inter_flashif_smart_write_page(uint32_t addr, uint32_t *buf, uint32_t len)
{
    FLASH_EraseInitTypeDef user_flash = {0};  //定义Flash擦除配置结构体
    
    /* 解锁Flash操作 */
    HAL_FLASH_Unlock();
	
    /* 配置单页擦除参数 */
    user_flash.TypeErase = FLASH_TYPEERASE_PAGES;
    user_flash.PageAddress = addr;
    user_flash.NbPages = 1;
	
    //说明：擦除一页后才能写入，擦除范围为单页全部地址

    uint32_t Error = 0;                    //存储擦除异常页编号，正常擦除则无错误页
    HAL_FLASHEx_Erase(&user_flash, &Error);  //执行页擦除操作

    for(uint32_t i = 0; i < len; i++)
    {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr+i*4, buf[i]); 
    }

    /* 锁定Flash，禁止后续擦写 */
    HAL_FLASH_Lock();
	return 0;
}

/**
 * @brief 片内Flash标准写入一页（带完整错误判断）
 * 
 * @param addr 写入起始地址(地址必须4字节对齐)
 * @param buf 待写入数据缓冲区(32位字数组)
 * @param len 待写入数据字数(1个字=4字节)
 * @return uint8_t 0成功 1擦除失败 2写入失败
 */
uint8_t inter_flashif_write_page(uint32_t addr, uint32_t *buf, uint32_t len)
{   
    FLASH_EraseInitTypeDef user_flash = {0};
    uint32_t Error = 0;

    HAL_FLASH_Unlock();

    user_flash.TypeErase = FLASH_TYPEERASE_PAGES;
    user_flash.PageAddress = addr;
    user_flash.NbPages = 1;
    //执行页擦除，擦除失败直接上锁并返回错误码1
    if (HAL_FLASHEx_Erase(&user_flash, &Error) != HAL_OK) {
        HAL_FLASH_Lock();
        return 1;
    }

    //循环按32位字写入，单字写入失败立即上锁返回错误码2
    for(uint32_t i = 0; i < len; i++)
    {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr+i*4, buf[i]) != HAL_OK) {
            HAL_FLASH_Lock();
            return 2;
        }
    }

    HAL_FLASH_Lock();
    return 0;
}



#define TEST_ADDR       0x8007400

/**
 * @brief Flash读写功能测试函数
 */
void inter_flash_test(void)
{
    uint8_t write[] = "1234FFO222-?";
    
    uint8_t read[32] = {0};
    
    //写入12个字节数据，转换为32位字指针传入，长度为12个字
    inter_flashif_write_page(TEST_ADDR, (uint32_t *)write, 12);

    //读取12字节数据到缓冲区
    inter_flashif_read_page(TEST_ADDR, read, 12);

    //打印16进制数据
    dump_hex(read, 12, 16);
}