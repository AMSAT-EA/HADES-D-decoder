# HADES-D-decoder
This is the ublic repository for the decoder of HADES-D satellite. It contains two versions:

- bit version
The bit version accepts 0 and 1 (as characters) from the standard input. (So use < to redirect from file or source). These are the raw bits generated by the demodulator.
When it finds sync (BF35) it begins to decode the incoming packet (checking CRC and doing the descrambling). It a bit is not received in time, it aborts the decoding (signal lost).

There is more information and also the decoder in AMSAT-EA web:
https://www.amsat-ea.org/proyectos/

- byte version
The byte version accepts as parameter a file containing an already descrambled and CRC checked byte sequence. It justs converts it to the original packet.
This program has been made to work with third party demodulators that deliver bytes.

For any questions, please write us to contacto@amsat-ea.org

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





