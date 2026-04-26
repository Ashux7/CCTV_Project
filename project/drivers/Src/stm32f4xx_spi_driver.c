/*
 * stm32f4xx_spi_driver.c
 *
 *  Created on: 14-Mar-2026
 *      Author: saura
 */
#include<stm32f4xx_spi_driver.h>
#include <stddef.h>

static void  spi_txe_interrupt_handle(SPI_Handle_t *pSPIHandle);
static void  spi_rxne_interrupt_handle(SPI_Handle_t *pSPIHandle);
static void  spi_ovr_err_interrupt_handle(SPI_Handle_t *pSPIHandle);
/*********************************************************************
 * @fn      		  - SPI_Init
 *
 * @brief             - Initializes the SPI peripheral according to the
 *                      configuration parameters in the SPI handle structure.
 *
 * @param[in]         - pSPIHandle: Pointer to the SPI handle structure
 *                      containing configuration (mode, bus config, speed,
 *                      data frame format, clock polarity/phase, etc.).
 *
 * @return            - None
 *
 * @Note              - Must enable peripheral clock before calling.
 *********************************************************************/
void SPI_Init(SPI_Handle_t *pSPIHandle) {
	// Temporary register to hold our configuration starting from 0
	uint32_t tempreg = 0;
	SPI_PeriClockControl(pSPIHandle->pSPIx,ENABLE);

	// 1. Configure the Device Mode (Master/Slave)
	tempreg |= (pSPIHandle->SPIConfig.SPI_DeviceMode << SPI_CR1_MSTR);

	// 2. Configure the Bus Mode
	if(pSPIHandle->SPIConfig.SPI_BusConfig == SPI_BUS_CONFIG_FD) {
		// Full Duplex: BIDIMODE should be cleared
		tempreg &= ~(1 << SPI_CR1_BIDIMODE);

	} else if (pSPIHandle->SPIConfig.SPI_BusConfig == SPI_BUS_CONFIG_HD) {
		// Half Duplex: BIDIMODE should be set
		tempreg |= (1 << SPI_CR1_BIDIMODE);

	} else if (pSPIHandle->SPIConfig.SPI_BusConfig == SPI_BUS_CONFIG_SIMPLEX_RXONLY) {
		// Simplex RX Only: BIDIMODE cleared, RXONLY set
		tempreg &= ~(1 << SPI_CR1_BIDIMODE);
		tempreg |= (1 << SPI_CR1_RXONLY);
	}

	// 3. Configure the SPI Clock Speed (Baud Rate)
	tempreg |= (pSPIHandle->SPIConfig.SPI_SclkSpeed << SPI_CR1_BR);

	// 4. Configure the Data Frame Format (8-bit or 16-bit)
	tempreg |= (pSPIHandle->SPIConfig.SPI_DFF << SPI_CR1_DFF);

	// 5. Configure CPOL, CPHA, and SSM
	tempreg |= (pSPIHandle->SPIConfig.SPI_CPOL << SPI_CR1_CPOL);
	tempreg |= (pSPIHandle->SPIConfig.SPI_CPHA << SPI_CR1_CPHA);
	tempreg |= (pSPIHandle->SPIConfig.SPI_SSM << SPI_CR1_SSM);

	// 6. Write the fully constructed configuration to the hardware register
	pSPIHandle->pSPIx->CR1 = tempreg;
}
/*********************************************************************
 * @fn      		  - SPI_DeInit
 *
 * @brief             - Resets the SPI peripheral registers to their default
 *                      reset values.
 *
 * @param[in]         - pSPIx: Base address of the SPI peripheral (SPI1, SPI2, etc.).
 *
 * @return            - None
 *
 * @Note              - Uses RCC reset mechanism.
 *********************************************************************/
void SPI_DeInit(SPI_RegDef_t *pSPIx){
 if(pSPIx == SPI1){
 SPI1_REG_RESET();
 }else if(pSPIx == SPI2){
	 SPI2_REG_RESET();
  }else if(pSPIx == SPI3){
		 SPI3_REG_RESET();
	}
}
uint8_t SPI_GetFlagStatus(SPI_RegDef_t *pSPIx,uint32_t Flagname){
	if (pSPIx->SR & Flagname){
		return FLAG_SET;
	}
	return FLAG_RESET;
}

/*********************************************************************
 * @fn      		  - SPI_PeriClockControl
 *
 * @brief             - Enables or disables the peripheral clock for a given SPI port.
 *
 * @param[in]         - pSPIx: Base address of the SPI peripheral.
 * @param[in]         - EnorDi: Enable or Disable macro (ENABLE/DISABLE).
 *
 * @return            - None
 *********************************************************************/
