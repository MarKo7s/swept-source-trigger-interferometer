/*******************************************************************************
* File Name: neg.h  
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

#if !defined(CY_PINS_neg_H) /* Pins neg_H */
#define CY_PINS_neg_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"
#include "neg_aliases.h"

/* APIs are not generated for P15[7:6] */
#if !(CY_PSOC5A &&\
	 neg__PORT == 15 && ((neg__MASK & 0xC0) != 0))


/***************************************
*        Function Prototypes             
***************************************/    

/**
* \addtogroup group_general
* @{
*/
void    neg_Write(uint8 value);
void    neg_SetDriveMode(uint8 mode);
uint8   neg_ReadDataReg(void);
uint8   neg_Read(void);
void    neg_SetInterruptMode(uint16 position, uint16 mode);
uint8   neg_ClearInterrupt(void);
/** @} general */

/***************************************
*           API Constants        
***************************************/
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup driveMode Drive mode constants
     * \brief Constants to be passed as "mode" parameter in the neg_SetDriveMode() function.
     *  @{
     */
        #define neg_DM_ALG_HIZ         PIN_DM_ALG_HIZ
        #define neg_DM_DIG_HIZ         PIN_DM_DIG_HIZ
        #define neg_DM_RES_UP          PIN_DM_RES_UP
        #define neg_DM_RES_DWN         PIN_DM_RES_DWN
        #define neg_DM_OD_LO           PIN_DM_OD_LO
        #define neg_DM_OD_HI           PIN_DM_OD_HI
        #define neg_DM_STRONG          PIN_DM_STRONG
        #define neg_DM_RES_UPDWN       PIN_DM_RES_UPDWN
    /** @} driveMode */
/** @} group_constants */
    
/* Digital Port Constants */
#define neg_MASK               neg__MASK
#define neg_SHIFT              neg__SHIFT
#define neg_WIDTH              1u

/* Interrupt constants */
#if defined(neg__INTSTAT)
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in neg_SetInterruptMode() function.
     *  @{
     */
        #define neg_INTR_NONE      (uint16)(0x0000u)
        #define neg_INTR_RISING    (uint16)(0x0001u)
        #define neg_INTR_FALLING   (uint16)(0x0002u)
        #define neg_INTR_BOTH      (uint16)(0x0003u) 
    /** @} intrMode */
/** @} group_constants */

    #define neg_INTR_MASK      (0x01u) 
#endif /* (neg__INTSTAT) */


/***************************************
*             Registers        
***************************************/

/* Main Port Registers */
/* Pin State */
#define neg_PS                     (* (reg8 *) neg__PS)
/* Data Register */
#define neg_DR                     (* (reg8 *) neg__DR)
/* Port Number */
#define neg_PRT_NUM                (* (reg8 *) neg__PRT) 
/* Connect to Analog Globals */                                                  
#define neg_AG                     (* (reg8 *) neg__AG)                       
/* Analog MUX bux enable */
#define neg_AMUX                   (* (reg8 *) neg__AMUX) 
/* Bidirectional Enable */                                                        
#define neg_BIE                    (* (reg8 *) neg__BIE)
/* Bit-mask for Aliased Register Access */
#define neg_BIT_MASK               (* (reg8 *) neg__BIT_MASK)
/* Bypass Enable */
#define neg_BYP                    (* (reg8 *) neg__BYP)
/* Port wide control signals */                                                   
#define neg_CTL                    (* (reg8 *) neg__CTL)
/* Drive Modes */
#define neg_DM0                    (* (reg8 *) neg__DM0) 
#define neg_DM1                    (* (reg8 *) neg__DM1)
#define neg_DM2                    (* (reg8 *) neg__DM2) 
/* Input Buffer Disable Override */
#define neg_INP_DIS                (* (reg8 *) neg__INP_DIS)
/* LCD Common or Segment Drive */
#define neg_LCD_COM_SEG            (* (reg8 *) neg__LCD_COM_SEG)
/* Enable Segment LCD */
#define neg_LCD_EN                 (* (reg8 *) neg__LCD_EN)
/* Slew Rate Control */
#define neg_SLW                    (* (reg8 *) neg__SLW)

/* DSI Port Registers */
/* Global DSI Select Register */
#define neg_PRTDSI__CAPS_SEL       (* (reg8 *) neg__PRTDSI__CAPS_SEL) 
/* Double Sync Enable */
#define neg_PRTDSI__DBL_SYNC_IN    (* (reg8 *) neg__PRTDSI__DBL_SYNC_IN) 
/* Output Enable Select Drive Strength */
#define neg_PRTDSI__OE_SEL0        (* (reg8 *) neg__PRTDSI__OE_SEL0) 
#define neg_PRTDSI__OE_SEL1        (* (reg8 *) neg__PRTDSI__OE_SEL1) 
/* Port Pin Output Select Registers */
#define neg_PRTDSI__OUT_SEL0       (* (reg8 *) neg__PRTDSI__OUT_SEL0) 
#define neg_PRTDSI__OUT_SEL1       (* (reg8 *) neg__PRTDSI__OUT_SEL1) 
/* Sync Output Enable Registers */
#define neg_PRTDSI__SYNC_OUT       (* (reg8 *) neg__PRTDSI__SYNC_OUT) 

/* SIO registers */
#if defined(neg__SIO_CFG)
    #define neg_SIO_HYST_EN        (* (reg8 *) neg__SIO_HYST_EN)
    #define neg_SIO_REG_HIFREQ     (* (reg8 *) neg__SIO_REG_HIFREQ)
    #define neg_SIO_CFG            (* (reg8 *) neg__SIO_CFG)
    #define neg_SIO_DIFF           (* (reg8 *) neg__SIO_DIFF)
#endif /* (neg__SIO_CFG) */

/* Interrupt Registers */
#if defined(neg__INTSTAT)
    #define neg_INTSTAT            (* (reg8 *) neg__INTSTAT)
    #define neg_SNAP               (* (reg8 *) neg__SNAP)
    
	#define neg_0_INTTYPE_REG 		(* (reg8 *) neg__0__INTTYPE)
#endif /* (neg__INTSTAT) */

#endif /* CY_PSOC5A... */

#endif /*  CY_PINS_neg_H */


/* [] END OF FILE */
