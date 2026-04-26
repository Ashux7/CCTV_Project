/*
 * stm32f4xx_gpio_driver.c
 *
 *  Created on: 06-Mar-2026
 *      Author: saura
 */
#include "stm32f4xx_gpio_driver.h"
/*********************************************************************
 * @fn      		  - GPIO_Init
 *
 * @brief             - Initializes the GPIO pin according to the configuration
 *                      parameters provided in the GPIO handle structure.
 *
 * @param[in]         - pGPIOHandle: Pointer to the GPIO handle structure
 *                      containing pin configuration (mode, speed, pull-up/pull-down,
 *                      output type, alternate function).
 *
 * @return            - None
 *
 * @Note              - This function must be called after enabling the peripheral
 *                      clock for the corresponding GPIO port.
 *********************************************************************/
void GPIO_Init(GPIO_Handle_t *pGPIOHandle)
{   //extra feature for convination of gpio enable peripheral clock

	 uint32_t temp=0; //temp. register

	 //enable the peripheral clock

	 GPIO_PeriClockControl(pGPIOHandle->pGPIOx, ENABLE);

	//1 . configure the mode of gpio pin

	if(pGPIOHandle->GPIO_PinConfig.GPIO_PinMode <= GPIO_MODE_ANALOG)
	{
		//the non interrupt mode
		temp = (pGPIOHandle->GPIO_PinConfig.GPIO_PinMode << (2 * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber ) );
		pGPIOHandle->pGPIOx->MODER &= ~( 0x3 << (2 * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber)); //clearing
		pGPIOHandle->pGPIOx->MODER |= temp; //setting

	}else
	{   if(pGPIOHandle->GPIO_PinConfig.GPIO_PinMode ==GPIO_MODE_IT_FT )
	{
		EXTI->FTSR |= (1 << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber) ;
		//clear the rtsr bit
		EXTI->RTSR &= ~(1 << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber) ;
	}else if(pGPIOHandle->GPIO_PinConfig.GPIO_PinMode ==GPIO_MODE_IT_RT){
		EXTI->RTSR |= (1 << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber) ;
				//clear the Ftsr bit
		EXTI->FTSR &= ~(1 << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber) ;
	}else if(pGPIOHandle->GPIO_PinConfig.GPIO_PinMode ==GPIO_MODE_IT_RFT){
		EXTI->RTSR |= (1 << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber) ;
		EXTI->FTSR |= (1 << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber) ;
	}
    // ENABLE THE EXTI INTERRUPT DELIVERY USING IMR
	EXTI->IMR |= (1 << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber) ;
    //CONFIGURATION OF THE GPIO PORT SELECTION IN SYSCFG_EXTICR
	uint8_t temp1 = pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber/4;
	uint8_t temp2 = pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber % 4;
	uint8_t portcode =GPIO_BASEADDR_TO_CODE(pGPIOHandle->pGPIOx);
	SYSCFG_PCLK_EN();
	SYSCFG->EXTICR[temp1] &=~(0xF <<(temp2*4));//CLEARING THE REGISTER
	SYSCFG->EXTICR[temp1]|=portcode <<(temp2*4); //SETTING THE REGISTER


	}

	//2. configure the speed
	temp = (pGPIOHandle->GPIO_PinConfig.GPIO_PinSpeed << ( 2 * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber) );
	pGPIOHandle->pGPIOx->OSPEEDR &= ~( 0x3 << ( 2 * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber)); //clearing
	pGPIOHandle->pGPIOx->OSPEEDR |= temp;

	//3. configure the pupd settings
	temp = (pGPIOHandle->GPIO_PinConfig.GPIO_PinPuPdControl << ( 2 * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber) );
	pGPIOHandle->pGPIOx->PUPDR &= ~( 0x3 << ( 2 * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber)); //clearing
	pGPIOHandle->pGPIOx->PUPDR |= temp;


	//4. configure the optype
	temp = (pGPIOHandle->GPIO_PinConfig.GPIO_PinOPType << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber );
	pGPIOHandle->pGPIOx->OTYPER &= ~( 0x1 << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber); //clearing
	pGPIOHandle->pGPIOx->OTYPER |= temp;

	//5. configure the alt functionality
	if(pGPIOHandle->GPIO_PinConfig.GPIO_PinMode == GPIO_MODE_ALTFN)
	{
		//configure the alt function registers.
		uint8_t temp1, temp2;

		temp1 = pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber / 8;
		temp2 = pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber  % 8;
		pGPIOHandle->pGPIOx->AFR[temp1] &= ~(0xF << ( 4 * temp2 ) ); //clearing
		pGPIOHandle->pGPIOx->AFR[temp1] |= (pGPIOHandle->GPIO_PinConfig.GPIO_PinAltFunMode << ( 4 * temp2 ) );
	}

}



