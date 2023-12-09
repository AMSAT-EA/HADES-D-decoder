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


#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <termios.h>  //termios, TCSANOW, ECHO, ICANON
#include <unistd.h>   //STDIN_FILENO
#include <math.h>
#include <stdio.h>

#define nzones 34
#define pi 3.1415926535898

#include "genesis_crc.h"
#include "genesis_scrambler.h"
#include "genesis_util_transmission.h"
#include "chess.h"
#include "sun.h"
#include "genesis_efemerides.h"

#define RX_BUFFER_SIZE 512
#define SYNCHRONIZED          0xBF35

#define INICIAL 	 0
#define WAITING_FOR_SYNC 1
#define READING_SIZE	 2 
#define READING_DATA	 3

const char name[MAXINA][5]={"SPA ","SPB ","SPC ","SPD ","SUN ","BAT ","BATP","BATN","CPU ","PL  ","SIM "};

void procesar(void);
void trx(double lat, double lon, int latc, int lonc, int rc, int *inout);
char* overflying(double lat, double lon);
void visualiza_efemeridespacket(_frm * ef);

FILE *f;


void main(int argc, char argv[]) {

	time_t t = time(NULL);
  	struct tm tm = *localtime(&t);
	char fecha[64];

	printf("\n");

        printf("**********************************************\n");
        printf("*                                            *\n");
        printf("*         HADES-D Telemetry Decoding         *\n");
        printf("*        AMSAT-EA - Free distribution        *\n");
        printf("*                                            *\n");
        printf("*    v1.21 Last compilation : %10s    *\n",__DATE__);
        printf("*                                            *\n");
        printf("**********************************************\n");
        printf("\n");


        sprintf(fecha, "%d%02d%02d-%02d:%02d:%02d :", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	printf("%s Running...", fecha);

	procesar();

}



// devuelve el tamaño, incluyendo checksum para el tipo de paquete (bytes utiles de Excel) (todo menos training y sync)

uint8_t telemetry_packet_size(int tipo_paquete) {

	uint8_t bytes_utiles[] = { 8, 26, 13, 26, 45, 33, 135, 67, 28, 123, 11, 45, 64 };

	/*

	0 - command
	1 - power
	2 - temps
	3 - status
	4 - power stats
	5 - temp stats
	6 - sunvector
        7 - radiometer
        8 - antenna deploy
        9 - ine
       10 - jugada ajedrez a satelite
       11 - chess 
	   12 - efemerides
	
	*/
	
	return bytes_utiles[tipo_paquete]; // utiles

}


void visualiza_chessmovepacket(chess_move_packet * packet) {

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char fecha[64];
    char callsign[7];
    char jugada[5];

    sprintf(fecha, "%d%02d%02d-%02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

    fprintf(f,"\n\n");
    fprintf(f,"*** Chess move packet received on local time %s ***\n", fecha);

    memset(callsign, 0, sizeof(callsign));
    memset(jugada, 0, sizeof(jugada));

    memcpy(callsign, packet->callsign, 6);
    memcpy(jugada, (uint8_t*)&packet->source, 2);
    memcpy(jugada+2, (uint8_t*)&packet->destination, 2);

    fprintf(f,"callsign    : %s\n",  callsign);
    fprintf(f,"move        : %s\n", jugada);

}


void visualiza_powerpacket(power_packet * packet) {

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char fecha[64];

    sprintf(fecha, "%d%02d%02d-%02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
  
    fprintf(f,"\n\n"); 
    fprintf(f,"*** Power packet received on local time %s ***\n", fecha);

    fprintf(f,"spa         : %4d mW \n", packet->spa << 1);
    fprintf(f,"spb         : %4d mW \n", packet->spb << 1);
    fprintf(f,"spc         : %4d mW \n", packet->spc << 1);
    fprintf(f,"spd         : %4d mW \n\n", packet->spd << 1);
//  fprintf(f,"vpe         : ----\n");
//  fprintf(f,"vpf         : ----\n\n");

    // 32 bits o desborda con las multiplicacion
    
    uint32_t vbus1 = (packet->vbusadc_vbatadc_hi >> 4);
             vbus1 = vbus1 * 1400;
	     vbus1 = vbus1 / 1000;

    uint16_t vbus2 = (packet->vcpuadc_lo_vbus2 & 0x0fff);
             vbus2 = vbus2 * 4;

    uint16_t vbus3 = (packet->vbus3_vbat2_hi >> 4);
    	     vbus3 = vbus3 * 4;

    fprintf(f,"vbus1       : %4d mV bus voltage read in CPU.ADC\n"  , vbus1);  
    fprintf(f,"vbus2       : %4d mV bus voltage read in EPS.I2C\n"  , vbus2);
    fprintf(f,"vbus3       : %4d mV bus voltage read in CPU.I2C\n\n", vbus3); 

    uint32_t vbat1 = (packet->vbusadc_vbatadc_hi << 8) & 0x0f00;
	     vbat1 = vbat1 | ((packet->vbatadc_lo_vcpuadc_hi >> 8) & 0x00ff);
    	     vbat1 = vbat1 * 1400;
	     vbat1 = vbat1 / 1000;

    fprintf(f,"vbat1       : %4d mV bat voltage read in EPS.ADC\n"  ,vbat1); 

    uint16_t vbat2 = (packet->vbus3_vbat2_hi << 8) & 0x0f00;
             vbat2 = (vbat2 | (packet->vbat2_lo_ibat_hi >> 8));
             vbat2 = vbat2 *4;  

    if (vbat2 < 1000) fprintf(f,"vbat2       :     \n\n"); 
    	else fprintf(f,"vbat2       : %4d mV bat voltage read in EPS.I2C\n\n", vbat2);

    uint16_t ibat = packet->vbat2_lo_ibat_hi  << 8;
             ibat = ibat | (packet->ibat_lo_icpu_hi >> 8);

    if (ibat & 0x0800) ibat = ibat |0x0f000;

    int16_t ibats = (int16_t) ibat;

    float ii;
    fprintf(f,"vbus1-vbat1 : %4d mV \n", vbus1-vbat1);
    ii =vbus1;
    ii-=vbat1; 
    ii/=0.310;
    //fprintf(f,"i@0R31      :%4.1f mA, orientative, low precission\n",ii);
    fprintf(f,"vbus3-vbus2 : %4d mV \n\n",vbus3-vbus2);
    ii =vbus3;
    ii-=vbus2; 
    ii/=0.280;
    //fprintf(f,"i@0R28      :%4.1f mA, orientativo, baja precision\n\n",ii);
    
    uint32_t vcpu_temp = (packet->vbatadc_lo_vcpuadc_hi << 4) & 0x0ff0;
             vcpu_temp = vcpu_temp | (packet->vcpuadc_lo_vbus2 >> 12);
    uint32_t vcpu = 1210*4096/vcpu_temp;

    uint16_t icpu = (packet->ibat_lo_icpu_hi << 4) & 0x0ff0;
             icpu = icpu | (packet->icpu_lo_ipl >> 12);

    // la cpu sale siempre negativa, chip al reves, multiplicamos por -1
    int16_t icpus = (int16_t) icpu;

    // si bit 12 esta a 1 ponemos el resto tambien para signo ok
    if (icpus & 0x800) icpus = (icpus | 0xf000)* -1; 

    uint16_t ipl = (packet->icpu_lo_ipl & 0x0fff);

    int16_t ipls = (int16_t) ipl;
    
    if (ipls & 0x0800) ipls = (ipls | 0xf000);

    fprintf(f,"vcpu        : %4d mV \n\n", vcpu); 
    fprintf(f,"icpu        : %4d mA @DCDCinput\n",  icpus);

    int32_t iii;
    iii =(icpus*vbus3);
    iii/=vcpu;
    fprintf(f,"icpu        : %4d mA @DCDCoutput (estimation)\n", iii);

    fprintf(f,"ipl         : %4d mA \n",   ipls);
    if (ibats == 0) fprintf(f,"ibat        : %4d mA\n\n", ibats);
    else
    if (ibats > 0) fprintf(f,"ibat        : %4d mA (Current flowing out from the battery)\n\n", ibats); // 8 y 8
		else fprintf(f,"ibat        : %4d mA (Current flowing into the battery)\n\n", ibats);

    fprintf(f,"pwrdul1     : %4d \n",  packet->powerdul*16);
    fprintf(f,"pwrdul4     : %4d \n",  packet->powerdul455*16);


///////////////////////////////////////////


}

#include <stdlib.h>

void visualiza_temppacket(temp_packet * packet) {

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char fecha[64];

    sprintf(fecha, "%d%02d%02d-%02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

    fprintf(f,"\n\n");
    fprintf(f,"*** Temp packet received on local time %s ***\n", fecha);

    if (packet->tpa   == 255) fprintf(f,"tpa         : \n"); else fprintf(f,"tpa         : %+5.1f degC temperature in SPA.I2C\n", ((float)packet->tpa/2)-40.0);
    if (packet->tpb   == 255) fprintf(f,"tpb         : \n"); else fprintf(f,"tpb         : %+5.1f degC temperature in SPB.I2C\n", ((float)packet->tpb/2)-40.0);
    if (packet->tpc   == 255) fprintf(f,"tpc         : \n"); else fprintf(f,"tpc         : %+5.1f degC temperature in SPC.I2C\n", ((float)packet->tpc/2)-40.0);
    if (packet->tpd   == 255) fprintf(f,"tpd         : \n"); else fprintf(f,"tpd         : %+5.1f degC temperature in SPD.I2C\n", ((float)packet->tpd/2)-40.0);
    if (packet->teps  == 255) fprintf(f,"teps        : \n"); else fprintf(f,"teps        : %+5.1f degC temperature in EPS.I2C\n", ((float)packet->teps/2)-40.0);
    if (packet->ttx   == 255) fprintf(f,"ttx         : \n"); else fprintf(f,"ttx         : %+5.1f degC temperature in  TX.I2C\n", ((float)packet->ttx/2)-40.0);
    if (packet->ttx2  == 255) fprintf(f,"ttx2        : \n"); else fprintf(f,"ttx2        : %+5.1f degC temperature in  TX.NTC\n", ((float)packet->ttx2/2)-40.0);
    if (packet->trx   == 255) fprintf(f,"trx         : \n"); else fprintf(f,"trx         : %+5.1f degC temperature in  RX.NTC\n", ((float)packet->trx/2)-40.0);
    if (packet->tcpu  == 255) fprintf(f,"tcpu        : \n"); else fprintf(f,"tcpu        : %+5.1f degC temperature in CPU.ADC\n", ((float)packet->tcpu/2)-40.0);

    //fprintf(f,"checksum    : %04X\n", packet->checksum);

}



typedef enum reset_cause_e
    {
        RESET_CAUSE_UNKNOWN = 0,
        RESET_CAUSE_LOW_POWER_RESET,
        RESET_CAUSE_WINDOW_WATCHDOG_RESET,
        RESET_CAUSE_INDEPENDENT_WATCHDOG_RESET,
        RESET_CAUSE_SOFTWARE_RESET,
        RESET_CAUSE_POWER_ON_POWER_DOWN_RESET,
        RESET_CAUSE_EXTERNAL_RESET_PIN_RESET,
        RESET_CAUSE_BROWNOUT_RESET,
		
} reset_cause_t;


const char * reset_cause_get_name(reset_cause_t reset_cause) {

        const char * reset_cause_name = "TBD";

        switch (reset_cause)
        {
            case RESET_CAUSE_UNKNOWN:
                reset_cause_name = "Unknown";
                break;
            case RESET_CAUSE_LOW_POWER_RESET:
                reset_cause_name = "Low Power Reset";
                break;
            case RESET_CAUSE_WINDOW_WATCHDOG_RESET:
                reset_cause_name = "Window Watchdog reset";
                break;
            case RESET_CAUSE_INDEPENDENT_WATCHDOG_RESET:
                reset_cause_name = "Independent watchdog reset";
                break;
            case RESET_CAUSE_SOFTWARE_RESET:
                reset_cause_name = "Software reset";
                break;
            case RESET_CAUSE_POWER_ON_POWER_DOWN_RESET:
                reset_cause_name = "Power-on reset (POR) / Power-down reset (PDR)";
                break;
            case RESET_CAUSE_EXTERNAL_RESET_PIN_RESET:
                reset_cause_name = "External reset pin reset";
                break;
            case RESET_CAUSE_BROWNOUT_RESET:
                reset_cause_name = "Brownout reset (BOR)";
                break;
        }

        return reset_cause_name;
}


const char * battery_status(uint8_t status) {


   switch(status) {

	case 0:
		return "Fully charged (4.2V)";
	case 1:
		return "Charged (Between 3.550V and 4.2V)";
	case 2:
		return "Half charged (Between 3300V and 3550V) - Limited transmissions";
	case 3:
		return "Low charge (Between 3200V and 3300V) - Limited transmissions, antenna deploying not allowed";
	case 4:
                return "Very Low charge (Between 2500V and 3200V) - Limited transmissions, antenna deploying not allowed";
	case 5:
                return "Battery damaged (Below 2500V) - Battery disconnected";


	default:
		return "Unknown";

   }
	

}


const char * transponder_mode(uint8_t mode) {

        switch(mode) {


                case 0:
                        return "Disabled";
                case 1:
                        return "Enabled in FM mode";
                case 2:
                        return "Enabled in FSK regenerative mode";
                deafult:
                        return "Unknown";

        }

}



void visualiza_statuspacket(status_packet * packet) {

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char fecha[64];

    sprintf(fecha, "%d%02d%02d-%02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

    fprintf(f,"\n\n");
    fprintf(f,"*** Status packet received on local time %s ***\n", fecha);

    uint16_t days = 0, hours = 0, minutes = 0, seconds = 0;
    uint16_t daysup = 0, hoursup = 0, minutesup = 0;

    days      = (packet->sclock / 86400); // segundos que dan para dias enteros
    hours     = (packet->sclock % 86400)/3600; // todo lo que no de para un dia dividido entre 3600 seg que son una hora
    minutes   = (packet->sclock % 3600)/60; // todo lo que no de para una hora dividido entre 60
    seconds   = (packet->sclock % 60);

    uint32_t segundos_up  = packet->uptime*60;

    daysup    = (segundos_up / 86400);
    hoursup   = (segundos_up % 86400)/3600;
    minutesup = (segundos_up % 3600)/60;

    fprintf(f,"sclock              : %10d seconds (satellite has been active for %d days and %02d:%02d:%02d hh:mm:ss)\n",packet->sclock, days, hours, minutes, seconds);
    fprintf(f,"uptime              : %10d minutes since the last CPU reset (%d days and %02d hours %02d mins)\n",packet->uptime, daysup, hoursup, minutesup);
    fprintf(f,"nrun                : %10d times satellite CPU was started\n",packet->nrun);
    fprintf(f,"npayload            : %10d times payload was activated\n",packet->npayload);

    if (packet->nwire > 25)  fprintf(f,"nwire               :         25 or more times antenna deployment was tried or disabled by command\n");
	else fprintf(f,"nwire               : %10d times antenna deployment was tried\n", packet->nwire);

    if (packet->nwire <= 25) fprintf(f,"ntrans              :          No info as antenna deployment attemps are still ongoing\n");
	                else fprintf(f,"ntrans              :	       %d times transponder was activated\n", packet->nwire-26); // 26 == 0
    fprintf(f,"nbusdrops           : %10d\n",packet->nbusdrops_lastreset >> 4);
    fprintf(f,"last_reset_cause    : %10d %s\n",packet->nbusdrops_lastreset & 0x0F, reset_cause_get_name(packet->nbusdrops_lastreset & 0x0F));
    fprintf(f,"bate (battery)      : %10X %s\n",packet->bate_mote >> 4, battery_status(packet->bate_mote >> 4));
    fprintf(f,"mote (transponder)  : %10X %s\n",packet->bate_mote & 0x0f, transponder_mode(packet->bate_mote & 0x0f));

    if (packet->nTasksNotExecuted == 0)
    fprintf(f,"nTasksNotExecuted   :          OK\n");
                else
    fprintf(f,"nTasksNotExecuted   : %10d Some tasks missed their execution time\n",packet->nTasksNotExecuted);

    if (packet->nExtEepromErrors == 0)
    fprintf(f,"nExtEepromErrors    :          OK\n");
                else
    fprintf(f,"nExtEepromErrors    : %10d EEPROM Fail\n",packet->nExtEepromErrors);

    if (packet->antennaDeployed == 0)
    fprintf(f,"antennaDeployed     :          KO (Antenna not yet deployed)\n");
		else
    	if (packet->antennaDeployed == 2)
    		fprintf(f,"antennaDeployed     :          UNKNOWN\n");
    	else if (packet->antennaDeployed == 1)
    fprintf(f,"antennaDeployed     :          OK (Antenna has been deployed)\n");

    if (packet->nTasksNotExecuted == 0 && packet->failed_task_id == 0) fprintf(f,"last_failed_task_id : \n");
    	else if (packet->nTasksNotExecuted == 0 && packet->failed_task_id == 255) fprintf(f,"last_failed_task_id :          Power amplifier disabled or does not respond\n");
    		else fprintf(f,"last_failed_task_id :          Q%dT%d\n", packet->failed_task_id >> 6, packet->failed_task_id & 0b00111111);

    if (packet->orbperiod == 255) fprintf(f,"messaging enabled   :          No\n");
    	else fprintf(f,"messaging enabled   :          Yes (%d messages stored)\n", packet->orbperiod);

    fprintf(f,"strfwd0 (id)        : %10X (%d)\n",packet->strfwd0, packet->strfwd0);
    fprintf(f,"strfwd1 (key)       : %10X (%d)\n",packet->strfwd1, packet->strfwd1);
    fprintf(f,"strfwd2 (value)     : %10X (%d)\n",packet->strfwd2, packet->strfwd2);
    fprintf(f,"strfwd3 (num_tcmds) : %10X (%d)\n",packet->strfwd3, packet->strfwd3);

    //fprintf(f,"checksum    : %04X\n", packet->checksum);

}


void visualiza_powerstatspacket(power_stats_packet * packet) {

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char fecha[64];

    sprintf(fecha, "%d%02d%02d-%02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

    fprintf(f,"\n\n");
    fprintf(f,"*** Power stats packet received on local time %s ***\n", fecha);

	int8_t minicpus;
	int8_t maxicpus;
	int8_t medicpus;
	
	int8_t temp;

	maxicpus = packet->maxicpu;
	minicpus = packet->minicpu;

   	fprintf(f,"minvbus1       : %4d mV ADC\n", 1400*(packet->minvbusadc_vbatadc_hi >> 4)/1000);
    	fprintf(f,"minvbat1       : %4d mV ADC\n", 1400*(((packet->minvbusadc_vbatadc_hi << 8) & 0x0f00) | ((packet->minvbatadc_lo_vcpuadc_hi >> 8) & 0x00ff))/1000);
    	fprintf(f,"minvcpu        : %4d mV ADC\n", 1210*4096/(((packet->minvbatadc_lo_vcpuadc_hi << 4) & 0x0ff0) | ((packet->minvcpuadc_lo_free > 4) & 0x000f)));
        fprintf(f,"minvbus2       : %4d mV I2C\n",packet->minvbus2*16*4);
	fprintf(f,"minvbus3       : %4d mV I2C\n",packet->minvbus3*16*4);
	fprintf(f,"minvbat2       : %4d mV I2C (only test)\n",packet->minvbat2*16*4);
	fprintf(f,"minibat        : %4d mA I2C (Max current flowing into the battery)\n",-1*(packet->minibat));
	fprintf(f,"minicpu        : %4d mA I2C\n",minicpus);
	fprintf(f,"minipl         : %4d mA I2C (only test)\n",packet->minipl);
	fprintf(f,"minpowerdul    : %4d dBm\n",packet->minpowerdul);
	fprintf(f,"minpowerdul455 : \n");
	fprintf(f,"minvdac        : \n");	
        fprintf(f,"maxvbus1       : %4d mV ADC\n", 1400*(packet->maxvbusadc_vbatadc_hi >> 4)/1000);
        fprintf(f,"maxvbat1       : %4d mV ADC\n", 1400*(((packet->maxvbusadc_vbatadc_hi << 8) & 0x0f00) | ((packet->maxvbatadc_lo_vcpuadc_hi >> 8) & 0x00ff))/1000);
        fprintf(f,"maxvcpu        : %4d mV ADC\n", 1210*4096/(((packet->maxvbatadc_lo_vcpuadc_hi << 4) & 0x0ff0) | ((packet->maxvcpuadc_lo_free > 4) & 0x000f)));
        fprintf(f,"maxvbus2       : %4d mV I2C\n",packet->maxvbus2*16*4);
	fprintf(f,"maxvbus3       : %4d mV I2C\n",packet->maxvbus3*16*4);	
	fprintf(f,"maxvbat2       : %4d mV I2C (only test)\n",packet->maxvbat2*16*4);		
	fprintf(f,"maxibat        : %4d mA I2C (Max current flowing out from the battery)\n",packet->maxibat);
	fprintf(f,"maxicpu        : %4d mA I2C\n",maxicpus);	
	fprintf(f,"maxipl         : %4d mA I2C (only test)\n",packet->maxipl << 2);		
	fprintf(f,"maxpowerdul    : %4d dBm\n",packet->maxpowerdul);	
	fprintf(f,"maxpowerdul455 : \n");	
	fprintf(f,"maxvdac        : \n");		
        fprintf(f,"medvbus1       : \n");
        fprintf(f,"medvbat1       : \n");
        fprintf(f,"medvcpu        : \n");
	fprintf(f,"\n");
        fprintf(f,"ibat_rx_charging               : %4d mA I2C\n",packet->ibat_rx_charging);
	fprintf(f,"ibat_rx_discharging            : %4d mA I2C\n",packet->ibat_rx_discharging);	
	fprintf(f,"ibat_tx_low_power_charging     : %4d mA I2C\n",packet->ibat_tx_low_power_charging);			
	fprintf(f,"ibat_tx_low_power_discharging  : %4d mA I2C\n",packet->ibat_tx_low_power_discharging);	
	fprintf(f,"ibat_tx_high_power_charging    : %4d mA I2C\n",packet->ibat_tx_high_power_charging);			
        fprintf(f,"ibat_tx_high_power_discharging : %4d mA I2C\n",packet->ibat_tx_high_power_discharging);
        fprintf(f,"\n");
	fprintf(f,"medpowerdul    : %4d dBm\n",packet->medpowerdul);	
	fprintf(f,"medpowerdul455 : \n");	
	fprintf(f,"medvdac        : \n");		
  
}




void visualiza_tempstatspacket(temp_stats_packet * packet) {

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char fecha[64];

    sprintf(fecha, "%d%02d%02d-%02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

    fprintf(f,"\n\n");
    fprintf(f,"*** Temp stats packet received on local time %s ***\n", fecha);

	if (packet->mintpa   == 255) fprintf(f,"mintpa         : \n"); else fprintf(f,"mintpa         : %+5.1f C MCP Temperature outside panel A\n", ((float)packet->mintpa/2)-40.0);
        if (packet->mintpb   == 255) fprintf(f,"mintpb         : \n"); else fprintf(f,"mintpb         : %+5.1f C MCP Temperature outside panel B\n", ((float)packet->mintpb/2)-40.0);
        if (packet->mintpc   == 255) fprintf(f,"mintpc         : \n"); else fprintf(f,"mintpc         : %+5.1f C MCP Temperature outside panel C\n", ((float)packet->mintpc/2)-40.0);
        if (packet->mintpd   == 255) fprintf(f,"mintpd         : \n"); else fprintf(f,"mintpd         : %+5.1f C MCP Temperature outside panel D\n", ((float)packet->mintpd/2)-40.0);
        if (packet->mintpe   == 255) fprintf(f,"mintpe         : \n"); else fprintf(f,"mintpe         : \n");
        if (packet->minteps  == 255) fprintf(f,"minteps        : \n"); else fprintf(f,"minteps        : %+5.1f C TMP100 Temperature EPS\n", ((float)packet->minteps/2)-40.0);
        if (packet->minttx   == 255) fprintf(f,"minttx         : \n"); else fprintf(f,"minttx         : %+5.1f C TMP100 Temperature TX\n", ((float)packet->minttx/2)-40.0);
        if (packet->minttx2  == 255) fprintf(f,"minttx2        : \n"); else fprintf(f,"minttx2        : %+5.1f C ADC Temperature TX\n", ((float)packet->minttx2/2)-40.0);
        if (packet->mintrx   == 255) fprintf(f,"mintrx         : \n"); else fprintf(f,"mintrx         : %+5.1f C ADC Temperature RX\n", ((float)packet->mintrx/2)-40.0);
        if (packet->mintcpu  == 255) fprintf(f,"mintcpu        : \n"); else fprintf(f,"mintcpu        : %+5.1f C INT Temperature CPU\n", ((float)packet->mintcpu/2)-40.0);
        if (packet->maxtpa   == 255) fprintf(f,"maxtpa         : \n"); else fprintf(f,"maxtpa         : %+5.1f C MCP Temperature outside panel A\n", ((float)packet->maxtpa/2)-40.0);
        if (packet->maxtpb   == 255) fprintf(f,"maxtpb         : \n"); else fprintf(f,"maxtpb         : %+5.1f C MCP Temperature outside panel B\n", ((float)packet->maxtpb/2)-40.0);
        if (packet->maxtpc   == 255) fprintf(f,"maxtpc         : \n"); else fprintf(f,"maxtpc         : %+5.1f C MCP Temperature outside panel C\n", ((float)packet->maxtpc/2)-40.0);
        if (packet->maxtpd   == 255) fprintf(f,"maxtpd         : \n"); else fprintf(f,"maxtpd         : %+5.1f C MCP Temperature outside panel D\n", ((float)packet->maxtpd/2)-40.0);
        if (packet->maxtpe   == 255) fprintf(f,"maxtpe         : \n"); else fprintf(f,"maxtpe         : \n");
        if (packet->maxteps  == 255) fprintf(f,"maxteps        : \n"); else fprintf(f,"maxteps        : %+5.1f C TMP100 Temperature EPS\n", ((float)packet->maxteps/2)-40.0);
        if (packet->maxttx   == 255) fprintf(f,"maxttx         : \n"); else fprintf(f,"maxttx         : %+5.1f C TMP100 Temperature TX\n", ((float)packet->maxttx/2)-40.0);
        if (packet->maxttx2  == 255) fprintf(f,"maxttx2        : \n"); else fprintf(f,"maxttx2        : %+5.1f C ADC Temperature TX\n", ((float)packet->maxttx2/2)-40.0);
        if (packet->maxtrx   == 255) fprintf(f,"maxtrx         : \n"); else fprintf(f,"maxtrx         : %+5.1f C ADC Temperature RX\n", ((float)packet->maxtrx/2)-40.0);
        if (packet->maxtcpu  == 255) fprintf(f,"maxtcpu        : \n"); else fprintf(f,"maxtcpu        : %+5.1f C INT Temperature CPU\n", ((float)packet->maxtcpu/2)-40.0);
        if (packet->medtpa   == 255) fprintf(f,"medtpa         : \n"); else fprintf(f,"medtpa         : %+5.1f C MCP Temperature outside panel A\n", ((float)packet->medtpa/2)-40.0);
      	if (packet->medtpb   == 255) fprintf(f,"medtpb         : \n"); else fprintf(f,"medtpb         : %+5.1f C MCP Temperature outside panel B\n", ((float)packet->medtpb/2)-40.0);
        if (packet->medtpc   == 255) fprintf(f,"medtpc         : \n"); else fprintf(f,"medtpc         : %+5.1f C MCP Temperature outside panel C\n", ((float)packet->medtpc/2)-40.0);
        if (packet->medtpd   == 255) fprintf(f,"medtpd         : \n"); else fprintf(f,"medtpd         : %+5.1f C MCP Temperature outside panel D\n", ((float)packet->medtpd/2)-40.0);
        if (packet->medtpe   == 255) fprintf(f,"medtpe         : \n"); else fprintf(f,"medtpe         : \n");
        if (packet->medteps  == 255) fprintf(f,"medteps        : \n"); else fprintf(f,"medteps        : %+5.1f C TMP100 Temperature EPS\n", ((float)packet->medteps/2)-40.0);
        if (packet->medttx   == 255) fprintf(f,"medttx         : \n"); else fprintf(f,"medttx         : %+5.1f C TMP100 Temperature TX\n", ((float)packet->medttx/2)-40.0);
        if (packet->medttx2  == 255) fprintf(f,"medttx2        : \n"); else fprintf(f,"medttx2        : %+5.1f C ADC Temperature TX\n", ((float)packet->medttx2/2)-40.0);
        if (packet->medtrx   == 255) fprintf(f,"medtrx         : \n"); else fprintf(f,"medtrx         : %+5.1f C ADC Temperature RX\n", ((float)packet->medtrx/2)-40.0);
        if (packet->medtcpu  == 255) fprintf(f,"medtcpu        : \n"); else fprintf(f,"medtcpu        : %+5.1f C INT Temperature CPU\n", ((float)packet->medtcpu/2)-40.0);


}

/*
*** Sunvector packet received on local time 20220720-20:41:25 ***
       TD      SPA      SPB      SPC      SPD      90A      90D 
        0        0        0        0        0        0        0 
        0        0        0        0        0        0        0 
        0        0        0        0        0        0        0 
        0        0        0        0        0        0        0 
        0        0        0        0        0        0        0 
        0        0        0        0        0        0        0 

sensor peak_level err_counter
   SPA          0          0
   SPB          0          0
   SPC          0          0
   SPD          0          0
   90A          0          0
   90D          0          0


*** Sunvector packet received on local time 20220720-20:41:25 ***
       TD      SPA      SPB      SPC      SPD      90A      90D 
===============================================================
        0        0        0        0        0        0        0 
        0        0        0        0        0        0        0 
        0        0        0        0        0        0        0 
        0        0        0        0        0        0        0 
        0        0        0        0        0        0        0 
        0        0        0        0        0        0        0 

sensor peak_level err_counter
   SPA          0          0
   SPB          0          0
   SPC          0          0
   SPD          0          0
   90A          0          0
   90D          0          0


*/

void visualiza_sunvectorpacket(SUNVECTOR * SunVector) {


    const char * name2[]={"SPA","SPB","SPC","SPD","SP1","SP2","SP3","SP4"};

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char fecha[64];
   
    sprintf(fecha, "%d%02d%02d-%02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    fprintf(f,"\n\n");
    fflush(stdout);
    fprintf(f,"*** Sunvector packet received on local time %s ***\n", fecha);
    fflush(stdout);

    int ns,nd;
    fprintf(f,"       TD ");for(nd=0;nd!=ND;nd++) fprintf(f,"%8s ",name2[nd]); fflush(stdout);

    fprintf(f,"\n=================================================================================");
    for(ns=0;ns!=NS;ns++)
    	{
	fprintf(f,"\n%9d ",SunVector->td[ns]);
	for(nd=0;nd!=ND;nd++) fprintf(f,"%8d ",SunVector->v[nd][ns]);
	}
    fprintf(f,"\n%9s ","PWR");for(nd=0;nd!=ND;nd++)fprintf(f,"%8d ",SunVector->p[nd]);
    fprintf(f,"\n%9s ","ERR");for(nd=0;nd!=ND;nd++)fprintf(f,"%8d ",SunVector->err[nd]);
    fprintf(f,"\n");

}


void visualiza_radiopacket(radiometer_packet * packet) 
{
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char fecha[64];
    sprintf(fecha, "%d%02d%02d-%02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    fprintf(f,"\n\n");
    fprintf(f,"*** Radiometer packet received on local time %s ***\n", fecha);
    uint16_t days = 0, hours = 0, minutes = 0, seconds = 0;

    days    = (packet->clock % (86400*30))/86400;
    hours   = (packet->clock % 86400)/3600;
    minutes = (packet->clock % 3600)/60;
    seconds = (packet->clock % 60);

    fprintf(f,"sclock                 : %d seconds (%d days and %02d:%02d:%02d hh:mm:ss)\n",packet->clock, days, hours, minutes, seconds);
    for (uint8_t i = 0; i < MAX_RADIOMETER_SAMPLES; i++)
	fprintf(f,"Sample %03d : %4d\n", i, packet->radiometer_sample[i]);

}


void visualiza_deploypacket(TLMBW * tlmbw) {

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char fecha[64];

    sprintf(fecha, "%d%02d%02d-%02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

    fprintf(f,"\n\n");
    fprintf(f,"*** Antenna deployment packet received on local time %s ***\n", fecha);

    fprintf(f, "\n");
    fprintf(f, "estado pulsador inicio:fin:ahora           : %d:%d:%d\n",tlmbw->state_begin,tlmbw->state_end,tlmbw->state_now);
    fprintf(f, "tension bateria en circuito abierto Voc/mV : %d\n",tlmbw->v1oc);
    fprintf(f, "caida de tension deltaV/mV                 : %u\n",tlmbw->v1);
    fprintf(f, "corriente quemado Ibr/mA                   : %u\n",tlmbw->i1);
    fprintf(f, "corriente quemado Ibr,pk/mA                : %u\n",tlmbw->i1pk);
    fprintf(f, "resistencia interna bateria Rbat/mohm      : %u\n",tlmbw->r1);
    fprintf(f, "tiempo quemado observado t/s               : %d\n",tlmbw->td);

}



void visualiza_ine(INE * ine) {

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char fecha[64];

    sprintf(fecha, "%d%02d%02d-%02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

    fprintf(f,"\n\n");
    fprintf(f,"*** Extended power (ine) packet received on local time %s ***\n", fecha);

    for(int n=0; n!= MAXINE;n++) {
	fprintf(f, "%2d %4s | VI %5d %5d | Pmean %5d | VIPpeak %5d %5d %5d \n", n, name[n], ine[n].v,  ine[n].i,  ine[n].p, ine[n].vp,  ine[n].ip,  ine[n].pp );
	}


}



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

// funciones para colores

void red_color   (void) { printf("\033[1;31m"); }
void yellow_color(void) { printf("\033[1;33m"); }
void reset_color (void) { printf("\033[0m");    }

void mostrar_tablero(uint8_t tablero_nodo [NUM_FILAS][NUM_COLUMNAS]) {

        uint8_t toca_blanco = 1;

        for (uint8_t fila = 0; fila < NUM_FILAS; fila++) {

                toca_blanco = !toca_blanco;

                for (uint8_t columna = 0; columna < NUM_COLUMNAS; columna++) {

                        toca_blanco = !toca_blanco;

                        switch(tablero_nodo[fila][columna]) {

                                case VACIO:
                                        if (toca_blanco) yellow_color(); else red_color();
                                        fprintf(f,".");
                                        reset_color();
                                break;

                                case TORRE_NEGRA:
                                        red_color();
                                        fprintf(f,"\u265C");
                                        reset_color();
                                break;

                                case CABALLO_NEGRO:
                                        red_color();
                                        fprintf(f,"\u265E");
                                        reset_color();
                                break;

                                case ALFIN_NEGRO:
                     red_color();
                     fprintf(f,"\u265D");
                     reset_color();
                                break;

                                case DAMA_NEGRA:
                     red_color();
                     fprintf(f,"\u265B");
                     reset_color();
                                break;

                                case REY_NEGRO:
                     red_color();
                     fprintf(f,"\u265A");
                     reset_color();
                                break;

                                case PEON_NEGRO:
                     red_color();
                     fprintf(f,"\u265F");
                     reset_color();
                                break;

                case TORRE_BLANCA:
                     yellow_color();
                     fprintf(f,"\u2656");
                     reset_color();
                break;

                case CABALLO_BLANCO:
                     yellow_color();
                     fprintf(f,"\u2658");
                     reset_color();
                break;

                case ALFIN_BLANCO:
                     yellow_color();
                     fprintf(f,"\u2657");
                     reset_color();
                break;

                case DAMA_BLANCA:
                     yellow_color();
                     fprintf(f,"\u2655");
                     reset_color();
                break;

                case REY_BLANCO:
                        yellow_color();
                    fprintf(f,"\u2654");
                    reset_color();
                break;

                case PEON_BLANCO:
                        yellow_color();
                    fprintf(f,"\u2659");
                    reset_color();
                break;

                        }

                }

		fprintf(f, "\n");

        }

}


//alonso, genero salida hacia fichero
void visualiza_chesspacket(chess_board_packet * packet) {

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    char fecha[64];
    sprintf(fecha, "%d%02d%02d-%02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

    char callsign[7];
    memset(callsign, 0, sizeof(callsign));
    memcpy(callsign, packet->callsign, 6);

    fprintf(f,"\n\n");
    fprintf(f,"*** Chess packet received on local time %s ***\n", fecha);
    fprintf(f,"last callsign : %s (Last callsign who sent a valid move)\n", callsign);

    if (packet->player_color == 0) fprintf(f,"player_color  : %02d (HAM community plays with white) \n", packet->player_color);
    else if (packet->player_color == 1) fprintf(f,"player_color  : %02d (HAM community plays with black) \n", packet->player_color);
    	else  fprintf(f,"player_color  : %02d (Something is wrong) \n", packet->player_color);

    char origen[2];
    char destino[2];

    origen[0]  = (packet->last_move >> 12) + 'a';
    origen[1]  = (packet->last_move >> 8 & 0x000f) + '0';
    destino[0] = (packet->last_move >> 4 & 0x000f) + 'a';
    destino[1] = (packet->last_move & 0x00f) + '0';

    fprintf(f,"last_move     : %04X (%c%c%c%c)\n", packet->last_move, origen[0], origen[1], destino[0], destino[1]);

    if (packet->game_status == 0) fprintf(f,"game_status   : %02X (Waiting for game to start)\n", packet->game_status);
    	else if (packet->game_status == 1) fprintf(f,"game_status   : %02X (Waiting for move from player)\n", packet->game_status);
		else  if (packet->game_status == 2) fprintf(f,"game_status   : %02X (Satellite is thinking...)\n", packet->game_status);
		  else if (packet->game_status == 3) fprintf(f,"game_status   : %02X (Last move was invalid - waiting for move from player)\n", packet->game_status);
			else fprintf(f,"game_status   : %02X (Error)\n", packet->game_status);

/*			
    for (uint8_t i = 0; i < 32; i++) 
    	fprintf(f,"board[%02d]     : %02X\n", i, packet->tablero[i]);*/

    uint8_t tablero[NUM_FILAS][NUM_COLUMNAS];

    // convertir a formato tablero para poder mostrarlo
    for (uint8_t i = 0; i < NUM_FILAS; i++) {
	for (uint8_t j = 0; j < NUM_COLUMNAS; j=j+2) {
           uint8_t valor =  packet->tablero[i*(NUM_FILAS/2)+(j/2)];
	   tablero[i][j]   = valor >> 4;
	   tablero[i][j+1] = valor & 0x0f;
	}
    }    

    fprintf(f,"\nCurrent chess board:\n");
    mostrar_tablero(tablero);

}

void visualiza_efemeridespacket(_frm * ef) {

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char fecha[64];

    time_t tunix = ef->utc;
    struct tm *timeinfo = gmtime(&tunix);

    int year = timeinfo->tm_year + 1900;
    int month = timeinfo->tm_mon + 1;
    int day = timeinfo->tm_mday;
    int hour = timeinfo->tm_hour;
    int minute = timeinfo->tm_min;
    int second = timeinfo->tm_sec;

    char fechahora[64];

    time_t tunix_tle = ef->sat.tle.epoch;
    struct tm *timeinfo_tle = gmtime(&tunix_tle);

    int year_tle = timeinfo_tle->tm_year + 1900;
    int month_tle = timeinfo_tle->tm_mon + 1;
    int day_tle = timeinfo_tle->tm_mday;
    int hour_tle = timeinfo_tle->tm_hour;
    int minute_tle = timeinfo_tle->tm_min;
    int second_tle = timeinfo_tle->tm_sec;

    char fechahora_tle[64];

    sprintf(fechahora, "%04d-%02d-%02d %02d:%02d:%02d", year, month, day, hour, minute, second);
    sprintf(fechahora_tle, "%04d-%02d-%02d %02d:%02d:%02d", year_tle, month_tle, day_tle, hour_tle, minute_tle, second_tle);

    sprintf(fecha, "%d%02d%02d-%02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

    fprintf(f,"\n\n");
    fprintf(f,"*** Ephemeris packet received on local time %s ***\n", fecha);

    fprintf(f,"UTC time    : %d seconds (%s) (on board) YYYY-MM-DD HH24:MI:SS\n", ef->utc, fechahora);
    fprintf(f,"lat         : %.2f degrees\n", (float)ef->lat/100);
    fprintf(f,"lon         : %.2f degrees\n", (float)ef->lon/100);
    fprintf(f,"alt         : %d km\n", ef->alt);
    if (ef->utc == 0 || ef->sat.tle.epoch == 0) fprintf(f,"zone        : unknown\n"); else fprintf(f,"zone        : %s\n", overflying((float)ef->lat/100, (float)ef->lon/100));
    fprintf(f,"Adr         : %d\n", ef->sat.adr);
    fprintf(f,"Ful         : %d\n", ef->sat.ful);
    fprintf(f,"Fdl         : %d\n", ef->sat.fdl);
    fprintf(f,"Epoch       : %d seconds (%s) (TLE time) YYYY-MM-DD HH24:MI:SS\n", ef->sat.tle.epoch, fechahora_tle);
    fprintf(f,"Xndt2o      : %f\n", ef->sat.tle.xndt2o);
    fprintf(f,"Xndd6o      : %f\n", ef->sat.tle.xndd6o);
    fprintf(f,"Bstar       : %f\n", ef->sat.tle.bstar);
    fprintf(f,"Xincl       : %f\n", ef->sat.tle.xincl);
    fprintf(f,"Xnodeo      : %f\n", ef->sat.tle.xnodeo);
    fprintf(f,"Eo          : %f\n", ef->sat.tle.eo);
    fprintf(f,"Omegao      : %f\n", ef->sat.tle.omegao);
    fprintf(f,"Xmo         : %f\n", ef->sat.tle.xmo);
    fprintf(f,"Xno         : %f\n", ef->sat.tle.xno);

}


void procesar(void) {

   uint16_t sync_buffer;
   uint8_t RX_buffer[RX_BUFFER_SIZE];
   uint8_t posible_paquete = 0;

   int num_bit_size = 0;
   uint8_t size = 0;
   uint16_t data_size = 0;

   uint8_t buffer_current_bit  = 0;
   uint16_t buffer_current_byte = 0;

   int8_t telemetry_packet = 0;
   uint16_t telemetry_size = 0;

   uint8_t type   = 0;
   uint8_t source = 0;

   power_packet powerp;
   temp_packet tempp;
   status_packet statusp;
   power_stats_packet powerstatsp;
   temp_stats_packet tempstatsp;
   chess_move_packet chessmovep;
   chess_board_packet chessp;
   INE ine[MAXINE];
   TLMBW tlmbw;
   SUNVECTOR sunvector;
   radiometer_packet radiop;
   _frm efemeridesp;

   char * desc [] = { "-", "pwr", "tmp", "status", "pwr_sts", "tmp_sts", "sun", "radiometer", "antenna", "extended power - ina219", "chess_move", "chess_board", "ephemeris" };

   time_t t = time(NULL);
   struct tm tm = *localtime(&t);
   char fecha[64];
   int estado = INICIAL;

   uint8_t current_byte = 0;

        static struct termios oldt, newt;

        tcgetattr( STDIN_FILENO, &oldt);
        newt = oldt;

        newt.c_lflag &= ~(ICANON);

	char input;

	memset(fecha, 0, sizeof(fecha));

   do {

        if (estado == INICIAL) {

                t = time(NULL);
                tm = *localtime(&t);

                sprintf(fecha, "%d%02d%02d-%02d:%02d:%02d :", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

                printf("\n");
                printf("%s Waiting for data from the modem...\n", fecha);

                estado = WAITING_FOR_SYNC;

        }

	t = time(NULL);
  	struct tm dateini = *localtime(&t);
 
	do {

	   //miro si trabajo
	   //{static long xx=0,t;if(++xx==3000){xx=0;t=time(0);fprintf(stderr,"!! TM time=%ld\n",t);}}

       	   tcsetattr( STDIN_FILENO, TCSANOW, &newt);

	   input = getchar();
	   if(input=='x')exit(0); //alonso junio23

           tcsetattr( STDIN_FILENO, TCSANOW, &oldt);

	   t = time(NULL);
	   struct tm dateend = *localtime(&t);

	   double diff_seconds = difftime(mktime(&dateend), mktime(&dateini));

	   if ((estado == READING_DATA || estado == READING_SIZE) && ((input != '0' && input != '1') || diff_seconds > 2)) {

	       	estado = INICIAL;

        	t = time(NULL);
        	tm = *localtime(&t);

                sprintf(fecha, "%d%02d%02d-%02d:%02d:%02d :", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
                printf("\n%s Signal was lost. Data discarded\n", fecha);
		fflush(stdout);

	   }


	} while (input != '0' && input != '1');

        t = time(NULL);
        tm = *localtime(&t);

	char current_bit_value = input -'0';

	current_byte = current_byte << 1;
	current_byte = current_byte | current_bit_value;

	// leer de stdint y meter en buffer

 	// vamos metiendo bits en el buffer de sync, si encontramos sync, el resto de bytes los almacenamos en buffer de procesamiento
    
	if (estado == WAITING_FOR_SYNC) {
    
        	sync_buffer = sync_buffer << 1;
        	sync_buffer = sync_buffer | current_bit_value;

   		if (sync_buffer == SYNCHRONIZED) {

        		sprintf(fecha, "%d%02d%02d-%02d:%02d:%02d :", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

			printf("%s Sync detected\n", fecha);
			fflush(stdout);
			estado = READING_SIZE;

			size = 0;
			num_bit_size = 0;

			continue;

		}
	}

	// estado READING_SIZE

	if (estado == READING_SIZE) {
	
	        size = size << 1;
                size = size | current_bit_value;

		num_bit_size++;

		if (num_bit_size != 8) continue;

                sprintf(fecha, "%d%02d%02d-%02d:%02d:%02d :", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

		type   = size >> 4;
		source = size & 0x0f;

		if (type < 13 && source == 8) {

			printf("%s packet type %d (%s)\n", fecha, type, desc[type]);
			fflush(stdout);

			telemetry_packet = type;

			telemetry_size   = telemetry_packet_size(type);

			if (type == 0) {

                        	printf("%s Size is too small. Data is discarded\n", fecha);
                                fflush(stdout);

                                estado = INICIAL;

                                continue;

			}

		} else {

			printf("%s Unknown type. Data is discarded\n", fecha);
                        fflush(stdout);

			estado = INICIAL;

			continue;

		}

		num_bit_size = 0;

		estado = READING_DATA;

		data_size = size;

		memset(RX_buffer, 0, sizeof(RX_buffer));
		RX_buffer[0] = size;

		buffer_current_byte = 1; // hemos guardado el size
		buffer_current_bit  = 0;

                sprintf(fecha, "%d%02d%02d-%02d:%02d:%02d :", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

                printf("%s Raw data         : %02X ", fecha, size);
		fflush(stdout);

		continue;
		
	} // estado reading size

	// estado READING_DATA 

	if (estado == READING_DATA) {

		static uint8_t current_bit  = 0;
		static uint8_t current_byte = 0;

		// los bits van llegando de MSB a LSB en cada byte

		RX_buffer[buffer_current_byte] = RX_buffer[buffer_current_byte] << 1;
		RX_buffer[buffer_current_byte] = RX_buffer[buffer_current_byte] | current_bit_value;

		buffer_current_bit++;

		if (buffer_current_bit != 8) continue;

                printf("%02X ", RX_buffer[buffer_current_byte]);
		fflush(stdout);

		buffer_current_bit = 0;
		buffer_current_byte++;

		if (buffer_current_byte == 260) {

			estado = INICIAL;
			continue;

		}


		// si tenemos un posible paquete de telemetria comprobar

		if (telemetry_packet && (buffer_current_byte == telemetry_size)) {

                        printf("\n");
                        printf("%s Checking telemetry packet\n", fecha);

			fflush(stdout);

			uint16_t checksum = RX_buffer[buffer_current_byte-2] << 8 | RX_buffer[buffer_current_byte-1];

			// en RX buffer tenemos desde size
			uint16_t calculated_checksum = crc16(RX_buffer, telemetry_size -2); // se quitan los de checksum

                       	sprintf(fecha, "%d%02d%02d-%02d:%02d:%02d :", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
			printf("%s Packet checksum %04X - calculated checksum %04X\n", fecha, checksum, calculated_checksum);
			fflush(stdout);

			if (checksum == calculated_checksum) {

				printf("%s Checksum matches. Valid packet\n", fecha);
				fflush(stdout);

				SelfSyncDeScrambler(RX_buffer+1, telemetry_size-3); // todo menos header y campo checksum

                                printf("%s Unscrambled data : ", fecha);
                                for (int i = 0; i < buffer_current_byte; i++) printf("%02X ", RX_buffer[i]);

 				fflush(stdout);
						
				//alonso
				char name[16*1024];
				sprintf(name,"received/%02d.tlm",type);
				f=fopen(name,"w");	
				
				if (f == NULL) {

					printf("\n\nPlease check received directory exists. Failed creating file %s\n", name);
					exit(-1);

				}

				switch(type) {

					case 1:
                                       		memset(&powerp, 0, sizeof(powerp));
                                        	memcpy((void*)&powerp + PACKET_HEADER_SIZE, (void*)RX_buffer, telemetry_size);  
						visualiza_powerpacket(&powerp);
					break;

					case 2:
                                                memset(&tempp, 0, sizeof(tempp));
                                                memcpy((void*)&tempp + PACKET_HEADER_SIZE, (void*)RX_buffer, telemetry_size);  
						visualiza_temppacket(&tempp);
					break;

                                        case 3:
                                                memset(&statusp, 0, sizeof(statusp));
                                                memcpy((void*)&statusp + PACKET_HEADER_SIZE, (void*)RX_buffer, telemetry_size);  
                                                visualiza_statuspacket(&statusp);
                                        break;

                                        case 4:
                                                 memset(&powerstatsp, 0, sizeof(powerstatsp));
                                                 memcpy((void*)&powerstatsp + PACKET_HEADER_SIZE, (void*)RX_buffer, telemetry_size);  
                                                 visualiza_powerstatspacket(&powerstatsp);
                                        break;

                                        case 5:  
                                                 memset(&tempstatsp, 0, sizeof(tempstatsp));
                                                 memcpy((void*)&tempstatsp + PACKET_HEADER_SIZE, (void*)RX_buffer, telemetry_size);  
                                                 visualiza_tempstatspacket(&tempstatsp);
                                        break;

                                        case 6: 
                                                 memset(&sunvector, 0, sizeof(sunvector));
                                                 memcpy((void*)&sunvector, (void*)RX_buffer+1, telemetry_size-3); // estas estructuras no llevan tipo ni CRC 
                                                 visualiza_sunvectorpacket(&sunvector);
                                        break;

                                        case 7:
                                                 memset(&radiop, 0, sizeof(radiop));
                                                 memcpy((void*)&radiop + PACKET_HEADER_SIZE, (void*)RX_buffer, telemetry_size);  
                                                 visualiza_radiopacket(&radiop);
                                        break;

					case 8: 
                                                 memset(&tlmbw, 0, sizeof(tlmbw));
                                                 memcpy((void*)&tlmbw, (void*)RX_buffer+1, telemetry_size-3);  // estas estructuras no llevan tipo ni CRC
                                                 visualiza_deploypacket(&tlmbw);
                                        break;

                                        case 9:  
                                                 memset(&ine, 0, sizeof(ine));
                                                 memcpy((void*)ine, (void*)RX_buffer+1, telemetry_size-3);  //  estas estructuras no llevan tipo ni CRC
                                                 visualiza_ine(ine);
                                        break;

					case 10:
                                                 memset(&chessmovep, 0, sizeof(chessmovep));
                                                 memcpy((void*)&chessmovep + PACKET_HEADER_SIZE, (void*)RX_buffer, telemetry_size);  
                                                 visualiza_chessmovepacket(&chessmovep);
					break;

					case 11:
                                                 memset(&chessp, 0, sizeof(chessp));
                                                 memcpy((void*)&chessp + PACKET_HEADER_SIZE, (void*)RX_buffer, telemetry_size);  
                                                 visualiza_chesspacket(&chessp);
					break;

					case 12:
                                                memset(&efemeridesp, 0, sizeof(efemeridesp));
                                                memcpy((void*)&efemeridesp + PACKET_HEADER_SIZE, (void*)RX_buffer, telemetry_size);  
                                                visualiza_efemeridespacket(&efemeridesp);
					break;

					default:

			
					break;

				} // switch

				//alonso, imprimo en pantalla
				fprintf(f,"%c",0);
				fclose(f);
				f=fopen(name,"r");	
				fread(name,16*1024,1,f);
				fclose(f);
				printf("%s\n",name);

				estado = INICIAL;
				continue;

			} else { // crc

                                printf("%s Not a valid telemetry packet or invalid checksum: data is discarded\n", fecha);
				fflush(stdout);

				telemetry_packet = 0;	

		 	} // if coincide checksum


		} // fin de procesamiento de telemetry packet

		if (!telemetry_packet) estado = INICIAL;

	} // reading data


    } while (1);

}


void /* Calculate if a satellite is above any zone */
trx(double lat, double lon, int latc, int lonc, int rc, int *inout)
{
    double rc_km, dlat, dlon, a_t, c_t, d_t;
    rc_km = rc * (pi / 180) * 6371; // radio del círculo en km
    dlat = (lat - latc)* (pi / 180) / 2.0;
    dlon = (lon - lonc)* (pi / 180) / 2.0;
    a_t = sin(dlat) * sin(dlat) + cos(latc * (pi / 180) ) * cos(lat * (pi / 180) ) * sin(dlon) * sin(dlon);
    c_t = 2.0 * atan2(sqrt(a_t), sqrt(1.0 - a_t));
    d_t = 6371 * c_t;

    if (d_t <= rc_km){
        *inout = 1;
    } else {
        *inout = 0;
    }
} /* trx */

char * overflying(double lat, double lon)
{
	//LAT,LONG,RADIO
	int zones[nzones][3]={{ 54,  -4,  7},  // Reino Unido 
						{ 42,  -4, 6},	// Europa (S) 
						{ 48,  16, 12},	// Europa (M) 
						{ 64,  18,  7},	// Europa (N) 						
						{ 36, 139, 11},	// Japon (N)  						
						{ 10, 115, 23},	// Asia (S)						
						{ 27,  98, 27},	// Asia (E)				
						{ 32,  61, 23},	// Asia (W)					
						{ 24,  44, 12},	// Arabia Saudi 
						{ 60, 151, 19},	// Rusia (E) 
						{ 62, 103, 19},	// Rusia (C) 
						{ 57,  53, 17},	// Rusia (W) 	  
						{-21,  80, 35},	// Oceano Indico   
						{-50,  40, 20},	// Oceano Indico 
						{-22, 134, 26},	// Oceania 
						{  0,  17, 38},	// Africa 
						{-40, -14, 27},	// Oceano Atlantico Sur 
						{-48, -39, 18},	// Oceano Atlantico Sur 
						{ 33, -37, 23},	// Oceano Atlantico Norte 
						{ 71, -37, 12},	// Groenlandia 		
						{ -8, -59, 24},	// America del Sur 
						{-40, -65, 19},	// America del Sur 
						{ 16, -88, 17},	// Centro America 
						{ 47, -97, 30},	// America del Norte 
						{ 63,-151,  8},	// America del Norte	
						{ 72, -98, 10},	// America del Norte				
						{-90,   0, 22},	// Antartida 
						{ 17,-173, 47},	// Oceano Pacifico
						{-25,-128, 47},	// Oceano Pacifico
						{-49, 142, 28},	// Oceano Pacifico
						{-38,-165, 34},	// Oceano Pacifico
						{ 90, 140, 10},	// Oceano Artico
						{ 90,   60, 8},	// Oceano Artico
						{ 90,  168, 8}};// Oceano Artico
				  
	char countries[nzones][20]={"United Kingdom","Europe","Europe","Europe","Japan","Asia","Asia","Asia",
	"Saudi Arabia","Russia","Russia","Russia","the Indian Ocean","the Indian Ocean","Oceania","Africa",
	"the Atlantic Ocean","the Atlantic Ocean","the Atlantic Ocean","Greenland","South America","South America",
	"Central America","North America","North America","North America","Antartica","the Pacific Ocean",
	"the Pacific Ocean","the Pacific Ocean","the Pacific Ocean","the Artic Ocean","the Artic Ocean","the Artic Ocean"};	

	int i; int p = 0;
    for (i = 0; i < nzones; i++) {
        int inout = -1;
        trx(lat, lon, zones[i][0], zones[i][1], zones[i][2], &inout);
        if (inout == 0)continue;
        p = 1; break;
    }
	
    static char result[128];
    memset(result, 0, sizeof(result));

	if(p==1)snprintf(result, sizeof(result), "Flying over %s", countries[i]);
	if(p==0)snprintf(result, sizeof(result), "The overflown area is not found in the database");
    return result;
}



