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
 * File:   scrambler.c
 * Author: Gabriel Otero Pérez
 *
 * Created on November 14, 2019, 9:27 PM
 */


#include <stdint.h>

const uint32_t InitialState = 0x2C350000;	// Initial state of the registers.

// rutina para scramblear un paquete

void SelfSyncScrambler(uint8_t* data, uint16_t length){
    
    uint32_t scrambler_LFSR = InitialState;

    uint8_t inBit, outBit;
    
    while(length--){
        
        for(int b = 7; b > 0; b--){
            
            inBit = (*data >> b) & 1;
            outBit = (inBit^(scrambler_LFSR >> 16)^(scrambler_LFSR >> 11)) & 1; // H(z) = 1 + z^-12 + z^-17.
            
            if(outBit)
                *data |= (0x01 << b);
            else
                *data &= ~(0x01 << b);
            
            scrambler_LFSR = ((scrambler_LFSR << 1)|(outBit & 1)) & 0x1FFFF;
        }
        
        data++;
        
    }
    
}


// rutina para des-scramblear un paquete

void SelfSyncDeScrambler(uint8_t* data, uint16_t length){
    
    uint8_t inBit, outBit;
    
    uint32_t descrambler_LFSR = InitialState;
    
    while(length--){
        
        for(int b = 7; b > 0; b--){
            
            inBit = (*data >> b) & 1;
            outBit = (inBit^(descrambler_LFSR >> 16)^(descrambler_LFSR >> 11)) & 1;
            
            if(outBit)
                *data |= (0x01 << b);
            else
                *data &= ~(0x01 << b);
            
            descrambler_LFSR = ((descrambler_LFSR << 1)|(inBit & 1)) & 0x1FFFF;
        }
   
        data++;
        
    }
    
}
