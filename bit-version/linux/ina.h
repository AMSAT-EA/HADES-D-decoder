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


#ifndef SRC_INA_H_
#define SRC_INA_H_
void INAReset(void);
void INARead(void);
void BWRead(void);


#define _clock (SYSTEM_STATUS.CLOCK*1000+clock_time_ms) //mal, arreglar

#define NONE    0
#define ATT1    0
#define ATT2    1
#define ATT4    2
#define ATT8    3
#define MPPTEFF 100   //eficiencia MPPT
#define MPPTV   4000   //mV tension nominal generada por MPPT
#define IPL     700*10 //corriente nominal en PL 700mA*10 divisiones
#define MAXINA  11
#define MAXINE MAXINA-1


#define PUBLIC extern

#define u16 uint16_t
#define s16  int16_t

PUBLIC u16   periodo_integracion;  //recalculo estadisticas cada 95mi=5700s ajustable via TC 
PUBLIC char* tm_ina_buf_ptr;
PUBLIC u16   tm_ina_buf_len;
PUBLIC char* tm_bw_buf_ptr;
PUBLIC u16   tm_bw_buf_len;



#pragma pack(1)  //https://stackoverflow.com/questions/3318410/pragma-pack-effect
typedef struct  _tlmbw {
   uint16_t v1oc; //medida con INA-lado-PL
   uint16_t v1;
   uint16_t i1;
   uint16_t i1pk;
   uint16_t r1;
   uint16_t v2oc; //medida con INA-lado-BUS
   uint16_t v2;
   uint16_t r2;
   uint32_t t0;
   uint16_t td;
   uint8_t  state_begin;
   uint8_t  state_end;
   uint8_t  state_now;
   uint8_t  enable;
   uint8_t  counter;
   uint8_t  tmp; //temperatura del sistema
   } __attribute__((packed)) TLMBW;

//contabilidad interna
typedef struct _ina {

    uint8_t s; //state

    uint16_t err;
    uint16_t bus; //div
    int16_t shu;
    int16_t shu_tlm; //lo que se envia por telemetria

    uint16_t v; //valores fisicos
    int32_t i;
    int64_t p;
    int64_t e;

    uint16_t vp; //picos
    uint16_t ip;
    uint64_t pp;

    uint32_t t;  //instante anterior lectura
    uint32_t dt; //tiempo transcurrido
    } __attribute__((packed)) INA;


typedef struct _ine {
        s16 v; //instantanea
        s16 i; //instantanea
        s16 p; //media
        s16 vp;//pico
        s16 ip;//pico
        s16 pp;//pico
        } __attribute__((packed)) INE;

#endif
