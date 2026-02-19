/*********************************************************************************

Copyright(c) 2020-2021 Analog Devices, Inc. All Rights Reserved.

This software is proprietary.  By using this software you agree
to the terms of the associated Analog Devices License Agreement.

*********************************************************************************/
/*!
 *  @addtogroup SPDIF_TX_Driver Transmit S/PDIF Driver
 *  @{
 */

/*!
 * @addtogroup SPDIF_TX_Driver_Static_Configuration Transmit S/PDIF Driver Static Configuration
 * @{
  Static Configuration provides an easy and convient way for application to configure the device during driver's open
  call. When an application provides custom static configuration file, then according to the setting provided driver initialises
  the device during driver's open call. If application fails to provide any custom configuration file, then by default S/PDIF TX device
  driver does not perform any static configuration.
 */

/*!
 * @file     adi_spdif_tx_config_SC59x.h
 *
 * @brief    Transmit S/PDIF static configuration file
 *
 * @version  $Revision: 37632 $
 *
 * @date     $Date: 2019-10-09 12:35:37 +0530 (Wed, 09 Oct 2019) $
 *
 * @details
 *           This is the static configuration file for the transmit S/PDIF Driver.
 */



#ifndef __ADI_SPDIF_TX_CONFIG_SC59X_H__
#define __ADI_SPDIF_TX_CONFIG_SC59X_H__


/**********************************************************************************************
 *					       SPDIF 0 TX
 **********************************************************************************************/

/*! Enable Static configuration for SPDIF TX0 */
#define USE_STATIC_SPDIF0_TX 0u

/*! SPDIF0 TX Over sampling clock. Set 0 for 256xFs and 1 for 384xFs */
#define SPDIF0_TX_FREQ               0u

/*! SPDIF0 TX single channel double frequency enable */ 
#define SPDIF0_TX_SCDF               0u

/*! SPDIF0 TX single channel double frequency channel selection */ 
#define SPDIF0_TX_SCDFLR             0u

/*! SPDIF0 TX input mode selection */ 
#define SPDIF0_TX_SMODEIN            0u

/*! SPDIF0 TX Auto enable */
#define SPDIF0_TX_AUTO               0u

/*! SPDIF0 TX Validity left */
#define SPDIF0_TX_VALIDL             0u

/*! SPDIF0 TX Validity right */
#define SPDIF0_TX_VALIDR             0u

/*! SPDIF0 TX External Sync enable */
#define SPDIF0_EXTSYNC               0u

/*! SPDIF0 TX STAT0A in SPDIF control register*/
#define SPDIF0_TXCTL_STAT0A          0u

/*! SPDIF0 Stat0B in SPDIF control register*/
#define SPDIF0_TXCTL_STAT0B          0u

/*! SPDIF0 TX STATA0 */
#define ADI_SPDIF0_TX_STATA0             0u

/*! SPDIF0 TX STATA1 */
#define ADI_SPDIF0_TX_STATA1             0u

/*! SPDIF0 TX STATA2 */
#define ADI_SPDIF0_TX_STATA2             0u

/*! SPDIF0 TX STATA3 */
#define ADI_SPDIF0_TX_STATA3             0u

/*! SPDIF0 TX STATA4 */
#define ADI_SPDIF0_TX_STATA4             0u

/*! SPDIF0 TX STATA5 */
#define ADI_SPDIF0_TX_STATA5             0u

/*! SPDIF0 TX STATB0  */
#define ADI_SPDIF0_TX_STATB0             0u

/*! SPDIF0 TX STATB1  */
#define ADI_SPDIF0_TX_STATB1             0u

/*! SPDIF0 TX STATB2  */
#define ADI_SPDIF0_TX_STATB2             0u

/*! SPDIF0 TX STATB3  */
#define ADI_SPDIF0_TX_STATB3             0u

/*! SPDIF0 TX STATB4  */
#define ADI_SPDIF0_TX_STATB4             0u

/*! SPDIF0 TX STATB5  */
#define ADI_SPDIF0_TX_STATB5             0u


/*! SPDIF0 TX USERBUF A0  */
#define ADI_SPDIF0_TX_UBUFFA0             0u

/*! SPDIF0 TX USERBUF A1  */
#define ADI_SPDIF0_TX_UBUFFA1             0u

/*! SPDIF0 TX USERBUF A2  */
#define ADI_SPDIF0_TX_UBUFFA2             0u

/*! SPDIF0 TX USERBUF A3  */
#define ADI_SPDIF0_TX_UBUFFA3             0u

/*! SPDIF0 TX USERBUF A4  */
#define ADI_SPDIF0_TX_UBUFFA4             0u

/*! SPDIF0 TX USERBUF A5  */
#define ADI_SPDIF0_TX_UBUFFA5             0u

