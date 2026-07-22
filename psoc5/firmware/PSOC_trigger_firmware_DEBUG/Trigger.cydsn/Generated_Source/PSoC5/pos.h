/*******************************************************************************
* File Name: pos.h  
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

#if !defined(CY_PINS_pos_H) /* Pins pos_H */
#define CY_PINS_pos_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"
#include "pos_aliases.h"

/* APIs are not generated for P15[7:6] */
#if !(CY_PSOC5A &&\
	 pos__PORT == 15 && ((pos__MASK & 0xC0) != 0))


/***************************************
*        Function Prototypes             
***************************************/    

/**
* \addtogroup group_general
* @{
*/
void    pos_Write(uint8 value);
void    pos_SetDriveMode(uint8 mode);
uint8   pos_ReadDataReg(void);
uint8   pos_Read(void);
void    pos_SetInterruptMode(uint16 position, uint16 mode);
uint8   pos_ClearInterrupt(void);
/** @} general */

/***************************************
*           API Constants        
***************************************/
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup driveMode Drive mode constants
     * \brief Constants to be passed as "mode" parameter in the pos_SetDriveMode() function.
     *  @{
     */
        #define pos_DM_ALG_HIZ         PIN_DM_ALG_HIZ
        #define pos_DM_DIG_HIZ         PIN_DM_DIG_HIZ
        #define pos_DM_RES_UP          PIN_DM_RES_UP
        #define pos_DM_RES_DWN         PIN_DM_RES_DWN
        #define pos_DM_OD_LO           PIN_DM_OD_LO
        #define pos_DM_OD_HI           PIN_DM_OD_HI
        #define pos_DM_STRONG          PIN_DM_STRONG
        #define pos_DM_RES_UPDWN       PIN_DM_RES_UPDWN
    /** @} driveMode */
/** @} group_constants */
    
/* Digital Port Constants */
#define pos_MASK               pos__MASK
#define pos_SHIFT              pos__SHIFT
#define pos_WIDTH              1u

/* Interrupt constants */
#if defined(pos__INTSTAT)
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in pos_SetInterruptMode() function.
     *  @{
     */
        #define pos_INTR_NONE      (uint16)(0x0000u)
        #define pos_INTR_RISING    (uint16)(0x0001u)
        #define pos_INTR_FALLING   (uint16)(0x0002u)
        #define pos_INTR_BOTH      (uint16)(0x0003u) 
    /** @} intrMode */
/** @} group_constants */

    #define pos_INTR_MASK      (0x01u) 
#endif /* (pos__INTSTAT) */


/***************************************
*             Registers        
***************************************/

/* Main Port Registers */
/* Pin State */
#define pos_PS                     (* (reg8 *) pos__PS)
/* Data Register */
#define pos_DR                     (* (reg8 *) pos__DR)
/* Port Number */
#define pos_PRT_NUM                (* (reg8 *) pos__PRT) 
/* Connect to Analog Globals */                                                  
#define pos_AG                     (* (reg8 *) pos__AG)                       
/* Analog MUX bux enable */
#define pos_AMUX                   (* (reg8 *) pos__AMUX) 
/* Bidirectional Enable */                                                        
#define pos_BIE                    (* (reg8 *) pos__BIE)
/* Bit-mask for Aliased Register Access */
#define pos_BIT_MASK               (* (reg8 *) pos__BIT_MASK)
/* Bypass Enable */
#define pos_BYP                    (* (reg8 *) pos__BYP)
/* Port wide control signals */                                                   
#define pos_CTL                    (* (reg8 *) pos__CTL)
/* Drive Modes */
#define pos_DM0                    (* (reg8 *) pos__DM0) 
#define pos_DM1                    (* (reg8 *) pos__DM1)
#define pos_DM2                    (* (reg8 *) pos__DM2) 
/* Input Buffer Disable Override */
#define pos_INP_DIS                (* (reg8 *) pos__INP_DIS)
/* LCD Common or Segment Drive */
#define pos_LCD_COM_SEG            (* (reg8 *) pos__LCD_COM_SEG)
/* Enable Segment LCD */
#define pos_LCD_EN                 (* (reg8 *) pos__LCD_EN)
/* Slew Rate Control */
#define pos_SLW                    (* (reg8 *) pos__SLW)

/* DSI Port Registers */
/* Global DSI Select Register */
#define pos_PRTDSI__CAPS_SEL       (* (reg8 *) pos__PRTDSI__CAPS_SEL) 
/* Double Sync Enable */
#define pos_PRTDSI__DBL_SYNC_IN    (* (reg8 *) pos__PRTDSI__DBL_SYNC_IN) 
/* Output Enable Select Drive Strength */
#define pos_PRTDSI__OE_SEL0        (* (reg8 *) pos__PRTDSI__OE_SEL0) 
#define pos_PRTDSI__OE_SEL1        (* (reg8 *) pos__PRTDSI__OE_SEL1) 
/* Port Pin Output Select Registers */
#define pos_PRTDSI__OUT_SEL0       (* (reg8 *) pos__PRTDSI__OUT_SEL0) 
#define pos_PRTDSI__OUT_SEL1       (* (reg8 *) pos__PRTDSI__OUT_SEL1) 
/* Sync Output Enable Registers */
#define pos_PRTDSI__SYNC_OUT       (* (reg8 *) pos__PRTDSI__SYNC_OUT) 

/* SIO registers */
#if defined(pos__SIO_CFG)
    #define pos_SIO_HYST_EN        (* (reg8 *) pos__SIO_HYST_EN)
    #define pos_SIO_REG_HIFREQ     (* (reg8 *) pos__SIO_REG_HIFREQ)
    #define pos_SIO_CFG            (* (reg8 *) pos__SIO_CFG)
    #define pos_SIO_DIFF           (* (reg8 *) pos__SIO_DIFF)
#endif /* (pos__SIO_CFG) */

/* Interrupt Registers */
#if defined(pos__INTSTAT)
    #define pos_INTSTAT            (* (reg8 *) pos__INTSTAT)
    #define pos_SNAP               (* (reg8 *) pos__SNAP)
    
	#define pos_0_INTTYPE_REG 		(* (reg8 *) pos__0__INTTYPE)
#endif /* (pos__INTSTAT) */

#endif /* CY_PSOC5A... */

#endif /*  CY_PINS_pos_H */


/* [] END OF FILE */
