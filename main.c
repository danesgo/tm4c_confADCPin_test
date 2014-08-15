/*
 * main.c
 * Rodrigo Chang
 * Fecha: 13 de agosto de 2014
 *
 * Programa para inicializar el modulo ADC0 utilizando el secuenciador 3 (1 muestra)
 * para muestrear de forma periodica una señal en el pin PB5 (AIN11)
 *
 */

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/tm4c123gh6pm.h"
#include "driverlib/sysctl.h"

#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/adc.h"
#include "driverlib/interrupt.h"
//#include "driverlib/debug.h"

// Prototipos de funciones
void ADC0SS3_Init(void);
void Timer0_Init(uint32_t ciclos);
void configurarLEDs(void);

// Definiciones
#define PF2	HWREG(GPIO_PORTF_BASE + 16)

int main(void) {
	// Configurar el reloj a 40MHz
	SysCtlClockSet(SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ | SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL);

	// Configuracion de GPIO
	configurarLEDs();
	// Configurar el ADC0SS3 en PB5
	ADC0SS3_Init();
	// Configurar el timer a 1s@40MHz
	Timer0_Init(40000000);
	// Configurar interrupciones globales
	IntMasterEnable();

	while (1) {
		// Esperar las interrupciones
	}
}

/*
 * Configura el modulo ADC0 secuenciador 3 en el pin
 */
void ADC0SS3_Init(void) {
	// Configuracion de la entrada analogica
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	GPIOPinTypeADC(GPIO_PORTB_BASE, GPIO_PIN_5);

	// 1.1 Configuracion de frecuencia de muestreo
	SysCtlADCSpeedSet(SYSCTL_ADCSPEED_125KSPS);
	// 1. Configuracion de reloj al modulo ADC0
	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
	// 2. Configurar el numero de secuenciador (=3) y el trigger
	ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_TIMER, 0);
	// 3. Configurar el unico paso del secuenciador 3 para sensar el canal 11 y generar interrupcion
	ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH11 | ADC_CTL_IE | ADC_CTL_END);
	// 4. Configurar las interrupciones
	ADCIntEnable(ADC0_BASE, 3);
	IntEnable(INT_ADC0SS3);
	IntPrioritySet(INT_ADC0SS3, 2);
	// 4. Habilitar el secuenciador 3
	ADCSequenceEnable(ADC0_BASE, 3);
}

/*
 * Configuracion del timer para muestreo
 */
void Timer0_Init(uint32_t ciclos) {
	// 1. Configuracion de reloj al periferico
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	// 2. Configurar el timer para modo de 32 bits periodico
	TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
	// 3. Configurar el periodo de timeout
	TimerLoadSet(TIMER0_BASE, TIMER_A, ciclos - 1);
	// 4. Configurar trigger para ADC
	TimerControlTrigger(TIMER0_BASE, TIMER_A, true);
	// 5. Interrupciones
	// 6. Iniciar el timer
	TimerEnable(TIMER0_BASE, TIMER_A);
}

/*
 * Configuración de LEDs
 */
void configurarLEDs(void) {
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2);
	PF2 = 0xff;
}

/*
 * Rutina de interrupcion ADC0SS3
 */
void Int_ADC0SS3(void) {
	volatile uint16_t muestra;

	// Borrar la interrupcion
	ADCIntClear(ADC0_BASE, 3);

	// Leer la muestra
	//muestra = (ADC0_SSFIFO3_R & 0xfff);
	muestra = ADC0_SSFIFO3_R;

	// Operaciones sobre la muestra

	// Hacer toggle a PF2
	PF2 ^= 0xff;
}
