/*******************************************************************************
* File Name: freqcounter_PM.c  
* Version 3.0
*
*  Description:
*    This file provides the power management source code to API for the
*    Counter.  
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

static freqcounter_backupStruct freqcounter_backup;


/*******************************************************************************
* Function Name: freqcounter_SaveConfig
********************************************************************************
* Summary:
*     Save the current user configuration
*
* Parameters:  
*  void
*
* Return: 
*  void
*
* Global variables:
*  freqcounter_backup:  Variables of this global structure are modified to 
*  store the values of non retention configuration registers when Sleep() API is 
*  called.
*
*******************************************************************************/
void freqcounter_SaveConfig(void) 
{
    #if (!freqcounter_UsingFixedFunction)

        freqcounter_backup.CounterUdb = freqcounter_ReadCounter();

        #if(!freqcounter_ControlRegRemoved)
            freqcounter_backup.CounterControlRegister = freqcounter_ReadControlRegister();
        #endif /* (!freqcounter_ControlRegRemoved) */

    #endif /* (!freqcounter_UsingFixedFunction) */
}


/*******************************************************************************
* Function Name: freqcounter_RestoreConfig
********************************************************************************
*
* Summary:
*  Restores the current user configuration.
*
* Parameters:  
*  void
*
* Return: 
*  void
*
* Global variables:
*  freqcounter_backup:  Variables of this global structure are used to 
*  restore the values of non retention registers on wakeup from sleep mode.
*
*******************************************************************************/
void freqcounter_RestoreConfig(void) 
{      
    #if (!freqcounter_UsingFixedFunction)

       freqcounter_WriteCounter(freqcounter_backup.CounterUdb);

        #if(!freqcounter_ControlRegRemoved)
            freqcounter_WriteControlRegister(freqcounter_backup.CounterControlRegister);
        #endif /* (!freqcounter_ControlRegRemoved) */

    #endif /* (!freqcounter_UsingFixedFunction) */
}


/*******************************************************************************
* Function Name: freqcounter_Sleep
********************************************************************************
* Summary:
*     Stop and Save the user configuration
*
* Parameters:  
*  void
*
* Return: 
*  void
*
* Global variables:
*  freqcounter_backup.enableState:  Is modified depending on the enable 
*  state of the block before entering sleep mode.
*
*******************************************************************************/
void freqcounter_Sleep(void) 
{
    #if(!freqcounter_ControlRegRemoved)
        /* Save Counter's enable state */
        if(freqcounter_CTRL_ENABLE == (freqcounter_CONTROL & freqcounter_CTRL_ENABLE))
        {
            /* Counter is enabled */
            freqcounter_backup.CounterEnableState = 1u;
        }
        else
        {
            /* Counter is disabled */
            freqcounter_backup.CounterEnableState = 0u;
        }
    #else
        freqcounter_backup.CounterEnableState = 1u;
        if(freqcounter_backup.CounterEnableState != 0u)
        {
            freqcounter_backup.CounterEnableState = 0u;
        }
    #endif /* (!freqcounter_ControlRegRemoved) */
    
    freqcounter_Stop();
    freqcounter_SaveConfig();
}


/*******************************************************************************
* Function Name: freqcounter_Wakeup
********************************************************************************
*
* Summary:
*  Restores and enables the user configuration
*  
* Parameters:  
*  void
*
* Return: 
*  void
*
* Global variables:
*  freqcounter_backup.enableState:  Is used to restore the enable state of 
*  block on wakeup from sleep mode.
*
*******************************************************************************/
void freqcounter_Wakeup(void) 
{
    freqcounter_RestoreConfig();
    #if(!freqcounter_ControlRegRemoved)
        if(freqcounter_backup.CounterEnableState == 1u)
        {
            /* Enable Counter's operation */
            freqcounter_Enable();
        } /* Do nothing if Counter was disabled before */    
    #endif /* (!freqcounter_ControlRegRemoved) */
    
}


/* [] END OF FILE */
