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


#ifndef GENESIS_EFEMERIDES_H
#define	GENESIS_EFEMERIDES_H

#include <stdint.h>
#define u8   uint8_t
#define u16  uint16_t
#define s16   int16_t
#define u32  uint32_t

#include"genesis_efemerides_prop.h"

#define PACKEF __attribute__((packed, scalar_storage_order("big-endian")))

typedef struct {
	u16   adr;
	u32   ful;
	u32   fdl;
	tle_t tle; 
	} 
	PACKEF _sat;

typedef struct {
	u32 train1;
	u32 train2;
	u16 sync;
	u8  len;
	u32 utc;
   _sat sat; //[3]
	s16 lat;
	s16 lon;
	u16 alt;
	u8  cnt;
	u16 crc;
	} 
	PACKEF _frm;

#endif
