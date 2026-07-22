/*******************************************************************************
* File Name: muxSel_PM.c
* Version 1.80
*
* Description:
*  This file contains the setup, control, and status commands to support 
*  the component operation in the low power mode. 
*
* Note:
*
********************************************************************************
* Copyright 2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#include "muxSel.h"

/* Check for removal by optimization */
#if !defined(muxSel_Sync_ctrl_reg__REMOVED)

static muxSel_BACKUP_STRUCT  muxSel_backup = {0u};

    
/*******************************************************************************
* Function Name: muxSel_SaveConfig
********************************************************************************
*
* Summary:
*  Saves the control register value.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void muxSel_SaveConfig(void) 
{
    muxSel_backup.controlState = muxSel_Control;
}


/*******************************************************************************
* Function Name: muxSel_RestoreConfig
********************************************************************************
*
* Summary:
*  Restores the control register value.
*
* Parameters:
*  None
*
* Return:
*  None
*
*
*******************************************************************************/
void muxSel_RestoreConfig(void) 
{
     muxSel_Control = muxSel_backup.controlState;
}


/*******************************************************************************
* Function Name: muxSel_Sleep
********************************************************************************
*
* Summary:
*  Prepares the component for entering the low power mode.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void muxSel_Sleep(void) 
{
    muxSel_SaveConfig();
}


/*******************************************************************************
* Function Name: muxSel_Wakeup
********************************************************************************
*
* Summary:
*  Restores the component after waking up from the low power mode.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void muxSel_Wakeup(void)  
{
    muxSel_RestoreConfig();
}

#endif /* End check for removal by optimization */


/* [] END OF FILE */
