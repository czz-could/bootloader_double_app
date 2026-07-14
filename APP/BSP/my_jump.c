#include "my_jump.h"
#include "inter_flash_cfg.h"
#include <stdio.h>
#include "stm32f1xx_hal.h"

typedef void (*Pfunction)(void);

//uint32_t first_page_address = 0x08008000;
//uint32_t pages_to_erase = 240;

void Jump_To_App(void)
{
    uint32_t app_start = USER_APP1_STRAT_ADDR;
    uint32_t app_stack = 0;
    uint32_t reset_handler = 0;
    // 获取下次需要运行的APP分区编号
    int8_t app_num = inter_flash_cfg_get_app_next_running_flash_number();
    if (app_num != 2)
    {
        // 下次分区无效则读取当前运行分区
        app_num = inter_flash_cfg_get_app_running_flash_number();
    }
    // 分区2则切换至APP2起始地址
    if (app_num == 2)
    {
        app_start = USER_APP2_STRAT_ADDR;
    }

    // 读取向量表栈顶地址与复位中断函数地址
    app_stack = *(__IO uint32_t*)app_start;
    reset_handler = *(__IO uint32_t*)(app_start + 4);
    // 修正复位函数地址偏移
    if (reset_handler >= USER_APP1_STRAT_ADDR && reset_handler < USER_APP2_STRAT_ADDR)
    {
        reset_handler = app_start + (reset_handler - USER_APP1_STRAT_ADDR);
    }

    printf("Jump target start=0x%08lx stack=0x%08lx reset=0x%08lx\r\n", app_start, app_stack, reset_handler);
    // 判断栈指针是否在64KB SRAM区间内(STM32F103ZET6)，复位地址是否在Flash合法区间
    if (app_stack >= 0x20000000 && app_stack < 0x20010000 && reset_handler >= 0x08000000 && reset_handler < 0x08080000)
    {
        __disable_irq();                // 关闭全局中断
        SCB->VTOR = app_start;          // 重定位向量表偏移地址
        __DSB();                        // 数据同步屏障
        __ISB();                        // 指令同步屏障
        __set_MSP(app_stack);           // 设置主堆栈指针
        // 跳转到APP复位入口函数
        ((void (*)(void))reset_handler)();
    }
    else
    {
        printf("Jump aborted: invalid stack/reset vector\r\n");
    }
}

int8_t Erase_Apparea(void)
{
    uint32_t target_page_address = 0;
		uint32_t pages_to_erase = 0;
    // 获取当前正在运行的APP分区
    int8_t app_num = inter_flash_cfg_get_app_running_flash_number();
    // 擦除对立分区：运行APP1则擦APP2，运行APP2则擦APP1
    if (app_num == 1)
    {
        target_page_address = USER_APP2_STRAT_ADDR;
				pages_to_erase = USER_APP2_STRAT_SIZE/2;
    }
    else
    {
        target_page_address = USER_APP1_STRAT_ADDR;
				pages_to_erase = USER_APP1_STRAT_SIZE/2;
    }

    __disable_irq();
    HAL_FLASH_Unlock();
    
    FLASH_EraseInitTypeDef erase = {0};
    erase.TypeErase   = FLASH_TYPEERASE_PAGES;
    erase.PageAddress = target_page_address;
    erase.NbPages     = pages_to_erase;          // 共计240页，480KB
    
    uint32_t page_error = 0;
    if (HAL_FLASHEx_Erase(&erase, &page_error) != HAL_OK) {
        printf("Erase Err at page address: 0x%lx\r\n", page_error);
        HAL_FLASH_Lock();
        __enable_irq();
        return -1;
    }
    
    printf("Erase OK\r\n");
    HAL_FLASH_Lock();
    __enable_irq();
    return 0;
}