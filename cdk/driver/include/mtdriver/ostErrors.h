/* 
 * The contents of this file are subject to the NOKOS 
 * License Version 1.0 (the "License"); you may not use
 * this file except in compliance with the License. 
 * 
 * Software distributed under the License is distributed
 * on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND,
 * either express or implied. See the License for the
 * specific language governing rights and limitations
 * under the License. 
 * 
 * The Original Software is:
 * ostErrors.h
 * 
 * Copyright (c) 2001 Nokia and others. All Rights Reserved. 
 * 
 * Contributor(s):
 */

/**
 * \file ostErrors.h
 *
 * \brief Error codes for OST
 *
 * \version $Id: ostErrors.h,v 1.3 2001/07/21 20:37:04 gillem Exp $
 *
 * \date <Add date>
 *
 * \author Rickard Westman, Nokia Home Communications
 *
 * This file contains common error codes for software modules in the 
 * OST, as well as base codes for module-specific error codes. 
 * See section "Error Codes" in "C/C++/Java Coding Practices for
 * Nokia NMT/NHC".
 */

#ifndef _OST_ERRORS_H_
#define _OST_ERRORS_H_

#ifdef __cplusplus
extern "C" {
#endif

/* ostErrors.h
 *
 * Copyright (c) 1999 Nokia Home Communications,
 *
 * Project:
 *      OST
 */

/* $Id: ostErrors.h,v 1.3 2001/07/21 20:37:04 gillem Exp $ */

/*
 * Defines and consts
 */

#define OK 0

#define OST_ERROR_BASE      800               /* Base for generic OST errors  */

// nicht so sauber ... ist im dmx.h auch drin
#ifndef EBUFFEROVERFLOW
#define EBUFFEROVERFLOW   (OST_ERROR_BASE+0)  /* Buffer overflow           */
#endif

#define ECRC              (OST_ERROR_BASE+1)  /* CRC error                 */
#define EINTERNAL         (OST_ERROR_BASE+2)  /* Unexpected internal error */
#define ESTATE            (OST_ERROR_BASE+3)  /* Device not in correct state */
#define EOLD              (OST_ERROR_BASE+4)  /* DVBSide SW too old        */

#define CC_ERROR_BASE      2000   /* Communication Controller */
#define IRIN_ERROR_BASE    2100   /* IR input module */
#define DVBFE_ERROR_BASE    900   /* DVB front-end device */
#define SEC_ERROR_BASE      910   /* Satellite Equipment Control device */
#define SC_ERROR_BASE       920   /* Smart card device */
#define CI_ERROR_BASE       930   /* Common Interface device */
#define FPD_ERROR_BASE       940   /* Front Panel device */

#define CC_DEVICE_ERROR_BASE 0x10000 /* Device specific errors for CC */
#ifdef __cplusplus
}
#endif 

#endif /*#ifndef _OST_ERRORS_H */