void SPI_PeriClockControl(SPI_RegDef_t *pSPIx,uint8_t EnorDi){
	if(EnorDi == ENABLE)
	{   if(pSPIx == SPI1){
		SPI1_PCLK_EN();
	       }
	else if(pSPIx == SPI2){
		SPI2_PCLK_EN();
	       }
	else if(pSPIx == SPI3){
		SPI3_PCLK_EN();
	       }
	}else{
		if(pSPIx == SPI1){
				SPI1_PCLK_DI();
			       }
		else if(pSPIx == SPI2){
				SPI2_PCLK_DI();
			       }
		else if(pSPIx == SPI3){
				SPI3_PCLK_DI();
			       }
	}

}

/*********************************************************************
 * @fn      		  - SPI_SendData
 *
 * @brief             - Sends data over SPI in blocking mode.
 *
 * @param[in]         - pSPIx: Base address of the SPI peripheral.
 * @param[in]         - pTxBuffer: Pointer to transmit buffer.
 * @param[in]         - Len: Length of data to send.
 *
 * @return            - None
 *
 * @Note              - This is a blocking call; waits until TXE flag is set.
 *********************************************************************/
void SPI_SendData(SPI_RegDef_t *pSPIx, uint8_t *pTxBuffer,uint32_t Len){
   while (Len > 0){
	   //1. wait untill TXE is set
	   while(SPI_GetFlagStatus(pSPIx, SPI_TXE_FLAG) == FLAG_RESET);
	   //2.  check the DFF bit in CR1
	   if((pSPIx-> CR1 & (1 << SPI_CR1_DFF) )){
		   //load the data in the dr
		   pSPIx->DR = *((uint16_t*)pTxBuffer);
		   Len--;
		   Len--;
		   (uint16_t*)pTxBuffer++;
	   }else{
	         *((volatile uint8_t *)&pSPIx->DR) = *pTxBuffer;
	         Len--; // decrement once because data is 8-bit wide
	         pTxBuffer++;

	   }
   }
}

/*********************************************************************
 * @fn      		  - SPI_ReceiveData
 *
 * @brief             - Receives data over SPI in blocking mode.
 *
 * @param[in]         - pSPIx: Base address of the SPI peripheral.
 * @param[in]         - pRxBuffer: Pointer to receive buffer.
 * @param[in]         - Len: Length of data to receive.
 *
 * @return            - None
 *
 * @Note              - This is a blocking call; waits until RXNE flag is set.
 *********************************************************************/
void SPI_ReceiveData(SPI_RegDef_t *pSPIx,  uint8_t *pRxBuffer, uint32_t Len)
{
	while(Len > 0)
		{
			//1. wait until RXNE is set
			while(SPI_GetFlagStatus(pSPIx,SPI_RXNE_FLAG)  == (uint8_t)FLAG_RESET );

			//2. check the DFF bit in CR1
			if( (pSPIx->CR1 & ( 1 << SPI_CR1_DFF) ) )
			{
				//16 bit DFF
				//1. load the data from DR to Rxbuffer address
				 *((uint16_t*)pRxBuffer) = pSPIx->DR ;
				Len--;
				Len--;
				(uint16_t*)pRxBuffer++;
			}else
			{
				//8 bit DFF
				*(pRxBuffer) = pSPIx->DR ;
				Len--;
				pRxBuffer++;
			}
		}

}



/*********************************************************************
 * @fn      		  - SPI_InterruptConfig
 *
 * @brief             - Configures the interrupt for a specific SPI peripheral.
 *
 * @param[in]         - IRQNumber: Interrupt request number associated with SPI.
 * @param[in]         - EnorDi: Enable or Disable macro (ENABLE/DISABLE).
 *
 * @return            - None
 *
 * @Note              - NVIC registers are configured in this function.
 *********************************************************************/
void SPI_InterruptConfig(uint8_t IRQNumber,uint8_t EnorDi){
	if(EnorDi == ENABLE)
	  {
		  if(IRQNumber <=31){
	         *NVIC_ISER0 |= (1 << IRQNumber);

		  }else if(IRQNumber >31 && IRQNumber< 64){
			  *NVIC_ISER1 |= (1 << IRQNumber%32);
		  }else if(IRQNumber >= 64 && IRQNumber < 96){
			  *NVIC_ISER2 |= (1 << IRQNumber%32);
		  }
	  }else{
		  if(IRQNumber <=31){
		          *NVIC_ICER0 |= (1 << IRQNumber);
		          }else if(IRQNumber >31 && IRQNumber< 64){
		 		  *NVIC_ICER1 |= (1 << IRQNumber%32);
		 	  }else if(IRQNumber >= 64 && IRQNumber < 96){
		 		  *NVIC_ICER2 |= (1 << IRQNumber%32);
		 	  }

		  }

}

