/*******************************************************************************
* File Name: comp_out_monitor.h  
* Version 2.20
*
* Description:
*  This file contains Pin function prototypes and register defines
*
* Note:
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_PINS_comp_out_monitor_H) /* Pins comp_out_monitor_H */
#define CY_PINS_comp_out_monitor_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"
#include "comp_out_monitor_aliases.h"

/* APIs are not generated for P15[7:6] */
#if !(CY_PSOC5A &&\
	 comp_out_monitor__PORT == 15 && ((comp_out_monitor__MASK & 0xC0) != 0))


/***************************************
*        Function Prototypes             
***************************************/    

/**
* \addtogroup group_general
* @{
*/
void    comp_out_monitor_Write(uint8 value);
void    comp_out_monitor_SetDriveMode(uint8 mode);
uint8   comp_out_monitor_ReadDataReg(void);
uint8   comp_out_monitor_Read(void);
void    comp_out_monitor_SetInterruptMode(uint16 position, uint16 mode);
uint8   comp_out_monitor_ClearInterrupt(void);
/** @} general */

/***************************************
*           API Constants        
***************************************/
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup driveMode Drive mode constants
     * \brief Constants to be passed as "mode" parameter in the comp_out_monitor_SetDriveMode() function.
     *  @{
     */
        #define comp_out_monitor_DM_ALG_HIZ         PIN_DM_ALG_HIZ
        #define comp_out_monitor_DM_DIG_HIZ         PIN_DM_DIG_HIZ
        #define comp_out_monitor_DM_RES_UP          PIN_DM_RES_UP
        #define comp_out_monitor_DM_RES_DWN         PIN_DM_RES_DWN
        #define comp_out_monitor_DM_OD_LO           PIN_DM_OD_LO
        #define comp_out_monitor_DM_OD_HI           PIN_DM_OD_HI
        #define comp_out_monitor_DM_STRONG          PIN_DM_STRONG
        #define comp_out_monitor_DM_RES_UPDWN       PIN_DM_RES_UPDWN
    /** @} driveMode */
/** @} group_constants */
    
/* Digital Port Constants */
#define comp_out_monitor_MASK               comp_out_monitor__MASK
#define comp_out_monitor_SHIFT              comp_out_monitor__SHIFT
#define comp_out_monitor_WIDTH              1u

/* Interrupt constants */
#if defined(comp_out_monitor__INTSTAT)
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in comp_out_monitor_SetInterruptMode() function.
     *  @{
     */
        #define comp_out_monitor_INTR_NONE      (uint16)(0x0000u)
        #define comp_out_monitor_INTR_RISING    (uint16)(0x0001u)
        #define comp_out_monitor_INTR_FALLING   (uint16)(0x0002u)
        #define comp_out_monitor_INTR_BOTH      (uint16)(0x0003u) 
    /** @} intrMode */
/** @} group_constants */

    #define comp_out_monitor_INTR_MASK      (0x01u) 
#endif /* (comp_out_monitor__INTSTAT) */


/***************************************
*             Registers        
***************************************/

/* Main Port Registers */
/* Pin State */
#define comp_out_monitor_PS                     (* (reg8 *) comp_out_monitor__PS)
/* Data Register */
#define comp_out_monitor_DR                     (* (reg8 *) comp_out_monitor__DR)
/* Port Number */
#define comp_out_monitor_PRT_NUM                (* (reg8 *) comp_out_monitor__PRT) 
/* Connect to Analog Globals */                                                  
#define comp_out_monitor_AG                     (* (reg8 *) comp_out_monitor__AG)                       
/* Analog MUX bux enable */
#define comp_out_monitor_AMUX                   (* (reg8 *) comp_out_monitor__AMUX) 
/* Bidirectional Enable */                                                        
#define comp_out_monitor_BIE                    (* (reg8 *) comp_out_monitor__BIE)
/* Bit-mask for Aliased Register Access */
#define comp_out_monitor_BIT_MASK               (* (reg8 *) comp_out_monitor__BIT_MASK)
/* Bypass Enable */
#define comp_out_monitor_BYP                    (* (reg8 *) comp_out_monitor__BYP)
/* Port wide control signals */                                                   
#define comp_out_monitor_CTL                    (* (reg8 *) comp_out_monitor__CTL)
/* Drive Modes */
#define comp_out_monitor_DM0                    (* (reg8 *) comp_out_monitor__DM0) 
#define comp_out_monitor_DM1                    (* (reg8 *) comp_out_monitor__DM1)
#define comp_out_monitor_DM2                    (* (reg8 *) comp_out_monitor__DM2) 
/* Input Buffer Disable Override */
#define comp_out_monitor_INP_DIS                (* (reg8 *) comp_out_monitor__INP_DIS)
/* LCD Common or Segment Drive */
#define comp_out_monitor_LCD_COM_SEG            (* (reg8 *) comp_out_monitor__LCD_COM_SEG)
/* Enable Segment LCD */
#define comp_out_monitor_LCD_EN                 (* (reg8 *) comp_out_monitor__LCD_EN)
/* Slew Rate Control */
#define comp_out_monitor_SLW                    (* (reg8 *) comp_out_monitor__SLW)

/* DSI Port Registers */
/* Global DSI Select Register */
#define comp_out_monitor_PRTDSI__CAPS_SEL       (* (reg8 *) comp_out_monitor__PRTDSI__CAPS_SEL) 
/* Double Sync Enable */
#define comp_out_monitor_PRTDSI__DBL_SYNC_IN    (* (reg8 *) comp_out_monitor__PRTDSI__DBL_SYNC_IN) 
/* Output Enable Select Drive Strength */
#define comp_out_monitor_PRTDSI__OE_SEL0        (* (reg8 *) comp_out_monitor__PRTDSI__OE_SEL0) 
#define comp_out_monitor_PRTDSI__OE_SEL1        (* (reg8 *) comp_out_monitor__PRTDSI__OE_SEL1) 
/* Port Pin Output Select Registers */
#define comp_out_monitor_PRTDSI__OUT_SEL0       (* (reg8 *) comp_out_monitor__PRTDSI__OUT_SEL0) 
#define comp_out_monitor_PRTDSI__OUT_SEL1       (* (reg8 *) comp_out_monitor__PRTDSI__OUT_SEL1) 
/* Sync Output Enable Registers */
#define comp_out_monitor_PRTDSI__SYNC_OUT       (* (reg8 *) comp_out_monitor__PRTDSI__SYNC_OUT) 

/* SIO registers */
#if defined(comp_out_monitor__SIO_CFG)
    #define comp_out_monitor_SIO_HYST_EN        (* (reg8 *) comp_out_monitor__SIO_HYST_EN)
    #define comp_out_monitor_SIO_REG_HIFREQ     (* (reg8 *) comp_out_monitor__SIO_REG_HIFREQ)
    #define comp_out_monitor_SIO_CFG            (* (reg8 *) comp_out_monitor__SIO_CFG)
    #define comp_out_monitor_SIO_DIFF           (* (reg8 *) comp_out_monitor__SIO_DIFF)
#endif /* (comp_out_monitor__SIO_CFG) */

/* Interrupt Registers */
#if defined(comp_out_monitor__INTSTAT)
    #define comp_out_monitor_INTSTAT            (* (reg8 *) comp_out_monitor__INTSTAT)
    #define comp_out_monitor_SNAP               (* (reg8 *) comp_out_monitor__SNAP)
    
	#define comp_out_monitor_0_INTTYPE_REG 		(* (reg8 *) comp_out_monitor__0__INTTYPE)
#endif /* (comp_out_monitor__INTSTAT) */

#endif /* CY_PSOC5A... */

#endif /*  CY_PINS_comp_out_monitor_H */


/* [] END OF FILE */