/*! SPDIF0 TX USERBUF B0  */
#define ADI_SPDIF0_TX_UBUFFB0             0u

/*! SPDIF0 TX USERBUF B1  */
#define ADI_SPDIF0_TX_UBUFFB1             0u

/*! SPDIF0 TX USERBUF B2  */
#define ADI_SPDIF0_TX_UBUFFB2             0u

/*! SPDIF0 TX USERBUF B3  */
#define ADI_SPDIF0_TX_UBUFFB3             0u

/*! SPDIF0 TX USERBUF B4  */
#define ADI_SPDIF0_TX_UBUFFB4             0u

/*! SPDIF0 TX USERBUF B5  */
#define ADI_SPDIF0_TX_UBUFFB5             0u

/* Static configuration for SPDIF0 TX */
#define ADI_SPDIF0_TX_FREQ               ((uint32_t)SPDIF0_TX_FREQ<<BITP_SPDIF_TX_CTL_FREQ)
#define ADI_SPDIF0_TX_SCDF               ((uint32_t)SPDIF0_TX_SCDF<<BITP_SPDIF_TX_CTL_SCDF)
#define ADI_SPDIF0_TX_SCDFLR             ((uint32_t)SPDIF0_TX_SCDFLR<<BITP_SPDIF_TX_CTL_SCDFLR)
#define ADI_SPDIF0_TX_SMODEIN            ((uint32_t)SPDIF0_TX_SMODEIN<<BITP_SPDIF_TX_CTL_SMODEIN)
#define ADI_SPDIF0_TX_AUTO               ((uint32_t)SPDIF0_TX_AUTO<<BITP_SPDIF_TX_CTL_AUTO)
#define ADI_SPDIF0_TX_VALIDL             ((uint32_t)SPDIF0_TX_VALIDL<<BITP_SPDIF_TX_CTL_VALIDL)
#define ADI_SPDIF0_TX_VALIDR             ((uint32_t)SPDIF0_TX_VALIDR<<BITP_SPDIF_TX_CTL_VALIDR)
#define ADI_SPDIF0_EXTSYNC               ((uint32_t)SPDIF0_EXTSYNC<<BITP_SPDIF_TX_CTL_EXTSYNC)

/* SPDIF0 TX CTL Reg*/
#define ADI_SPDIF0_TX_CTL ((uint32_t)SPDIF0_TXCTL_STAT0A<<BITP_SPDIF_TX_CTL_BYTE0A|(uint32_t) SPDIF0_TXCTL_STAT0B<<BITP_SPDIF_TX_CTL_BYTE0B|ADI_SPDIF0_TX_FREQ|ADI_SPDIF0_TX_SCDF|ADI_SPDIF0_TX_SCDFLR|ADI_SPDIF0_TX_SMODEIN|ADI_SPDIF0_TX_AUTO|ADI_SPDIF0_TX_VALIDL|ADI_SPDIF0_TX_VALIDR|ADI_SPDIF0_EXTSYNC)


/**********************************************************************************************
 *					       SPDIF 1 TX
 **********************************************************************************************/

/*! Enable Static configuration for SPDIF TX1 */
#define USE_STATIC_SPDIF1_TX 0u

/*! SPDIF1 TX Over sampling clock. Set 0 for 256xFs and 1 for 384xFs */
#define SPDIF1_TX_FREQ               0u

/*! SPDIF1 TX single channel double frequency enable */ 
#define SPDIF1_TX_SCDF               0u

/*! SPDIF1 TX single channel double frequency channel selection */ 
#define SPDIF1_TX_SCDFLR             0u

/*! SPDIF1 TX input mode selection */ 
#define SPDIF1_TX_SMODEIN            0u

/*! SPDIF1 TX Auto enable */
#define SPDIF1_TX_AUTO               0u

/*! SPDIF1 TX Validity left */
#define SPDIF1_TX_VALIDL             0u

/*! SPDIF1 TX Validity right */
#define SPDIF1_TX_VALIDR             0u

/*! SPDIF1 TX External Sync enable */
#define SPDIF1_EXTSYNC               0u

/*! SPDIF1 TX STAT0A in SPDIF control register*/
#define SPDIF1_TXCTL_STAT0A          0u

/*! SPDIF1 Stat0B in SPDIF control register*/
#define SPDIF1_TXCTL_STAT0B          0u

/*! SPDIF1 TX STATA0 */
#define ADI_SPDIF1_TX_STATA0             0u

/*! SPDIF1 TX STATA1 */
#define ADI_SPDIF1_TX_STATA1             0u

/*! SPDIF1 TX STATA2 */
#define ADI_SPDIF1_TX_STATA2             0u

/*! SPDIF1 TX STATA3 */
#define ADI_SPDIF1_TX_STATA3             0u

