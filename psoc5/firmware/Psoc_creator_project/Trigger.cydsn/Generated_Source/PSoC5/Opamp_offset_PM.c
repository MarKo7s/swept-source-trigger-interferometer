/*******************************************************************************
* File Name: Opamp_offset_PM.c
* Version 1.90
*
* Description:
*  This file provides the power management source code to the API for the 
*  OpAmp (Analog Buffer) component.
*
* Note:
*
********************************************************************************
* Copyright 2008-2012, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#include "Opamp_offset.h"

static Opamp_offset_BACKUP_STRUCT  Opamp_offset_backup;


/*******************************************************************************  
* Function Name: Opamp_offset_SaveConfig
********************************************************************************
*
* Summary:
*  Saves the current user configuration registers.
* 
* Parameters:
*  void
* 
* Return:
*  void
*
*******************************************************************************/
void Opamp_offset_SaveConfig(void) 
{
    /* Nothing to save as registers are System reset on retention flops */
}


/*******************************************************************************  
* Function Name: Opamp_offset_RestoreConfig
********************************************************************************
*
* Summary:
*  Restores the current user configuration registers.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void Opamp_offset_RestoreConfig(void) 
{
    /* Nothing to restore */
}


/*******************************************************************************   
* Function Name: Opamp_offset_Sleep
********************************************************************************
*
* Summary:
*  Disables block's operation and saves its configuration. Should be called 
*  just prior to entering sleep.
*
* Parameters:
*  void
*
* Return:
*  void
*
* Global variables:
*  Opamp_offset_backup: The structure field 'enableState' is modified 
*  depending on the enable state of the block before entering to sleep mode.
*
*******************************************************************************/
void Opamp_offset_Sleep(void) 
{
    /* Save OpAmp enable state */
    if((Opamp_offset_PM_ACT_CFG_REG & Opamp_offset_ACT_PWR_EN) != 0u)
    {
        /* Component is enabled */
        Opamp_offset_backup.enableState = 1u;
         /* Stops the component */
         Opamp_offset_Stop();
    }
    else
    {
        /* Component is disabled */
        Opamp_offset_backup.enableState = 0u;
    }
    /* Saves the configuration */
    Opamp_offset_SaveConfig();
}


/*******************************************************************************  
* Function Name: Opamp_offset_Wakeup
********************************************************************************
*
* Summary:
*  Enables block's operation and restores its configuration. Should be called
*  just after awaking from sleep.
*
* Parameters:
*  void
*
* Return:
*  void
*
* Global variables:
*  Opamp_offset_backup: The structure field 'enableState' is used to 
*  restore the enable state of block after wakeup from sleep mode.
*
*******************************************************************************/
void Opamp_offset_Wakeup(void) 
{
    /* Restore the user configuration */
    Opamp_offset_RestoreConfig();

    /* Enables the component operation */
    if(Opamp_offset_backup.enableState == 1u)
    {
        Opamp_offset_Enable();
    } /* Do nothing if component was disable before */
}


/* [] END OF FILE */
