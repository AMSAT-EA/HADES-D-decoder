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
 * File        : genesis_util_transmission.h
 *
 * Description : This file includes all the function prototypes
 * Last update : 07 October 2016                                              
 *                                                                            
*/

#ifndef GENESIS_UTIL_TRANSMISSION
#define GENESIS_UTIL_TRANSMISSION

#include <stdio.h>

#include "genesis.h"
#include "ina.h"


#define PACKET_ADDRESS_HADES_D              	8 // 8 FOR HADES-D

#define BYTE_SIZE                           	8
#define DELAY_MS_BETWEEN_BITS               	20 		// for 50 bits / second

#define	PACKET_HEADER_SIZE			10 		// 8 training + 2 de sync

#define PACKET_TYPE_POWER_TELEMETRY         	0x18	// tipo 1 direccion 8
#define PACKET_TYPE_TEMP_TELEMETRY        	0x28	// tipo 2 direccion 8
#define PACKET_TYPE_STATUS_TELEMETRY        	0x38
#define PACKET_TYPE_POWERSTATS_TELEMETRY    	0x48
#define PACKET_TYPE_TEMPSTATS_TELEMETRY    	0x58
#define PACKET_TYPE_SUNVECTOR_TELEMETRY    	0x68
#define PACKET_TYPE_RADIOMETER_TELEMETRY    	0x78
#define PACKET_TYPE_DEPLOY_TELEMETRY    	0x88
#define PACKET_TYPE_EXTENDERPOWER_INE_TELEMETRY 0x98
#define PACKET_TYPE_CHESS_MOVE			0xA 	// subida a satélite
#define PACKET_TYPE_CHESS_TELEMETRY    		0xB8

#define PACKET_FREE                         	0x00

#define TELEMETRY_PACKETTYPE_SIZE           	2
#define TELEMETRY_ADDRESS_SIZE              	4
#define EEPROM_SIZE                         	256
#define MAX_SPIN_SAMPLES                    	12
#define MAX_RADIOMETER_SAMPLES              	60
#define MAX_INE 								10

#define PACKET_TRAINING_HEADER 					0xAAAAAAAAAAAAAAAA
#define PACKET_SYNC_HEADER    					0x35BF // sync es 0xBF35 pero se almacena al revés

#define PACKET_TRAINING_HEADER_BYTE				0xAA
#define PACKET_SYNC_HEADER_HI  					(PACKET_SYNC_HEADER >> 8)
#define PACKET_SYNC_HEADER_LO 					(PACKET_SYNC_HEADER & 0x00FF) // sync es 0xBF35 pero se almacena al revés

extern char* 	tm_ina_buf_ptr;
extern uint16_t tm_ina_buf_len;
extern char* 	tm_bw_buf_ptr;
extern uint16_t tm_bw_buf_len;

// structures for the different packet types

// formato de paquete power

typedef struct st_power_packet {

    uint64_t training;
    uint16_t sync;
    uint8_t  packettype_address;

    uint8_t  spa;
	uint8_t  spb;
	uint8_t  spc;
	uint8_t  spd;
	uint8_t  spe;
	uint8_t  spf;
	uint16_t vbusadc_vbatadc_hi;     // 12 y 4
	uint16_t vbatadc_lo_vcpuadc_hi;  // 8  y 8
	uint16_t vcpuadc_lo_vbus2;	 // 4  y 12
	uint16_t vbus3_vbat2_hi;		 // 12 y 4
	uint16_t vbat2_lo_ibat_hi;		 // 8  y 8  -- iepsi2c son 16 bits
	uint16_t ibat_lo_icpu_hi;	 // 8  y 8
	uint16_t icpu_lo_ipl;		 // 4  y 12
	uint8_t  powerdul;
	uint8_t  powerdul455;
	uint8_t  vdac;

	uint16_t checksum;


} __attribute__((packed)) power_packet;

// formato de paquete temp

