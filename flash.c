#include <flash.h>

void flashUnlock() {
	while ((FLASH->SR & FLASH_SR_BSY) != 0 );     // Wait for the flash memory not to be busy
	if ((FLASH->CR & FLASH_CR_LOCK) != 0 ){       // Check if the controller is unlocked already
		FLASH->KEYR = FLASH_KEY1;            // Write the first key
		FLASH->KEYR = FLASH_KEY2;        // Write the second key
	}
}

void flashLock() {
	FLASH->CR |= FLASH_CR_LOCK;
}

/**
 * Erase one page of flash memory, setting all its contents to 1's.
 * It is necessary to call this function before writing 
 * to any address in flash memory.
 *
 * On the stm32f04x one page of flash is 1K
 *
 * *NOTE:* This function is dangerous, it does not check where it is
 * writing data to, so be *very* careful useing it.
 *
 * \param addr Starting address of the page to erase.
 * 
 * \returns \ref FlashError, an enum which gives the state of the flash
 * controller.
 */
FlashError flashErasePage(uint32_t addr) {
	FlashError status;
	FLASH->CR |= FLASH_CR_PER;              // Page erase operation
	FLASH->AR = addr;                  			// Set the address to the page to be written
	FLASH->CR |= FLASH_CR_STRT;							// Start the page erase

	while ((FLASH->SR & FLASH_SR_BSY) != 0);// Wait until page erase is done

	if ((FLASH->SR & FLASH_SR_EOP) != 0){   // If the end of operation bit is set
		FLASH->SR |= FLASH_SR_EOP;      	    // Clear it, the operation was successful
		status = FLASH_OK;
	}
	else if(FLASH->SR & FLASH_SR_PGERR){    // Otherwise there was an error condition
		FLASH->SR |= FLASH_SR_PGERR;    	    // Clear programming error
		status = FLASH_PRG_ERROR;
	}
	else{                                   // Must be Santa! No, write protection error.
		FLASH->SR |= FLASH_SR_WRPERR;
		status = FLASH_WRT_ERROR;
	}
	FLASH->CR &= ~FLASH_CR_PER;             //  Get out of page erase mode
	return status;
}

/**
 * Copy a page (1K) of flash. If erasing is enabled it will erase the page 
 * pointed to by dest_addr then copy the page pointed to by src_addr to dest_addr
 *
 * \param src_addr Address of page to copy from
 * \param dest_addr Adress of page to copy to
 * \param erase Erase destination? yes (true), no (false)
 *
 * \returns \ref FlashError stating whether every flash opperation was successfull 
 */
FlashError flashCopyPage(uint32_t src_addr, uint32_t dest_addr, bool erase) {
	flashUnlock();
	if(erase){
		flashErasePage(dest_addr);
	}

	for(int i=0; i<1024; i+=2){
		flashWrite_16(dest_addr+i, *((uint16_t*) src_addr+i));	
	}
	
	return FLASH_OK;
}

FlashError flashWrite_16(uint32_t addr, uint16_t data) {
	FlashError status;

	FLASH->CR |= FLASH_CR_PG;               // Programing mode
	*(__IO uint16_t*)(addr) = data;         // Write data

	while ((FLASH->SR & FLASH_SR_BSY) != 0);// Wait until the end of the operation

	if ((FLASH->SR & FLASH_SR_EOP) != 0){   // If the end of operation bit is set
		FLASH->SR |= FLASH_SR_EOP;          // Clear it, the operation was successful
		status = FLASH_OK;
	}
	else if(FLASH->SR & FLASH_SR_PGERR){    // Otherwise there was an error condition
		FLASH->SR |= FLASH_SR_PGERR;        // Clear programming error
		status = FLASH_PRG_ERROR;
	}
	else{                                   // Must be Santa! No, write protection error.
		FLASH->SR |= FLASH_SR_WRPERR;
		status = FLASH_WRT_ERROR;
	}
	return status;
}

FlashError flashWrite_32(uint32_t addr, uint32_t data){
	FlashError status;

	status = flashWrite_16(addr, (uint16_t) data);
	if(status !=FLASH_OK){
		return status;
	}
	status = flashWrite_16(addr+2, (uint16_t) data >> 16);

	return status;
}

void flashWriteMemBlock(uint32_t addr, uint8_t* data, uint16_t len){
	//writing in 16 bit incriments, advance by 2 chars
	int i;
	uint32_t word_addr = addr;
	for(i=0; i<len-1; i+=2){ 
		//archatecture is little-endian (i think)
		uint16_t word = data[i] | (data[i+1] << 8);
		flashWrite_16(word_addr, word);
		word_addr+=2;
	}
	//if odd pad last byte with a null character
	if(len & 1){
		uint16_t word = data[i] | (0x00 << 8);
		flashWrite_16(word_addr, word);
	}
	else {
		//add null character
		flashWrite_16(word_addr, 0x0000);
	}
}
