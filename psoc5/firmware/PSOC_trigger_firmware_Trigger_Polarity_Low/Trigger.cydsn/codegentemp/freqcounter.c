/*******************************************************************************
* File Name: freqcounter.c  
* Version 3.0
*
*  Description:
*     The Counter component consists of a 8, 16, 24 or 32-bit counter with
*     a selectable period between 2 and 2^Width - 1.  
*
*   Note:
*     None
*
********************************************************************************
* Copyright 2008-2012, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#include "freqcounter.h"

uint8 freqcounter_initVar = 0u;


/*******************************************************************************
* Function Name: freqcounter_Init
********************************************************************************
* Summary:
*     Initialize to the schematic state
* 
* Parameters:  
*  void  
*
* Return: 
*  void
*
*******************************************************************************/
void freqcounter_Init(void) 
{
        #if (!freqcounter_UsingFixedFunction && !freqcounter_ControlRegRemoved)
            uint8 ctrl;
        #endif /* (!freqcounter_UsingFixedFunction && !freqcounter_ControlRegRemoved) */
        
        #if(!freqcounter_UsingFixedFunction) 
            /* Interrupt State Backup for Critical Region*/
            uint8 freqcounter_interruptState;
        #endif /* (!freqcounter_UsingFixedFunction) */
        
        #if (freqcounter_UsingFixedFunction)
            /* Clear all bits but the enable bit (if it's already set for Timer operation */
            freqcounter_CONTROL &= freqcounter_CTRL_ENABLE;
            
            /* Clear the mode bits for continuous run mode */
            #if (CY_PSOC5A)
                freqcounter_CONTROL2 &= ((uint8)(~freqcounter_CTRL_MODE_MASK));
            #endif /* (CY_PSOC5A) */
            #if (CY_PSOC3 || CY_PSOC5LP)
                freqcounter_CONTROL3 &= ((uint8)(~freqcounter_CTRL_MODE_MASK));                
            #endif /* (CY_PSOC3 || CY_PSOC5LP) */
            /* Check if One Shot mode is enabled i.e. RunMode !=0*/
            #if (freqcounter_RunModeUsed != 0x0u)
                /* Set 3rd bit of Control register to enable one shot mode */
                freqcounter_CONTROL |= freqcounter_ONESHOT;
            #endif /* (freqcounter_RunModeUsed != 0x0u) */
            
            /* Set the IRQ to use the status register interrupts */
            freqcounter_CONTROL2 |= freqcounter_CTRL2_IRQ_SEL;
            
            /* Clear and Set SYNCTC and SYNCCMP bits of RT1 register */
            freqcounter_RT1 &= ((uint8)(~freqcounter_RT1_MASK));
            freqcounter_RT1 |= freqcounter_SYNC;     
                    
            /*Enable DSI Sync all all inputs of the Timer*/
            freqcounter_RT1 &= ((uint8)(~freqcounter_SYNCDSI_MASK));
            freqcounter_RT1 |= freqcounter_SYNCDSI_EN;

        #else
            #if(!freqcounter_ControlRegRemoved)
            /* Set the default compare mode defined in the parameter */
            ctrl = freqcounter_CONTROL & ((uint8)(~freqcounter_CTRL_CMPMODE_MASK));
            freqcounter_CONTROL = ctrl | freqcounter_DEFAULT_COMPARE_MODE;
            
            /* Set the default capture mode defined in the parameter */
            ctrl = freqcounter_CONTROL & ((uint8)(~freqcounter_CTRL_CAPMODE_MASK));
            
            #if( 0 != freqcounter_CAPTURE_MODE_CONF)
                freqcounter_CONTROL = ctrl | freqcounter_DEFAULT_CAPTURE_MODE;
            #else
                freqcounter_CONTROL = ctrl;
            #endif /* 0 != freqcounter_CAPTURE_MODE */ 
            
            #endif /* (!freqcounter_ControlRegRemoved) */
        #endif /* (freqcounter_UsingFixedFunction) */
        
        /* Clear all data in the FIFO's */
        #if (!freqcounter_UsingFixedFunction)
            freqcounter_ClearFIFO();
        #endif /* (!freqcounter_UsingFixedFunction) */
        
        /* Set Initial values from Configuration */
        freqcounter_WritePeriod(freqcounter_INIT_PERIOD_VALUE);
        #if (!(freqcounter_UsingFixedFunction && (CY_PSOC5A)))
            freqcounter_WriteCounter(freqcounter_INIT_COUNTER_VALUE);
        #endif /* (!(freqcounter_UsingFixedFunction && (CY_PSOC5A))) */
        freqcounter_SetInterruptMode(freqcounter_INIT_INTERRUPTS_MASK);
        
        #if (!freqcounter_UsingFixedFunction)
            /* Read the status register to clear the unwanted interrupts */
            (void)freqcounter_ReadStatusRegister();
            /* Set the compare value (only available to non-fixed function implementation */
            freqcounter_WriteCompare(freqcounter_INIT_COMPARE_VALUE);
            /* Use the interrupt output of the status register for IRQ output */
            
            /* CyEnterCriticalRegion and CyExitCriticalRegion are used to mark following region critical*/
            /* Enter Critical Region*/
            freqcounter_interruptState = CyEnterCriticalSection();
            
            freqcounter_STATUS_AUX_CTRL |= freqcounter_STATUS_ACTL_INT_EN_MASK;
            
            /* Exit Critical Region*/
            CyExitCriticalSection(freqcounter_interruptState);
            
        #endif /* (!freqcounter_UsingFixedFunction) */
}