/*********************************************************************
 * @fn      		  - SPI_IRQHandling
 *
 * @brief             - Handles the interrupt triggered by the SPI peripheral.
 *
 * @param[in]         - pHandle: Pointer to SPI handle structure.
 *
 * @return            - None
 *
 * @Note              - Clears interrupt flags and calls application callback.
 *********************************************************************/
void SPI_IRQHandling(SPI_Handle_t *pHandle){
	uint8_t temp1 , temp2;
		//first lets check for TXE
		temp1 = pHandle->pSPIx->SR & ( 1 << SPI_SR_TXE);
		temp2 = pHandle->pSPIx->CR2 & ( 1 << SPI_CR2_TXEIE);

		if( temp1 && temp2)
		{
			//handle TXE
			spi_txe_interrupt_handle(pHandle);
		}

		// check for RXNE
		temp1 = pHandle->pSPIx->SR & ( 1 << SPI_SR_RXNE);
		temp2 = pHandle->pSPIx->CR2 & ( 1 << SPI_CR2_RXNEIE);

		if( temp1 && temp2)
		{
			//handle RXNE
			spi_rxne_interrupt_handle(pHandle);
		}

		// check for ovr flag
		temp1 = pHandle->pSPIx->SR & ( 1 << SPI_SR_OVR);
		temp2 = pHandle->pSPIx->CR2 & ( 1 << SPI_CR2_ERRIE);

		if( temp1 && temp2)
		{
			//handle ovr error
			spi_ovr_err_interrupt_handle(pHandle);
		}


}

/*********************************************************************
 * @fn      		  - SPI_IRQPriorityConfig
 *
 * @brief             - Configures the priority of a given SPI interrupt.
 *
 * @param[in]         - IRQNumber: Interrupt request number.
 * @param[in]         - IRQPriority: Priority level for the interrupt.
 *
 * @return            - None
 *
 * @Note              - NVIC priority registers are configured in this function.
 *********************************************************************/
void SPI_IRQPriorityConfig(uint8_t IRQNumber,uint32_t IRQPriority){
	//1. first lets find out the ipr register
		uint8_t iprx = IRQNumber / 4;
		uint8_t iprx_section  = IRQNumber %4 ;

		uint8_t shift_amount = ( 8 * iprx_section) + ( 8 - NO_PR_BITS_IMPLEMENTED) ;

		*(  NVIC_PR_BASE_ADDR + iprx ) |=  ( IRQPriority << shift_amount );

}

void SPI_PeripheralControl(SPI_RegDef_t *pSPIx,uint8_t EnorDi){
	if(EnorDi == ENABLE){
		pSPIx->CR1 |=(1 << SPI_CR1_SPE);
	}else{
		pSPIx->CR1 &= ~(1 << SPI_CR1_SPE);
	}
}

void SPI_SSIConfig(SPI_RegDef_t *pSPIx,uint8_t EnorDi){
	if(EnorDi == ENABLE){
		pSPIx->CR1 |=(1 << SPI_CR1_SSI);
	}else{
		pSPIx->CR1 &= ~(1 << SPI_CR1_SSI);
	}
}

void SPI_SSOEConfig(SPI_RegDef_t *pSPIx,uint8_t EnorDi){
	if(EnorDi == ENABLE){
		pSPIx->CR2 |=(1 << SPI_CR2_SSOE);
	}else{
		pSPIx->CR2 &= ~(1 << SPI_CR2_SSOE);
	}
}

uint8_t SPI_SendDataIT(SPI_Handle_t *pSPIHandle,uint8_t *pTxBuffer, uint32_t Len)
{
	uint8_t state = pSPIHandle->TxState;

	if(state != SPI_BUSY_IN_TX)
	{
		//1 . Save the Tx buffer address and Len information in some global variables
		pSPIHandle->pTxBuffer = pTxBuffer;
		pSPIHandle->TxLen = Len;
		//2.  Mark the SPI state as busy in transmission so that
		//    no other code can take over same SPI peripheral until transmission is over
		pSPIHandle->TxState = SPI_BUSY_IN_TX;

		//3. Enable the TXEIE control bit to get interrupt whenever TXE flag is set in SR
		pSPIHandle->pSPIx->CR2 |= ( 1 << SPI_CR2_TXEIE );

	}


	return state;
}