/*********************************************************************
 * @fn      		  - GPIO_DeInit
 *
 * @brief             - Resets the GPIO port registers to their default reset values.
 *
 * @param[in]         - pGPIOx: Base address of the GPIO peripheral (e.g., GPIOA, GPIOB).
 *
 * @return            - None
 *
 * @Note              - This function uses the RCC reset mechanism to reset the GPIO.
 *********************************************************************/
void GPIO_DeInit(GPIO_RegDef_t *pGPIOx){
	if(pGPIOx == GPIOA)
		{
			GPIOA_REG_RESET();
		}else if (pGPIOx == GPIOB)
		{
			GPIOB_REG_RESET();
		}else if (pGPIOx == GPIOC)
		{
			GPIOC_REG_RESET();
		}else if (pGPIOx == GPIOD)
		{
			GPIOD_REG_RESET();
		}else if (pGPIOx == GPIOE)
		{
			GPIOE_REG_RESET();
		}else if (pGPIOx == GPIOF)
		{
			GPIOF_REG_RESET();
		}else if (pGPIOx == GPIOG)
		{
			GPIOG_REG_RESET();
		}else if (pGPIOx == GPIOH)
		{
			GPIOH_REG_RESET();
		}else if (pGPIOx == GPIOI)
		{
			GPIOI_REG_RESET();
		}


}

/*********************************************************************
 * @fn      		  - GPIO_PeriClockControl
 *
 * @brief             - Enables or disables the peripheral clock for a given GPIO port.
 *
 * @param[in]         - pGPIOx: Base address of the GPIO peripheral.
 * @param[in]         - EnorDi: Enable or Disable macro (ENABLE/DISABLE).
 *
 * @return            - None
 *
 * @Note              - Must be called before using any GPIO functionality.
 *********************************************************************/
void GPIO_PeriClockControl(GPIO_RegDef_t *pGPIOx,uint8_t EnorDi){
if(EnorDi == ENABLE)
{   if(pGPIOx == GPIOA){
	GPIOA_PCLK_EN();
       }
else if(pGPIOx == GPIOB){
	GPIOB_PCLK_EN();
       }
else if(pGPIOx == GPIOC){
	GPIOC_PCLK_EN();
       }
else if(pGPIOx == GPIOD){
	GPIOD_PCLK_EN();
       }
else if(pGPIOx == GPIOE){
	GPIOE_PCLK_EN();
       }
else if(pGPIOx == GPIOF){
	GPIOF_PCLK_EN();
       }
else if(pGPIOx == GPIOG){
	GPIOG_PCLK_EN();
       }
else if(pGPIOx == GPIOH){
	GPIOH_PCLK_EN();
       }
else if(pGPIOx == GPIOI){
	GPIOI_PCLK_EN();
       }
else if(pGPIOx == GPIOJ){
	GPIOJ_PCLK_EN();
       }
else if(pGPIOx == GPIOK){
	GPIOK_PCLK_EN();
       }
}
else{
	if(pGPIOx == GPIOA){
		GPIOA_PCLK_DI();
	       }
	else if(pGPIOx == GPIOB){
		GPIOB_PCLK_DI();
	       }
	else if(pGPIOx == GPIOC){
		GPIOC_PCLK_DI();
	       }
	else if(pGPIOx == GPIOD){
		GPIOD_PCLK_DI();
	       }
	else if(pGPIOx == GPIOE){
		GPIOE_PCLK_DI();
	       }
	else if(pGPIOx == GPIOF){
		GPIOF_PCLK_DI();
	       }
	else if(pGPIOx == GPIOG){
		GPIOG_PCLK_DI();
	       }
	else if(pGPIOx == GPIOH){
		GPIOH_PCLK_DI();
	       }
	else if(pGPIOx == GPIOI){
		GPIOI_PCLK_DI();
	       }
	else if(pGPIOx == GPIOJ){
		GPIOJ_PCLK_DI();
	       }
	else if(pGPIOx == GPIOK){
		GPIOK_PCLK_DI();
	   }
}
}

/*********************************************************************
 * @fn      		  - GPIO_ReadFromInputPin
 *
 * @brief             - Reads the logic level (0 or 1) from a specific input pin.
 *
 * @param[in]         - pGPIOx: Base address of the GPIO peripheral.
 * @param[in]         - PinNumber: Pin number to be read.
 *
 * @return            - uint8_t: Logic level (0 = LOW, 1 = HIGH).
 *
 * @Note              - Ensure the pin is configured as input before calling.
 *********************************************************************/
