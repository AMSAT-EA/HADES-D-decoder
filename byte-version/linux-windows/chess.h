/*************************************************************************************/
/*                                                                                   */
/* License information / Informacion de licencia                                     */
/*                               						     */
/* Todos los contenidos de la web de AMSAT EA se distribuyen                         */
/* bajo licencia Creative Commons CC BY 4.0 Internacional                            */
/* (distribución, modificacion u y uso libre mientras se cite AMSAT EA como fuente). */
/*                                                                                   */
/* All the contents of the AMSAT EA website are distributed                          */
/*  under a Creative Commons CC BY 4.0 International license                         */
/* (free distribution, modification and use crediting AMSAT EA as the source).       */
/*										     */
/* AMSAT EA 2023                                                                     */
/* https://www.amsat-ea.org/proyectos/                                               */
/*                                                                                   */
/*************************************************************************************/

//
// spacechess
// Programmed by Felix Paez EA4GQS and Raul de Frutos
//

#ifndef SPACECHESS_H
#define	SPACECHESS_H

#define NUM_FILAS 			8
#define NUM_COLUMNAS 			8

// tipos de pieza teniendo en cuenta el color

#define VACIO				0
#define PEON_BLANCO			1
#define TORRE_BLANCA			2
#define CABALLO_BLANCO  		3
#define ALFIN_BLANCO 			4
#define DAMA_BLANCA 			5
#define REY_BLANCO 			6

#define PEON_NEGRO 			7
#define TORRE_NEGRA 			8
#define CABALLO_NEGRO 			9
#define ALFIN_NEGRO 			10
#define DAMA_NEGRA 			11
#define REY_NEGRO 			12

// tipos de pieza sin tener en cuenta el color

#define PEON 				0
#define CABALLO 			1
#define ALFIL 				2
#define TORRE 				3
#define DAMA 				4
#define REY 				5

#define TIPO_PIEZA 			6
#define MOVIMIENTO 			56
#define DESPLAZAMIENTO 			2

#define BLANCAS 			0
#define NEGRAS 				1

#endif