/*******************************************************************************
* Function Name: freqcounter_Enable
********************************************************************************
* Summary:
*     Enable the Counter
* 
* Parameters:  
*  void  
*
* Return: 
*  void
*
* Side Effects: 
*   If the Enable mode is set to Hardware only then this function has no effect 
*   on the operation of the counter.
*
*******************************************************************************/
void freqcounter_Enable(void) 
{
    /* Globally Enable the Fixed Function Block chosen */
    #if (freqcounter_UsingFixedFunction)
        freqcounter_GLOBAL_ENABLE |= freqcounter_BLOCK_EN_MASK;
        freqcounter_GLOBAL_STBY_ENABLE |= freqcounter_BLOCK_STBY_EN_MASK;
    #endif /* (freqcounter_UsingFixedFunction) */  
        
    /* Enable the counter from the control register  */
    /* If Fixed Function then make sure Mode is set correctly */
    /* else make sure reset is clear */
    #if(!freqcounter_ControlRegRemoved || freqcounter_UsingFixedFunction)
        freqcounter_CONTROL |= freqcounter_CTRL_ENABLE;                
    #endif /* (!freqcounter_ControlRegRemoved || freqcounter_UsingFixedFunction) */
    
}


/*******************************************************************************
* Function Name: freqcounter_Start
********************************************************************************
* Summary:
*  Enables the counter for operation 
*
* Parameters:  
*  void  
*
* Return: 
*  void
*
* Global variables:
*  freqcounter_initVar: Is modified when this function is called for the  
*   first time. Is used to ensure that initialization happens only once.
*
*******************************************************************************/
void freqcounter_Start(void) 
{
    if(freqcounter_initVar == 0u)
    {
        freqcounter_Init();
        
        freqcounter_initVar = 1u; /* Clear this bit for Initialization */        
    }
    
    /* Enable the Counter */
    freqcounter_Enable();        
}


/*******************************************************************************
* Function Name: freqcounter_Stop
********************************************************************************
* Summary:
* Halts the counter, but does not change any modes or disable interrupts.
*
* Parameters:  
*  void  
*
* Return: 
*  void
*
* Side Effects: If the Enable mode is set to Hardware only then this function
*               has no effect on the operation of the counter.
*
*******************************************************************************/
void freqcounter_Stop(void) 
{
    /* Disable Counter */
    #if(!freqcounter_ControlRegRemoved || freqcounter_UsingFixedFunction)
        freqcounter_CONTROL &= ((uint8)(~freqcounter_CTRL_ENABLE));        
    #endif /* (!freqcounter_ControlRegRemoved || freqcounter_UsingFixedFunction) */
    
    /* Globally disable the Fixed Function Block chosen */
    #if (freqcounter_UsingFixedFunction)
        freqcounter_GLOBAL_ENABLE &= ((uint8)(~freqcounter_BLOCK_EN_MASK));
        freqcounter_GLOBAL_STBY_ENABLE &= ((uint8)(~freqcounter_BLOCK_STBY_EN_MASK));
    #endif /* (freqcounter_UsingFixedFunction) */
}


/*******************************************************************************
* Function Name: freqcounter_SetInterruptMode
********************************************************************************
* Summary:
* Configures which interrupt sources are enabled to generate the final interrupt
*
* Parameters:  
*  InterruptsMask: This parameter is an or'd collection of the status bits
*                   which will be allowed to generate the counters interrupt.   
*
* Return: 
*  void
*
*******************************************************************************/
void freqcounter_SetInterruptMode(uint8 interruptsMask) 
{
    freqcounter_STATUS_MASK = interruptsMask;
}


