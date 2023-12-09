/*************************************************************************************/
/*                                                                                   */
/* License information / Informacion de licencia                                     */
/*                                                                                   */
/* Todos los contenidos de la web de AMSAT EA se distribuyen                         */
/* bajo licencia Creative Commons CC BY 4.0 Internacional                            */
/* (distribución, modificacion u y uso libre mientras se cite AMSAT EA como fuente). */
/*                                                                                   */
/* All the contents of the AMSAT EA website are distributed                          */
/*  under a Creative Commons CC BY 4.0 International license                         */
/* (free distribution, modification and use crediting AMSAT EA as the source).       */
/*                                                                                   */
/* AMSAT EA 2023                                                                     */
/* https://www.amsat-ea.org/proyectos/                                               */
/*                                                                                   */
/*************************************************************************************/


/*
 *                                                       
 * Project     : GENESIS                                            
 * File        : genesis_crc.h
 *
 * Description : GENESIS MCU main program
 *               Gabriel Otero
 * Last update : 10 October 2016                                              
 *                                                                            
*/

#include <stdint.h>

#ifndef CRC_H
#define	CRC_H

uint16_t crc16(uint8_t* data_p, uint16_t length);

#endif	/* CRC_H */