typedef struct st_temp_packet {

    uint64_t training;
    uint16_t sync;
    uint8_t  packettype_address;

    // se mandan sin signo porque van en incrementos de 0.5C con LUT

    uint8_t  tpa;
    uint8_t  tpb;
    uint8_t  tpc;
    uint8_t  tpd;
    uint8_t  tpe;
    uint8_t  teps;
    uint8_t  ttx;
    uint8_t  ttx2;
    uint8_t  trx;
    uint8_t  tcpu;
    
    uint16_t checksum;

} __attribute__((packed)) temp_packet ;

// formato de paquete status

typedef struct st_status_packet {

    uint64_t training;
    uint16_t sync;
    uint8_t  packettype_address;

    uint32_t sclock;
    uint16_t uptime;
    uint16_t nrun;
    uint8_t  npayload;
    uint8_t  nwire;
    uint8_t  nbusdrops_lastreset;
    uint8_t  bate_mote;

	uint8_t  nTasksNotExecuted;
	uint8_t  antennaDeployed;
	uint8_t  nExtEepromErrors;

	uint8_t  failed_task_id;
	uint8_t  orbperiod;

	uint8_t  strfwd0;
	uint16_t strfwd1;
	uint16_t strfwd2;
	uint8_t  strfwd3;

	uint16_t checksum;


} __attribute__((packed)) status_packet;


// formato de paquete para estadísticas power

typedef struct st_power_stats_packet {

    uint64_t training;
    uint16_t sync;
    uint8_t  packettype_address;

	uint16_t minvbusadc_vbatadc_hi;
	uint16_t minvbatadc_lo_vcpuadc_hi;
	uint8_t  minvcpuadc_lo_free;
	uint8_t  minvbus2;
	uint8_t  minvbus3;
	uint8_t  minvbat2;
	uint8_t  minibat;
	uint8_t  minicpu;
	uint8_t  minipl;
	uint8_t  minpowerdul;
	uint8_t  minpowerdul455;
	uint8_t  minvdac;
	uint16_t maxvbusadc_vbatadc_hi;
	uint16_t maxvbatadc_lo_vcpuadc_hi;
	uint8_t  maxvcpuadc_lo_free;
	uint8_t  maxvbus2;
	uint8_t  maxvbus3;
	uint8_t  maxvbat2;
	uint8_t  maxibat;
	uint8_t  maxicpu;
	uint8_t  maxipl;
	uint8_t  maxpowerdul;
	uint8_t  maxpowerdul455;
	uint8_t  maxvdac;
	uint16_t medvbusadc_vbatadc_hi;
	uint16_t medvbatadc_lo_vcpuadc_hi;
	uint8_t  medvcpuadc_lo_free;
	uint8_t  ibat_rx_charging;
	uint8_t  ibat_rx_discharging;
	uint8_t  ibat_tx_low_power_charging;
	uint8_t  ibat_tx_low_power_discharging;
	uint8_t  ibat_tx_high_power_charging;
	uint8_t  ibat_tx_high_power_discharging;
	uint8_t  medpowerdul;
	uint8_t  medpowerdul455;
	uint8_t  medvdac;

	uint16_t checksum;

} __attribute__((packed)) power_stats_packet;


// formato de paquete para estadísticas power

typedef struct st_temp_stats_packet {

    uint64_t training;
    uint16_t sync;
    uint8_t  packettype_address;

    // es correcto como uint8, se envían con LUT en incrementos de 0.5C

    uint8_t  mintpa;
	uint8_t  mintpb;
	uint8_t  mintpc;
	uint8_t  mintpd;
	uint8_t  mintpe;
	uint8_t  minteps;
	uint8_t  minttx;
	uint8_t  minttx2;
	uint8_t  mintrx;
	uint8_t  mintcpu;
	uint8_t  maxtpa;
	uint8_t  maxtpb;
	uint8_t  maxtpc;
	uint8_t  maxtpd;
	uint8_t  maxtpe;
	uint8_t  maxteps;
	uint8_t  maxttx;
	uint8_t  maxttx2;
	uint8_t  maxtrx;
	uint8_t  maxtcpu;
	uint8_t  medtpa;
	uint8_t  medtpb;
	uint8_t  medtpc;
	uint8_t  medtpd;
	uint8_t  medtpe;
	uint8_t  medteps;
	uint8_t  medttx;
	uint8_t  medttx2;
	uint8_t  medtrx;
	uint8_t  medtcpu;

	uint16_t checksum;

} __attribute__((packed)) temp_stats_packet;


