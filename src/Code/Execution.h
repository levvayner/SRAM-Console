#define HSRAM_ADDR            0x20000000u /**< HSRAM base address */
#define HSRAM_SIZE            0x00030000u /* 192 kB */
#define DBL_TAP_PTR ((volatile uint32_t *)(HSRAM_ADDR + HSRAM_SIZE - 4))
#define RESET_CONTROLLER RSTC
#define DBL_TAP_MAGIC 0xf01669ef // Randomly selected, adjusted to have first and last bit set
#define DBL_TAP_MAGIC_QUICK_BOOT 0xf02669ef

//https://www.eevblog.com/forum/microcontrollers/stuck-on-how-to-load-application-from-custom-arm-bootloader-samd51/
static void startApp(int32_t appStartAddress) {
    uint32_t app_start_address;

    /* Load the Reset Handler address of the application */
    app_start_address = (*(uint32_t *)(appStartAddress + 4)  );              // appStartAddress is 0x8000

    /**
     * Test reset vector of application @appStartAddress+4
     * Sanity check on the Reset_Handler address
     */
    if (app_start_address < IFLASH0_ADDR || app_start_address > IFLASH_SIZE + IFLASH0_ADDR) {           // This check is passed
        /* Stay in bootloader */
        Serial.print("Code execution failed. Address 0x"); Serial.print(app_start_address,HEX); 
        Serial.print(" is out of valid range  0x");Serial.print(IFLASH0_ADDR, HEX); Serial.print(" to 0x");
        Serial.println(IFLASH_SIZE + IFLASH0_ADDR, HEX);
        return ;
    } else {
        Serial.print("App provided valid address 0x"); Serial.println(app_start_address, HEX);
    }
    app_start_address += IFLASH0_SIZE;
    if (RESET_CONTROLLER->RSTC_SR & 0x1 << 17) { //reset in progress
        *DBL_TAP_PTR = 0;
    }
    else if (*DBL_TAP_PTR == DBL_TAP_MAGIC) {
        *DBL_TAP_PTR = 0;
        return; // stay in bootloader                                          // This is to double tab to go into SAMBA programing mode
    }
    else {
        if (*DBL_TAP_PTR != DBL_TAP_MAGIC_QUICK_BOOT) {
            *DBL_TAP_PTR = DBL_TAP_MAGIC;
            delay(500);
        }
        *DBL_TAP_PTR = 0;
    }

    /* Rebase the Stack Pointer */
    __set_MSP(*(uint32_t *)appStartAddress);

    /* Rebase the vector table base address */
    SCB->VTOR = ((uint32_t)appStartAddress & SCB_VTOR_TBLOFF_Msk);
    Serial.print("Jumping into application at 0x"); Serial.println(app_start_address, HEX);
    /* Jump to application Reset Handler in the application */
    //asm("bx %0" ::"r"(app_start_address)); 
                                                             // This is where we leave off
    graphics.~VRAM();
    void (*ptr)();
    ptr =  (void (*)(void))(unsigned *)(*(unsigned *)(app_start_address));
    ptr();

    //if we return
    graphics = VRAM();
}