/*! SPDIF1 TX STATA4 */
#define ADI_SPDIF1_TX_STATA4             0u

/*! SPDIF1 TX STATA5 */
#define ADI_SPDIF1_TX_STATA5             0u

/*! SPDIF1 TX STATB0  */
#define ADI_SPDIF1_TX_STATB0             0u

/*! SPDIF1 TX STATB1  */
#define ADI_SPDIF1_TX_STATB1             0u

/*! SPDIF1 TX STATB2  */
#define ADI_SPDIF1_TX_STATB2             0u

/*! SPDIF1 TX STATB3  */
#define ADI_SPDIF1_TX_STATB3             0u

/*! SPDIF1 TX STATB4  */
#define ADI_SPDIF1_TX_STATB4             0u

/*! SPDIF1 TX STATB5  */
#define ADI_SPDIF1_TX_STATB5             0u


/*! SPDIF1 TX USERBUF A0  */
#define ADI_SPDIF1_TX_UBUFFA0             0u

/*! SPDIF1 TX USERBUF A1  */
#define ADI_SPDIF1_TX_UBUFFA1             0u

/*! SPDIF1 TX USERBUF A2  */
#define ADI_SPDIF1_TX_UBUFFA2             0u

/*! SPDIF1 TX USERBUF A3  */
#define ADI_SPDIF1_TX_UBUFFA3             0u

/*! SPDIF1 TX USERBUF A4  */
#define ADI_SPDIF1_TX_UBUFFA4             0u

/*! SPDIF1 TX USERBUF A5  */
#define ADI_SPDIF1_TX_UBUFFA5             0u

/*! SPDIF1 TX USERBUF B0  */
#define ADI_SPDIF1_TX_UBUFFB0             0u

/*! SPDIF1 TX USERBUF B1  */
#define ADI_SPDIF1_TX_UBUFFB1             0u

/*! SPDIF1 TX USERBUF B2  */
#define ADI_SPDIF1_TX_UBUFFB2             0u

/*! SPDIF1 TX USERBUF B3  */
#define ADI_SPDIF1_TX_UBUFFB3             0u

/*! SPDIF1 TX USERBUF B4  */
#define ADI_SPDIF1_TX_UBUFFB4             0u

/*! SPDIF1 TX USERBUF B5  */
#define ADI_SPDIF1_TX_UBUFFB5             0u

/* Static configuration for SPDIF1 TX */
#define ADI_SPDIF1_TX_FREQ               ((uint32_t)SPDIF1_TX_FREQ<<BITP_SPDIF_TX_CTL_FREQ)
#define ADI_SPDIF1_TX_SCDF               ((uint32_t)SPDIF1_TX_SCDF<<BITP_SPDIF_TX_CTL_SCDF)
#define ADI_SPDIF1_TX_SCDFLR             ((uint32_t)SPDIF1_TX_SCDFLR<<BITP_SPDIF_TX_CTL_SCDFLR)
#define ADI_SPDIF1_TX_SMODEIN            ((uint32_t)SPDIF1_TX_SMODEIN<<BITP_SPDIF_TX_CTL_SMODEIN)
#define ADI_SPDIF1_TX_AUTO               ((uint32_t)SPDIF1_TX_AUTO<<BITP_SPDIF_TX_CTL_AUTO)
#define ADI_SPDIF1_TX_VALIDL             ((uint32_t)SPDIF1_TX_VALIDL<<BITP_SPDIF_TX_CTL_VALIDL)
#define ADI_SPDIF1_TX_VALIDR             ((uint32_t)SPDIF1_TX_VALIDR<<BITP_SPDIF_TX_CTL_VALIDR)
#define ADI_SPDIF1_EXTSYNC               ((uint32_t)SPDIF0_EXTSYNC<<BITP_SPDIF_TX_CTL_EXTSYNC)

/* SPDIF1 TX CTL Reg*/
#define ADI_SPDIF1_TX_CTL ((uint32_t)SPDIF1_TXCTL_STAT0A<<BITP_SPDIF_TX_CTL_BYTE0A|(uint32_t) SPDIF1_TXCTL_STAT0B<<BITP_SPDIF_TX_CTL_BYTE0B|ADI_SPDIF1_TX_FREQ|ADI_SPDIF1_TX_SCDF|ADI_SPDIF1_TX_SCDFLR|ADI_SPDIF1_TX_SMODEIN|ADI_SPDIF1_TX_AUTO|ADI_SPDIF1_TX_VALIDL|ADI_SPDIF1_TX_VALIDR|ADI_SPDIF1_EXTSYNC)

#endif /*__ADI_SPDIF_TX_CONFIG_SC59X_H__*/

/**@}*/
/**@}*/
