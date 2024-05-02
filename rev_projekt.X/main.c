/* 
246833
0:GPIO-Rozsvecovani led BTN2 0 az 6 ledek pak preteceni
1:UART-Poslani retezce naopak
2:PWM-Ovladani rychlosti motoru potenciometrem led5 je opacna PWM1
3:ADC-Vypis pot1 a pot2 na displej ve V
4:DAC-Orezana sinusovka
5:GAME-Rychlost reakce
6:Prehravac hudby ? timer:
 */
#pragma config FOSC = HSMP      // Oscillator Selection bits (HS oscillator (medium power 4-16 MHz))
#pragma config PLLCFG = ON      // 4X PLL Enable (Oscillator multiplied by 4)
#pragma config WDTEN = OFF      // Watchdog Timer Enable bits (Watch dog timer is always disabled. SWDTEN has no effect.)

#include <xc.h>
#include <stdio.h>
#include <stdint.h>

#include "bsp.h"
#include "uart.h"
#include "fsm.h"
#include "lcd.h"
#include "adc.h"

#define _XTAL_FREQ 8E6
#define DELAY 0x0000//0xFFFF - (60000-1)

#define BTN1 PORTCbits.RC0
#define BTN2 PORTAbits.RA4
#define BTN3 PORTAbits.RA3
#define BTN4 PORTAbits.RA2

#define LED1 LATDbits.LATD2

void gpio_init(void) {
    TRISDbits.TRISD2 = 0;
    TRISDbits.TRISD3 = 0;
    TRISCbits.TRISC4 = 0;
    TRISDbits.TRISD4 = 0;
    TRISDbits.TRISD5 = 0;
    TRISDbits.TRISD6 = 0;
    
    TRISAbits.TRISA3 = 1;
    ANSELAbits.ANSA3 = 0;
    ANSELAbits.ANSA2 = 0;
}

void driveLED(char in){
        LATD2 = in & 1;             //LED0
        LATD3 = in & 2 ? 1 : 0;     //LED1
        LATC4 = in & 4 ? 1 : 0;     //LED2
        LATD4 = in & 8 ? 1 : 0;     //LED3
        LATD5 = in & 16 ? 1 : 0;    //LED4
        LATD6 = in & 32 ? 1 : 0;    //LED5
}

void knight_rider(void){
    unsigned char led_state = 0;
    for (int i = 0; i<6; i++){
        led_state |= (1 << i);
        driveLED(led_state);
        for(long i=1; i<50000; i++);
    } 
    
    for (int i = 5; i>=0; i--){
        led_state &= ~(1 << i);
        driveLED(led_state);
        for(long i=1; i<50000; i++);
    }    
}    

void gpio0(void) {
    static unsigned char led_state = 0;
    driveLED(~led_state);
    led_state = (led_state << 1) + 1;
    if (led_state > 0b111111) {
        led_state = 0;
    }
}


void main(void) {
    
    bsp_init();
    while(1) {
        if(BTN2) {
            gpio0();
            while(BTN2);
            __delay_ms(500);
        }   
    }
        
    return;
}
