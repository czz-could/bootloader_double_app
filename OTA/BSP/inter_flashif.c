#include "inter_flashif.h"
#include "stm32f1xx_hal_flash_ex.h"
#include "common.h"


#ifndef FLASH_PAGE_SIZE
#define FLASH_PAGE_SIZE  2048U
#endif
/**
 * @brief �ۼ�У��ͼ�����
 * 
 * @param data ��ҪУ�������
 * @param len ���ݳ���
 * @return uint8_t У���
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
 * @brief �ڲ�flash���ݶ�ȡ
 * 
 * @param addr ��ȡ���ݵĵ�ַ
 * @param buf ���ݴ�Ż�����
 * @param len ��ȡ���ݵĳ���
 * @return uint8_t 
 */
uint8_t inter_flashif_read_page(uint32_t addr, uint8_t *buf, uint32_t len)
{
    uint32_t flash_addr = addr;

    if(len > FLASH_PAGE_SIZE)          //���ݳ���
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
 * @brief �ڲ�flash����д��
 * 
 * @param addr д�����ݵĵ�ַ(����4�ֽڶ���)
 * @param buf ��Ҫд�����ݵĻ�����
 * @param len  д�����ݳ���
 * @return uint8_t 
 */
uint8_t inter_flashif_smart_write_page(uint32_t addr, uint32_t *buf, uint32_t len)
{
    FLASH_EraseInitTypeDef user_flash = {0};  //���� FLASH_EraseInitTypeDef �ṹ��Ϊ My_Flash
    
    /* ���� */
    HAL_FLASH_Unlock();
	
    /* ������ҳ */
    user_flash.TypeErase = FLASH_TYPEERASE_PAGES;
    user_flash.PageAddress = addr;
    user_flash.NbPages = 1;
	
      //˵��Ҫ������ҳ�����˲���������Min_Data = 1��Max_Data =(���ҳ��-��ʼҳ��ֵ)֮���ֵ

    uint32_t Error = 0;                    //����PageError,������ִ�����������ᱻ����Ϊ������FLASH��ַ
    HAL_FLASHEx_Erase(&user_flash, &Error);  //���ò�����������

    for(uint32_t i = 0; i < len; i++)
    {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr+i*4, buf[i]); 
    }

    /* ���� */
    HAL_FLASH_Lock();
	return 0;
}

/**
 * @brief �ڲ�flash����д��
 * 
 * @param addr д�����ݵĵ�ַ(����4�ֽڶ���)
 * @param buf ��Ҫд�����ݵĻ�����
 * @param len  д�����ݳ���
 * @return uint8_t 
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