uint8_t GPIO_ReadFromInputPin(GPIO_RegDef_t *pGPIOx,uint8_t PinNumber){
	volatile uint8_t value;
	value=(uint8_t)((pGPIOx ->IDR >> PinNumber) & 0x00000001);

return value;
}

/*********************************************************************
 * @fn      		  - GPIO_ReadFromOutputPin
 *
 * @brief             - Reads the entire output data register of the GPIO port.
 *
 * @param[in]         - pGPIOx: Base address of the GPIO peripheral.
 *
 * @return            - uint16_t: Value of the output data register.
 *
 * @Note              - Useful for reading multiple pin states at once.
 *********************************************************************/
uint16_t GPIO_ReadFromOutputPin(GPIO_RegDef_t *pGPIOx){
	volatile uint16_t value;
	value=(uint16_t)(pGPIOx ->IDR);

return value;
}

/*********************************************************************
 * @fn      		  - GPIO_WritetoOutputPin
 *
 * @brief             - Writes a logic value (0 or 1) to a specific GPIO output pin.
 *
 * @param[in]         - pGPIOx: Base address of the GPIO peripheral.
 * @param[in]         - PinNumber: Pin number to be written.
 * @param[in]         - Value: Logic level to write (0 = LOW, 1 = HIGH).
 *
 * @return            - None
 *
 * @Note              - Ensure the pin is configured as output before calling.
 *********************************************************************/
void GPIO_WriteToOutputPin(GPIO_RegDef_t *pGPIOx,uint8_t PinNumber,uint8_t Value){
   if(Value == GPIO_PIN_SET){
	   pGPIOx->ODR |=(1<<PinNumber);
   }
   else{
	   pGPIOx->ODR &= ~(1<<PinNumber);
   }
}

/*********************************************************************
 * @fn      		  - GPIO_WriteToOutputPort
 *
 * @brief             - Writes a 16-bit value to the entire GPIO output data register.
 *
 * @param[in]         - pGPIOx: Base address of the GPIO peripheral.
 * @param[in]         - Value: 16-bit value to write to the port.
 *
 * @return            - None
 *
 * @Note              - This will overwrite all pin states of the port.
 *********************************************************************/
void GPIO_WriteToOutputPort(GPIO_RegDef_t *pGPIOx,uint16_t Value){
   pGPIOx ->ODR =Value;
}

/*********************************************************************
 * @fn      		  - GPIO_ToggleOutputPin
 *
 * @brief             - Toggles the current logic state of a specific GPIO output pin.
 *
 * @param[in]         - pGPIOx: Base address of the GPIO peripheral.
 * @param[in]         - PinNumber: Pin number to be toggled.
 *
 * @return            - None
 *
 * @Note              - Useful for blinking LEDs or toggling signals.
 *********************************************************************/
void GPIO_ToggleOutputPin(GPIO_RegDef_t *pGPIOx,uint8_t PinNumber){
    pGPIOx ->ODR ^=(1<< PinNumber);
}

/*********************************************************************
 * @fn      		  - GPIO_IRQConfig
 *
 * @brief             - Configures the interrupt for a specific GPIO pin.
 *
 * @param[in]         - IRQNumber: Interrupt request number associated with the pin.
 * @param[in]         - IRQPriority: Priority level for the interrupt.
 * @param[in]         - EnorDi: Enable or Disable macro (ENABLE/DISABLE).
 *
 * @return            - None
 *
 * @Note              - NVIC registers are configured in this function.
 *********************************************************************/
void GPIO_InterruptConfig(uint8_t IRQNumber,uint8_t EnorDi){
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

void GPIO_IRQPriorityConfig(uint8_t IRQNumber,uint32_t IRQPriority){
  //1.first lets find out the ipr register
	uint8_t iprx= IRQNumber /4;
	uint8_t iprx_section = IRQNumber % 4;
	uint8_t shift=(8*iprx_section)+(8-NO_PR_BITS_IMPLEMENTED);
	*(NVIC_PR_BASE_ADDR +iprx) |=(IRQPriority << shift);

}
/*********************************************************************
 * @fn      		  - GPIO_IRQHandling
 *
 * @brief             - Handles the interrupt triggered by a specific GPIO pin.
 *
 * @param[in]         - PinNumber: Pin number that triggered the interrupt.
 *
 * @return            - None
 *
 * @Note              - Clears the EXTI pending register bit corresponding to the pin.
 *********************************************************************/
void GPIO_IRQHandling(uint8_t PinNumber){
    if(EXTI->PR & (1<< PinNumber)){
    	EXTI->PR |= (1<<PinNumber);
    }
}