/*******************************************************************************
* Function Name: freqcounter_ReadStatusRegister
********************************************************************************
* Summary:
*   Reads the status register and returns it's state. This function should use
*       defined types for the bit-field information as the bits in this
*       register may be permuteable.
*
* Parameters:  
*  void
*
* Return: 
*  (uint8) The contents of the status register
*
* Side Effects:
*   Status register bits may be clear on read. 
*
*******************************************************************************/
uint8   freqcounter_ReadStatusRegister(void) 
{
    return freqcounter_STATUS;
}


#if(!freqcounter_ControlRegRemoved)
/*******************************************************************************
* Function Name: freqcounter_ReadControlRegister
********************************************************************************
* Summary:
*   Reads the control register and returns it's state. This function should use
*       defined types for the bit-field information as the bits in this
*       register may be permuteable.
*
* Parameters:  
*  void
*
* Return: 
*  (uint8) The contents of the control register
*
*******************************************************************************/
uint8   freqcounter_ReadControlRegister(void) 
{
    return freqcounter_CONTROL;
}


/*******************************************************************************
* Function Name: freqcounter_WriteControlRegister
********************************************************************************
* Summary:
*   Sets the bit-field of the control register.  This function should use
*       defined types for the bit-field information as the bits in this
*       register may be permuteable.
*
* Parameters:  
*  void
*
* Return: 
*  (uint8) The contents of the control register
*
*******************************************************************************/
void    freqcounter_WriteControlRegister(uint8 control) 
{
    freqcounter_CONTROL = control;
}

#endif  /* (!freqcounter_ControlRegRemoved) */


#if (!(freqcounter_UsingFixedFunction && (CY_PSOC5A)))
/*******************************************************************************
* Function Name: freqcounter_WriteCounter
********************************************************************************
* Summary:
*   This funtion is used to set the counter to a specific value
*
* Parameters:  
*  counter:  New counter value. 
*
* Return: 
*  void 
*
*******************************************************************************/
void freqcounter_WriteCounter(uint32 counter) \
                                   
{
    #if(freqcounter_UsingFixedFunction)
        /* assert if block is already enabled */
        CYASSERT (0u == (freqcounter_GLOBAL_ENABLE & freqcounter_BLOCK_EN_MASK));
        /* If block is disabled, enable it and then write the counter */
        freqcounter_GLOBAL_ENABLE |= freqcounter_BLOCK_EN_MASK;
        CY_SET_REG16(freqcounter_COUNTER_LSB_PTR, (uint16)counter);
        freqcounter_GLOBAL_ENABLE &= ((uint8)(~freqcounter_BLOCK_EN_MASK));
    #else
        CY_SET_REG32(freqcounter_COUNTER_LSB_PTR, counter);
    #endif /* (freqcounter_UsingFixedFunction) */
}
#endif /* (!(freqcounter_UsingFixedFunction && (CY_PSOC5A))) */


/*******************************************************************************
* Function Name: freqcounter_ReadCounter
********************************************************************************
* Summary:
* Returns the current value of the counter.  It doesn't matter
* if the counter is enabled or running.
*
* Parameters:  
*  void:  
*
* Return: 
*  (uint32) The present value of the counter.
*
*******************************************************************************/
uint32 freqcounter_ReadCounter(void) 
{
    /* Force capture by reading Accumulator */
    /* Must first do a software capture to be able to read the counter */
    /* It is up to the user code to make sure there isn't already captured data in the FIFO */
    #if(freqcounter_UsingFixedFunction)
		(void)CY_GET_REG16(freqcounter_COUNTER_LSB_PTR);
	#else
		(void)CY_GET_REG8(freqcounter_COUNTER_LSB_PTR_8BIT);
	#endif/* (freqcounter_UsingFixedFunction) */
    
    /* Read the data from the FIFO (or capture register for Fixed Function)*/
    #if(freqcounter_UsingFixedFunction)
        return ((uint32)CY_GET_REG16(freqcounter_STATICCOUNT_LSB_PTR));
    #else
        return (CY_GET_REG32(freqcounter_STATICCOUNT_LSB_PTR));
    #endif /* (freqcounter_UsingFixedFunction) */
}


/*******************************************************************************
* Function Name: freqcounter_ReadCapture
********************************************************************************
* Summary:
*   This function returns the last value captured.
*
* Parameters:  
*  void
*
* Return: 
*  (uint32) Present Capture value.
*
*******************************************************************************/
uint32 freqcounter_ReadCapture(void) 
{
    #if(freqcounter_UsingFixedFunction)
        return ((uint32)CY_GET_REG16(freqcounter_STATICCOUNT_LSB_PTR));
    #else
        return (CY_GET_REG32(freqcounter_STATICCOUNT_LSB_PTR));
    #endif /* (freqcounter_UsingFixedFunction) */
}


