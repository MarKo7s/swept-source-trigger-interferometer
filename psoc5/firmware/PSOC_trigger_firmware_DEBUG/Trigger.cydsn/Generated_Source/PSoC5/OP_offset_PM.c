/*******************************************************************************
* File Name: OP_offset_PM.c
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

#include "OP_offset.h"

static OP_offset_BACKUP_STRUCT  OP_offset_backup;


/*******************************************************************************  
* Function Name: OP_offset_SaveConfig
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
void OP_offset_SaveConfig(void) 
{
    /* Nothing to save as registers are System reset on retention flops */
}


/*******************************************************************************  
* Function Name: OP_offset_RestoreConfig
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
void OP_offset_RestoreConfig(void) 
{
    /* Nothing to restore */
}


/*******************************************************************************   
* Function Name: OP_offset_Sleep
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
*  OP_offset_backup: The structure field 'enableState' is modified 
*  depending on the enable state of the block before entering to sleep mode.
*
*******************************************************************************/
void OP_offset_Sleep(void) 
{
    /* Save OpAmp enable state */
    if((OP_offset_PM_ACT_CFG_REG & OP_offset_ACT_PWR_EN) != 0u)
    {
        /* Component is enabled */
        OP_offset_backup.enableState = 1u;
         /* Stops the component */
         OP_offset_Stop();
    }
    else
    {
        /* Component is disabled */
        OP_offset_backup.enableState = 0u;
    }
    /* Saves the configuration */
    OP_offset_SaveConfig();
}


/*******************************************************************************  
* Function Name: OP_offset_Wakeup
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
*  OP_offset_backup: The structure field 'enableState' is used to 
*  restore the enable state of block after wakeup from sleep mode.
*
*******************************************************************************/
void OP_offset_Wakeup(void) 
{
    /* Restore the user configuration */
    OP_offset_RestoreConfig();

    /* Enables the component operation */
    if(OP_offset_backup.enableState == 1u)
    {
        OP_offset_Enable();
    } /* Do nothing if component was disable before */
}


/* [] END OF FILE */
