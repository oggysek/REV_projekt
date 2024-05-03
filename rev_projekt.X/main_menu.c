// REV FSM
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

#define _XTAL_FREQ 32e6

void rc_isr_handle(void);

void init(fsm_t *fsm, uint8_t event);
void state1(fsm_t *fsm, uint8_t event);
void state2(fsm_t *fsm, uint8_t event);
void state3(fsm_t *fsm, uint8_t event);
void state4(fsm_t *fsm, uint8_t event);
void uart_state(fsm_t *fsm, uint8_t event);
void pot_state(fsm_t *fsm, uint8_t event);

typedef struct{

    char data[32];
    char idx;
    
}uart_msg_t;

volatile static uart_msg_t msg;

int main(void) {
    __delay_ms(100);
    bsp_init();
    uart_init();
    adc_init();
    LCD_Init();
    
    
    GIEL = 1;                       
    GIEH = 1; 
    
    TMR2ON = 1;                     
    TMR1ON = 1;
    
    bsp_reg_RC_cb(rc_isr_handle);
    
	fsm_t   fsm; 
    uint8_t event = 0; 

    fsm_init(&fsm, &init);

    while(1){
        if (fsm_get_event(&event)){

            fsm_dispatch(&fsm, event);

        }
    }
	
	return 0;
}

//State functions
//##############################################################################

void init(fsm_t *fsm, uint8_t event){
    
    switch(event){
        case EV_ENTRY:
            LCD_ShowString(1, "Welcome         ");
            bsp_set_timeout(3000);
            break;
        case EV_EXIT:
            printf("Init state exit\n");
            break;
        case EV_TIMEOUT:
            fsm_transition(fsm, &state1);
            break;
    }
}

void state1(fsm_t *fsm, uint8_t event){
    
    static uint8_t leds = 0;
    
	switch(event){
        case EV_ENTRY:
            LCD_ShowString(1, ">>> 0_GPIO          ");
            LCD_ShowString(2, "    State2           ");
            leds = 0b11000000;
            break;
        case EV_EXIT:
            bsp_drive_led(0);
            printf("Exit state 1\n");
            break;
        case EV_BTN1_PRESSED:
            fsm_transition(fsm, &state2);
            break;
        case EV_BTN3_PRESSED:
            leds = (leds >> 1);
            bsp_drive_led(leds);
            leds += 0b10000000;
            if (leds == 0b11111111) leds = 0b10000000;
            break;
    }
}

void state2(fsm_t *fsm, uint8_t event){
    
	switch(event){
        case EV_ENTRY:
            LCD_ShowString(1, ">>> State2           ");
            LCD_ShowString(2, "    State3           ");
            bsp_set_timeout(500);
            break;
        case EV_EXIT:
            printf("Exit state 2\n");
            LED6 = 1;
            LCD_Clear();
            break;
        case EV_BTN1_PRESSED:
            fsm_transition(fsm, &state3);
            break;
        case EV_BTN2_PRESSED:
            fsm_transition(fsm, &state1);
            break;
        case EV_TIMEOUT:
            LED6 ^= 1;
            bsp_set_timeout(500);
            break;
    }
}

void state3(fsm_t *fsm, uint8_t event){
    
	switch(event){
        case EV_ENTRY:
            LCD_ShowString(1, ">>> State3           ");
            LCD_ShowString(2, "    State4           ");
            break;
        case EV_EXIT:
            printf("Exit state 3\n");
            LCD_Clear();
            break;
        case EV_BTN1_PRESSED:
            fsm_transition(fsm, &state4);
            break;
        case EV_BTN2_PRESSED:
            fsm_transition(fsm, &state2);
            break;
        case EV_BTN3_PRESSED:
            fsm_transition(fsm, &pot_state);
            break;
    }
}

void state4(fsm_t *fsm, uint8_t event){
    
	switch(event){
        case EV_ENTRY:
            printf("Enter state 4\n");
            LCD_ShowString(1, "    State3           ");
            LCD_ShowString(2, ">>> State4           ");
            break;
        case EV_EXIT:
            printf("Exit state 4\n");
            LCD_Clear();
            break;
        case EV_BTN2_PRESSED:
            fsm_transition(fsm, &state3);
            break;
        case EV_BTN3_PRESSED:
            fsm_transition(fsm, &uart_state);
            break;
    }
}

void uart_state(fsm_t *fsm, uint8_t event){

    switch(event){
        case EV_ENTRY:
            printf("Enter state Uart\n");
            LCD_ShowString(1, "Zadej zpravu:         ");
            printf("Zadej zpravu:\n");
            break;
        case EV_EXIT:
            printf("Exit state Uart\n");
            LCD_Clear();
            break;
        case EV_BTN4_PRESSED:
            fsm_transition(fsm, &state4);
            break;
        case EV_RC_MSG:
            printf("Zprava: %s\n", msg.data);
    }

}
void pot_state(fsm_t *fsm, uint8_t event){

	switch(event){
        case EV_ENTRY:
            LCD_ShowString(1, "Potenciometr:           ");
            printf("Enter state POT\n");
            break;
        case EV_EXIT:
            printf("Exit state POT\n");
            LCD_Clear();
            break;
        case EV_BTN4_PRESSED:
            fsm_transition(fsm, &state3);
            break;
        case EV_TICK:
        {
            char str[17];
            sprintf(str, "POT: %.1f mV          ", (float)adc_read(POT1)*3.22265625);
            LCD_ShowString(2, str);
            break;
        }
    }

}

void rc_isr_handle(void){

    char rchar = RCREG1;

    if(rchar != '\n'){
        msg.data[msg.idx++] = rchar;
        if(msg.idx > 31){
            msg.idx = 0;
        }
    }
    else{
        msg.data[msg.idx] = '\0';
        msg.idx = 0;
        fsm_add_event(EV_RC_MSG);
    }
}