uint8_t SPI_ReceiveDataIT(SPI_Handle_t *pSPIHandle, uint8_t *pRxBuffer, uint32_t Len)
{
	uint8_t state = pSPIHandle->RxState;

	if(state != SPI_BUSY_IN_RX)
	{
		//1 . Save the Rx buffer address and Len information in some global variables
		pSPIHandle->pRxBuffer = pRxBuffer;
		pSPIHandle->RxLen = Len;
		//2.  Mark the SPI state as busy in reception so that
		//    no other code can take over same SPI peripheral until reception is over
		pSPIHandle->RxState = SPI_BUSY_IN_RX;

		//3. Enable the RXNEIE control bit to get interrupt whenever RXNEIE flag is set in SR
		pSPIHandle->pSPIx->CR2 |= ( 1 << SPI_CR2_RXNEIE );

	}


	return state;

}




//some helper function implementations

static void  spi_txe_interrupt_handle(SPI_Handle_t *pSPIHandle)
{
	// check the DFF bit in CR1
	if( (pSPIHandle->pSPIx->CR1 & ( 1 << SPI_CR1_DFF) ) )
	{
		//16 bit DFF
		//1. load the data in to the DR
		pSPIHandle->pSPIx->DR =   *((uint16_t*)pSPIHandle->pTxBuffer);
		pSPIHandle->TxLen--;
		pSPIHandle->TxLen--;
		(uint16_t*)pSPIHandle->pTxBuffer++;
	}else
	{
		//8 bit DFF
		pSPIHandle->pSPIx->DR =   *pSPIHandle->pTxBuffer;
		pSPIHandle->TxLen--;
		pSPIHandle->pTxBuffer++;
	}

	if(! pSPIHandle->TxLen)
	{
		//TxLen is zero , so close the spi transmission and inform the application that
		//TX is over.

		//this prevents interrupts from setting up of TXE flag
		SPI_CloseTransmisson(pSPIHandle);
		SPI_ApplicationEventCallback(pSPIHandle,SPI_EVENT_TX_CMPLT);
	}

}


static void  spi_rxne_interrupt_handle(SPI_Handle_t *pSPIHandle)
{
	//do rxing as per the dff
	if(pSPIHandle->pSPIx->CR1 & ( 1 << 11))
	{
		//16 bit
		*((uint16_t*)pSPIHandle->pRxBuffer) = (uint16_t) pSPIHandle->pSPIx->DR;
		pSPIHandle->RxLen -= 2;
		pSPIHandle->pRxBuffer++;
		pSPIHandle->pRxBuffer++;

	}else
	{
		//8 bit
		*(pSPIHandle->pRxBuffer) = (uint8_t) pSPIHandle->pSPIx->DR;
		pSPIHandle->RxLen--;
		pSPIHandle->pRxBuffer++;
	}

	if(! pSPIHandle->RxLen)
	{
		//reception is complete
		SPI_CloseReception(pSPIHandle);
		SPI_ApplicationEventCallback(pSPIHandle,SPI_EVENT_RX_CMPLT);
	}

}


static void  spi_ovr_err_interrupt_handle(SPI_Handle_t *pSPIHandle)
{

	uint8_t temp;
	//1. clear the ovr flag
	if(pSPIHandle->TxState != SPI_BUSY_IN_TX)
	{
		temp = pSPIHandle->pSPIx->DR;
		temp = pSPIHandle->pSPIx->SR;
	}
	(void)temp;
	//2. inform the application
	SPI_ApplicationEventCallback(pSPIHandle,SPI_EVENT_OVR_ERR);

}


void SPI_CloseTransmisson(SPI_Handle_t *pSPIHandle)
{
	pSPIHandle->pSPIx->CR2 &= ~( 1 << SPI_CR2_TXEIE);
	pSPIHandle->pTxBuffer = NULL;
	pSPIHandle->TxLen = 0;
	pSPIHandle->TxState = SPI_READY;

}

void SPI_CloseReception(SPI_Handle_t *pSPIHandle)
{
	pSPIHandle->pSPIx->CR2 &= ~( 1 << SPI_CR2_RXNEIE);
	pSPIHandle->pRxBuffer = NULL;
	pSPIHandle->RxLen = 0;
	pSPIHandle->RxState = SPI_READY;

}



void SPI_ClearOVRFlag(SPI_RegDef_t *pSPIx)
{
	uint8_t temp;
	temp = pSPIx->DR;
	temp = pSPIx->SR;
	(void)temp;

}

__attribute__((weak)) void SPI_ApplicationEventCallback(SPI_Handle_t *pSPIHandle,uint8_t AppEv)
{
	//This is a weak implementation . the user application may override this function.
}