typedef struct st_sunvector_packet {

    uint64_t training;
    uint16_t sync;
    uint8_t  packettype_address;

    char *   raw_data_pointer;
    uint8_t  raw_data_size;

    uint16_t checksum;

} __attribute__((packed)) sunvector_packet;


typedef struct st_radiometer_packet {

    uint64_t training;
    uint16_t sync;
    uint8_t  packettype_address;

    // 90 muestras (una �rbita), con una muestra por minuto que ha medido la media de ruido de ese minuto
    
    // instante de reloj de la primera muestra    
    uint32_t clock;                          // sample clock            - 32 bits
    
    // las muestras est�n espaciadas 1 hora
    
    uint8_t  radiometer_sample[MAX_RADIOMETER_SAMPLES];  // medias de DUL
    
    uint16_t checksum;
    
} __attribute__((packed)) radiometer_packet;



typedef struct st_deploy_packet {

    uint64_t training;
    uint16_t sync;
    uint8_t  packettype_address;

    char *   raw_data_pointer;
    uint8_t  raw_data_size;

	uint16_t checksum;

} __attribute__((packed)) deploy_packet;


typedef struct st_sunsensors_packet {

    uint64_t training;
    uint16_t sync;
    uint8_t  packettype_address;

    uint32_t firstclock;
    uint8_t  vpa1;
	uint8_t  vpa2;
	uint8_t  vpb1;
	uint8_t  vpb2;
	uint8_t  vpc1;
	uint8_t  vpc2;
	uint8_t  vpd1;
	uint8_t  vpd2;
	uint8_t  vpe1;
	uint8_t  vpe2;
	uint8_t  vpf1;
	uint8_t  vpf2;
	uint8_t  vppa1;
	uint8_t  vppa2;
	uint8_t  vppb1;
	uint8_t  vppb2;
	uint8_t  vppc1;
	uint8_t  vppc2;
	uint8_t  vppd1;
	uint8_t  vppd2;
	uint8_t  vppe1;
	uint8_t  vppe2;
	uint8_t  vppf1;
	uint8_t  vppf2;

	uint16_t checksum;

} __attribute__((packed)) sunsensors_packet;


typedef struct st_extendedpower_ine_packet {

    uint64_t training;
    uint16_t sync;
    uint8_t  packettype_address;

    char *   raw_data_pointer;
    uint8_t  raw_data_size;

	uint16_t checksum;

} __attribute__((packed)) extendedpower_ine_packet;


typedef struct st_chess_move_packet {

    uint64_t training;
    uint16_t sync;
    uint8_t  packettype_address;

    uint8_t callsign[6];
	uint8_t source;
	uint8_t destination;

	uint16_t checksum;

} __attribute__((packed)) chess_move_packet;


typedef struct st_chess_board_packet {

    uint64_t training;
    uint16_t sync;
    uint8_t  packettype_address;

    uint8_t  callsign[6];

    uint8_t  player_color;
	uint16_t last_move;

	uint8_t  game_status;

	/* Representación en memoria del tablero

	   01234567
	   ABCDEFGH

	08 TCADRACT
	17 PPPPPPPP NEGRAS
	26 ........
	35 ........
	44 ........
	53 ........
	62 PPPPPPPP BLANCAS
	71 TCADRACT

	   01234567
	   ABCDEFGH
	*/

	uint8_t  tablero[32]; // posiciones de 4 bits

	uint16_t checksum;

} __attribute__((packed)) chess_board_packet;


#define MAX_TX_BUFFER_SIZE 512

extern uint32_t global_clock_copy_for_tasks;


//extern unsigned char diff_semaphore;
//extern unsigned int global_diff;

uint8_t celsius_to_tx(float celsius);


#endif
