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


#ifndef GENESIS_EFEMERIDES_PROP_H
#define	GENESIS_EFEMERIDES_PROP_H

#define DOUBLE double
#include <stdint.h>
#define u32 uint32_t

typedef struct { DOUBLE xe,  ye,   ze;  } vector_recef;
typedef struct { DOUBLE lat, lon,  alt; } vector_lla;
typedef struct { DOUBLE azi, elev, rng; } vector_aer;

//estos datos vuelan como se realiza el computo
#define PACKEF2 __attribute__((packed))
//#define PACKEF2 __attribute__((packed),scalar_storage_order("little-endian"))
typedef struct { u32 epoch; float xndt2o, xndd6o, bstar, xincl, xnodeo, eo, omegao, xmo, xno; } PACKEF2 tle_t;

#endif
