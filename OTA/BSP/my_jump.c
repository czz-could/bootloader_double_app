#include "my_jump.h"
#include "usart.h"  // 添加串口输出支持

typedef void (*Pfunction)(void);

//uint32_t first_page_address = 0x08008000;
//uint32_t pages_to_erase = 240;

void Jump_To_App(void)
{
		uint32_t APP_START = 0;
		uint32_t app_stack = 0;
		uint32_t reset_handler = 0;
		int8_t app_num = inter_flash_cfg_get_app_next_running_flash_number();
		if (app_num != 2)
		{
			app_num = inter_flash_cfg_get_app_running_flash_number();
		}
		if(app_num == 1)
		{
			APP_START = USER_APP1_STRAT_ADDR;
		}
		else
		{
			APP_START = USER_APP2_STRAT_ADDR;
		}

		app_stack = *(__IO uint32_t*)APP_START;
		reset_handler = *(__IO uint32_t*)(APP_START + 4);
		if (reset_handler >= USER_APP1_STRAT_ADDR && reset_handler < USER_APP2_STRAT_ADDR)
		{
			reset_handler = APP_START + (reset_handler - USER_APP1_STRAT_ADDR);
		}

		printf("Jump target start=0x%08lx stack=0x%08lx reset=0x%08lx\r\n", APP_START, app_stack, reset_handler);
    
    // 检查栈顶指针是否在 64KB SRAM 范围内 (STM32F103ZET6)
    if (app_stack >= 0x20000000 && app_stack < 0x20010000 && reset_handler >= 0x08000000 && reset_handler < 0x08080000)
    {
        __disable_irq();                // 关闭中断
        SCB->VTOR = APP_START;          // 设置向量表偏移
        __DSB();                        // 数据同步屏障
        __ISB();                        // 指令同步屏障
        __set_MSP(app_stack);           // 设置主栈指针
				uint8_t peek[64];
				// 假设版本字符串在固件偏移 0x100 附近，可以多试几个偏移
				for (uint32_t off = 0x100; off < 0x400; off += 0x40) {
						inter_flashif_read_page(APP_START + off, peek, 64);
						printf("APP2+0x%lx: %s\r\n", off, peek);
				}
					uint8_t full_dump[256];
				inter_flashif_read_page(APP_START, full_dump, 256);
				printf("=== APP2 FULL DUMP ===\r\n");
				dump_hex(full_dump, 256, 16);
				printf("=== END DUMP ===\r\n");
        // 跳转到应用程序
        ((void (*)(void))reset_handler)();
    }
    else
    {
        printf("Jump aborted: invalid stack/reset vector\r\n");
    }
}

int8_t Erase_Apparea(void)
{
		uint32_t first_page_address = 0;
		uint32_t pages_to_erase = 0;
		int8_t ota_running_flash_nember = inter_flash_cfg_get_app_running_flash_number();
		if(ota_running_flash_nember == 1)
		{
			first_page_address = USER_APP2_STRAT_ADDR;
			pages_to_erase = USER_APP2_START_SIZE / FLASH_PAGE_SIZE;
			printf("running app1，Erase app2\r\n");
			HAL_Delay(1000);
		}
		else
		{
			first_page_address = USER_APP1_STRAT_ADDR;
			pages_to_erase = USER_APP1_START_SIZE / FLASH_PAGE_SIZE;
			printf("running app2，Erase app1\r\n");	
			HAL_Delay(1000);			
		}
    __disable_irq();
    HAL_FLASH_Unlock();
    
    FLASH_EraseInitTypeDef erase = {0};
    erase.TypeErase   = FLASH_TYPEERASE_PAGES;
    erase.PageAddress = first_page_address;
    erase.NbPages     = pages_to_erase;          // 240KB
    
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