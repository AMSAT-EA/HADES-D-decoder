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
 * File:   scrambler.h
 * Author: Gabriel Otero Pérez
 *
 * Created on November 14, 2019, 9:27 PM
 */

#include <stdint.h>

#ifndef SCRAMBLER_H
#define	SCRAMBLER_H

void SelfSyncScrambler(uint8_t* data, uint16_t length);
void SelfSyncDeScrambler(uint8_t* data, uint16_t length);

#endif	/* SCRAMBLER_H */

