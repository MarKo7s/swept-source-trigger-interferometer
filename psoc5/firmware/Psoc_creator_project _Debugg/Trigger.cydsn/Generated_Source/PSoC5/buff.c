/*******************************************************************************
* File Name: buff.c
* Version 1.90
*
* Description:
*  This file provides the source code to the API for OpAmp (Analog Buffer) 
*  Component.
*
* Note:
*
********************************************************************************
* Copyright 2008-2012, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#include "buff.h"

uint8 buff_initVar = 0u;


/*******************************************************************************   
* Function Name: buff_Init
********************************************************************************
*
* Summary:
*  Initialize component's parameters to the parameters set by user in the 
*  customizer of the component placed onto schematic. Usually called in 
*  buff_Start().
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void buff_Init(void) 
{
    buff_SetPower(buff_DEFAULT_POWER);
}


/*******************************************************************************   
* Function Name: buff_Enable
********************************************************************************
*
* Summary:
*  Enables the OpAmp block operation
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void buff_Enable(void) 
{
    /* Enable negative charge pumps in ANIF */
    buff_PUMP_CR1_REG  |= (buff_PUMP_CR1_CLKSEL | buff_PUMP_CR1_FORCE);

    /* Enable power to buffer in active mode */
    buff_PM_ACT_CFG_REG |= buff_ACT_PWR_EN;

    /* Enable power to buffer in alternative active mode */
    buff_PM_STBY_CFG_REG |= buff_STBY_PWR_EN;
}


/*******************************************************************************
* Function Name:   buff_Start
********************************************************************************
*
* Summary:
*  The start function initializes the Analog Buffer with the default values and 
*  sets the power to the given level. A power level of 0, is same as 
*  executing the stop function.
*
* Parameters:
*  void
*
* Return:
*  void
*
* Global variables:
*  buff_initVar: Used to check the initial configuration, modified 
*  when this function is called for the first time.
*
*******************************************************************************/
void buff_Start(void) 
{
    if(buff_initVar == 0u)
    {
        buff_initVar = 1u;
        buff_Init();
    }

    buff_Enable();
}


/*******************************************************************************
* Function Name: buff_Stop
********************************************************************************
*
* Summary:
*  Powers down amplifier to lowest power state.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void buff_Stop(void) 
{
    /* Disable power to buffer in active mode template */
    buff_PM_ACT_CFG_REG &= (uint8)(~buff_ACT_PWR_EN);

    /* Disable power to buffer in alternative active mode template */
    buff_PM_STBY_CFG_REG &= (uint8)(~buff_STBY_PWR_EN);
    
    /* Disable negative charge pumps for ANIF only if all ABuf is turned OFF */
    if(buff_PM_ACT_CFG_REG == 0u)
    {
        buff_PUMP_CR1_REG &= (uint8)(~(buff_PUMP_CR1_CLKSEL | buff_PUMP_CR1_FORCE));
    }
}


/*******************************************************************************
* Function Name: buff_SetPower
********************************************************************************
*
* Summary:
*  Sets power level of Analog buffer.
*
* Parameters: 
*  power: PSoC3: Sets power level between low (1) and high power (3).
*         PSoC5: Sets power level High (0)
*
* Return:
*  void
*
**********************************************************************************/
void buff_SetPower(uint8 power) 
{
    #if (CY_PSOC3 || CY_PSOC5LP)
        buff_CR_REG &= (uint8)(~buff_PWR_MASK);
        buff_CR_REG |= power & buff_PWR_MASK;      /* Set device power */
    #else
        CYASSERT(buff_HIGHPOWER == power);
    #endif /* CY_PSOC3 || CY_PSOC5LP */
}


/* [] END OF FILE */