/*******************************************************************************
* Function Name: freqcounter_WritePeriod
********************************************************************************
* Summary:
* Changes the period of the counter.  The new period 
* will be loaded the next time terminal count is detected.
*
* Parameters:  
*  period: (uint32) A value of 0 will result in
*         the counter remaining at zero.  
*
* Return: 
*  void
*
*******************************************************************************/
void freqcounter_WritePeriod(uint32 period) 
{
    #if(freqcounter_UsingFixedFunction)
        CY_SET_REG16(freqcounter_PERIOD_LSB_PTR,(uint16)period);
    #else
        CY_SET_REG32(freqcounter_PERIOD_LSB_PTR, period);
    #endif /* (freqcounter_UsingFixedFunction) */
}


/*******************************************************************************
* Function Name: freqcounter_ReadPeriod
********************************************************************************
* Summary:
* Reads the current period value without affecting counter operation.
*
* Parameters:  
*  void:  
*
* Return: 
*  (uint32) Present period value.
*
*******************************************************************************/
uint32 freqcounter_ReadPeriod(void) 
{
    #if(freqcounter_UsingFixedFunction)
        return ((uint32)CY_GET_REG16(freqcounter_PERIOD_LSB_PTR));
    #else
        return (CY_GET_REG32(freqcounter_PERIOD_LSB_PTR));
    #endif /* (freqcounter_UsingFixedFunction) */
}


#if (!freqcounter_UsingFixedFunction)
/*******************************************************************************
* Function Name: freqcounter_WriteCompare
********************************************************************************
* Summary:
* Changes the compare value.  The compare output will 
* reflect the new value on the next UDB clock.  The compare output will be 
* driven high when the present counter value compares true based on the 
* configured compare mode setting. 
*
* Parameters:  
*  Compare:  New compare value. 
*
* Return: 
*  void
*
*******************************************************************************/
void freqcounter_WriteCompare(uint32 compare) \
                                   
{
    #if(freqcounter_UsingFixedFunction)
        CY_SET_REG16(freqcounter_COMPARE_LSB_PTR, (uint16)compare);
    #else
        CY_SET_REG32(freqcounter_COMPARE_LSB_PTR, compare);
    #endif /* (freqcounter_UsingFixedFunction) */
}


/*******************************************************************************
* Function Name: freqcounter_ReadCompare
********************************************************************************
* Summary:
* Returns the compare value.
*
* Parameters:  
*  void:
*
* Return: 
*  (uint32) Present compare value.
*
*******************************************************************************/
uint32 freqcounter_ReadCompare(void) 
{
    return (CY_GET_REG32(freqcounter_COMPARE_LSB_PTR));
}


#if (freqcounter_COMPARE_MODE_SOFTWARE)
/*******************************************************************************
* Function Name: freqcounter_SetCompareMode
********************************************************************************
* Summary:
*  Sets the software controlled Compare Mode.
*
* Parameters:
*  compareMode:  Compare Mode Enumerated Type.
*
* Return:
*  void
*
*******************************************************************************/
void freqcounter_SetCompareMode(uint8 compareMode) 
{
    /* Clear the compare mode bits in the control register */
    freqcounter_CONTROL &= ((uint8)(~freqcounter_CTRL_CMPMODE_MASK));
    
    /* Write the new setting */
    freqcounter_CONTROL |= compareMode;
}
#endif  /* (freqcounter_COMPARE_MODE_SOFTWARE) */


#if (freqcounter_CAPTURE_MODE_SOFTWARE)
/*******************************************************************************
* Function Name: freqcounter_SetCaptureMode
********************************************************************************
* Summary:
*  Sets the software controlled Capture Mode.
*
* Parameters:
*  captureMode:  Capture Mode Enumerated Type.
*
* Return:
*  void
*
*******************************************************************************/
void freqcounter_SetCaptureMode(uint8 captureMode) 
{
    /* Clear the capture mode bits in the control register */
    freqcounter_CONTROL &= ((uint8)(~freqcounter_CTRL_CAPMODE_MASK));
    
    /* Write the new setting */
    freqcounter_CONTROL |= ((uint8)((uint8)captureMode << freqcounter_CTRL_CAPMODE0_SHIFT));
}
#endif  /* (freqcounter_CAPTURE_MODE_SOFTWARE) */


/*******************************************************************************
* Function Name: freqcounter_ClearFIFO
********************************************************************************
* Summary:
*   This function clears all capture data from the capture FIFO
*
* Parameters:  
*  void:
*
* Return: 
*  None
*
*******************************************************************************/
void freqcounter_ClearFIFO(void) 
{

    while(0u != (freqcounter_ReadStatusRegister() & freqcounter_STATUS_FIFONEMP))
    {
        (void)freqcounter_ReadCapture();
    }

}
#endif  /* (!freqcounter_UsingFixedFunction) */


/* [] END OF FILE */

