//0:GPIO-Rozsvecovani led BTN2 0 az 6 ledek pak preteceni
//1:UART-Poslani retezce naopak
//2:PWM-Ovladani rychlosti motoru potenciometrem led5 je opacna PWM1
//3:ADC-Vypis pot1 a pot2 na displej ve V
//4:DAC-Orezana sinusovka
//5:GAME-Rychlost reakce
//6:Prehravac hudby ? timer:

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
#define SETDUTY(x) CCPR1L = x



void rc_isr_handle(void);

void init(fsm_t *fsm, uint8_t event);
void state0(fsm_t *fsm, uint8_t event);
void state1(fsm_t *fsm, uint8_t event);
void state2(fsm_t *fsm, uint8_t event);
void state3(fsm_t *fsm, uint8_t event);
void state4(fsm_t *fsm, uint8_t event);
void state5(fsm_t *fsm, uint8_t event);
void state6(fsm_t *fsm, uint8_t event);
void gpio_state0(fsm_t *fsm, uint8_t event);
void uart_state1(fsm_t *fsm, uint8_t event);
void pwm_state2(fsm_t *fsm, uint8_t event);
void adc_state3(fsm_t *fsm, uint8_t event);
void dac_state4(fsm_t *fsm, uint8_t event);
void game_state5(fsm_t *fsm, uint8_t event);
void hw_state6(fsm_t *fsm, uint8_t event);

typedef struct{

    char data[32];
    char idx;
    int last;
    
}uart_msg_t;

volatile static uart_msg_t msg, msg_rev;

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
    
    
    
    //init - PWM
    TRISDbits.RD5 = 1;              // nastavim jako vstup pin P1B
    TRISCbits.RC2 = 1;              // nastavim jako vstup pin P1A
    
    
    CCPTMRS0bits.C1TSEL = 0b00;     // Timer 2 
    PR2 = 199;                      // f = 10kHz
    CCP1CONbits.P1M = 0b00;         // PWM single
    CCP1CONbits.CCP1M = 0b1100;     // PWM single
    CCPR1L = 0;                     // strida 0%    
    T2CONbits.T2CKPS = 0b00;        // 1:1 Prescaler
    TMR2IF = 0;                     // nastavi se az pretece timer
    TMR2ON = 1;                     // staci zapnout defaultne je nastaven jak chceme
    while(!TMR2IF){};               // cekam az jednou pretece
        
    TRISDbits.RD5 = 0;              // nastavim jako vystup pin P1B
    TRISCbits.RC2 = 0;              // nastavim jako vystup pin P1A
            
    // ADC pro potenciometr
    ANSELE = 0b1;                   //RE0/AN5
    ADCON2bits.ADFM = 0;            //left justified
    ADCON2bits.ADCS = 0b110;        //Fosc/64
    ADCON2bits.ACQT = 0b110;        //16 Tad
    ADCON0bits.ADON = 1;            //ADC zapnout
    ADCON0bits.CHS = 5;             // kanal AN5
    

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
            LCD_ShowString(2, "      ^  v  >  <");
            bsp_set_timeout(5000);
            break;
        case EV_EXIT:
            printf("Init state exit\n");
            break;
        case EV_BTN1_PRESSED:       // skip pro vsechny tlacitka
        case EV_BTN2_PRESSED:
        case EV_BTN3_PRESSED:
        case EV_BTN4_PRESSED:
        case EV_TIMEOUT:
            fsm_transition(fsm, &state0);
            break;
    }
}

void state0(fsm_t *fsm, uint8_t event){
    
	switch(event){
        case EV_ENTRY:
            printf("Enter state 0\n");
            LCD_ShowString(1, ">>> 0_GPIO          ");
            LCD_ShowString(2, "    1_UART          ");
            break;
        case EV_EXIT:
            printf("Exit state 0\n");
            LCD_Clear();
            break;
        case EV_BTN2_PRESSED:
            fsm_transition(fsm, &state1);
            break;
        case EV_BTN3_PRESSED:
            fsm_transition(fsm, &gpio_state0);
            break;
    }
}

void state1(fsm_t *fsm, uint8_t event){
    
	switch(event){
        case EV_ENTRY:
            printf("Enter state 1\n");
            LCD_ShowString(1, ">>> 1_UART           ");
            LCD_ShowString(2, "    2_PWM            ");
            break;
        case EV_EXIT:
            printf("Exit state 1\n");
            LCD_Clear();
            break;
        case EV_BTN1_PRESSED:
            fsm_transition(fsm, &state0);
            break;
        case EV_BTN2_PRESSED:
            fsm_transition(fsm, &state2);
            break;
        case EV_BTN3_PRESSED:
            fsm_transition(fsm, &uart_state1);
            break;
    }
}

void state2(fsm_t *fsm, uint8_t event){
    
	switch(event){
        case EV_ENTRY:
            printf("Enter state 2\n");
            LCD_ShowString(1, ">>> 2_PWM            ");
            LCD_ShowString(2, "    3_ADC            ");
            break;
        case EV_EXIT:
            printf("Exit state 2\n");
            LCD_Clear();
            break;
        case EV_BTN1_PRESSED:
            fsm_transition(fsm, &state1);
            break;
        case EV_BTN2_PRESSED:
            fsm_transition(fsm, &state3);
            break;
        case EV_BTN3_PRESSED:
            fsm_transition(fsm, &pwm_state2);
            break;
    }
}

void state3(fsm_t *fsm, uint8_t event){
    
	switch(event){
        case EV_ENTRY:
            printf("Enter state 3\n");
            LCD_ShowString(1, ">>> 3_ADC            ");
            LCD_ShowString(2, "    4_DAC            ");
            break;
        case EV_EXIT:
            printf("Exit state 3\n");
            LCD_Clear();
            break;
        case EV_BTN1_PRESSED:
            fsm_transition(fsm, &state2);
            break;
        case EV_BTN2_PRESSED:
            fsm_transition(fsm, &state4);
            break;
        case EV_BTN3_PRESSED:
            fsm_transition(fsm, &adc_state3);
            break;
    }
}

void state4(fsm_t *fsm, uint8_t event){
    
	switch(event){
        case EV_ENTRY:
            printf("Enter state 4\n");
            LCD_ShowString(1, ">>> 4_DAC            ");
            LCD_ShowString(2, "    5_GAME           ");
            break;
        case EV_EXIT:
            printf("Exit state 4\n");
            LCD_Clear();
            break;
        case EV_BTN1_PRESSED:
            fsm_transition(fsm, &state3);
            break;
        case EV_BTN2_PRESSED:
            fsm_transition(fsm, &state5);
            break;
        case EV_BTN3_PRESSED:
            fsm_transition(fsm, &dac_state4);
            break;
    }
}

void state5(fsm_t *fsm, uint8_t event){
    
	switch(event){
        case EV_ENTRY:
            printf("Enter state 5\n");
            LCD_ShowString(1, ">>> 5_GAME           ");
            LCD_ShowString(2, "    6_HW             ");
            break;
        case EV_EXIT:
            printf("Exit state 5\n");
            LCD_Clear();
            break;
        case EV_BTN1_PRESSED:
            fsm_transition(fsm, &state4);
            break;
        case EV_BTN2_PRESSED:
            fsm_transition(fsm, &state6);
            break;
        case EV_BTN3_PRESSED:
            fsm_transition(fsm, &game_state5);
            break;
    }
}

void state6(fsm_t *fsm, uint8_t event){
    
	switch(event){
        case EV_ENTRY:
            printf("Enter state 6\n");
            LCD_ShowString(1, "    5_GAME           ");
            LCD_ShowString(2, ">>> 6_HW             ");
            break;
        case EV_EXIT:
            printf("Exit state 6\n");
            LCD_Clear();
            break;
        case EV_BTN1_PRESSED:
            fsm_transition(fsm, &state5);
            break;
        case EV_BTN3_PRESSED:
            fsm_transition(fsm, &hw_state6);
            break;
    }
}

void gpio_state0(fsm_t *fsm, uint8_t event){

    static uint8_t leds = 0;
    switch(event){
        case EV_ENTRY:
            printf("Enter state 0_GPIO\n");
            LCD_ShowString(1, "Rozsvecovani led");
            LCD_ShowString(2, "mackej BTN2     ");
            leds = 0b11000000;
            break;
        case EV_EXIT:
            printf("Exit state 0_GPIO\n");
            LCD_Clear();
            bsp_drive_led(0);
            break;
        case EV_BTN2_PRESSED:
            leds = (leds >> 1);
            bsp_drive_led(leds);
            leds += 0b10000000;
            if (leds == 0b11111111) leds = 0b10000000;
            break;
        case EV_BTN4_PRESSED:
            fsm_transition(fsm, &state0);
            break;
    }
}

void uart_state1(fsm_t *fsm, uint8_t event){

    switch(event){
        case EV_ENTRY:
            printf("Enter state 1_UART\n");
            LCD_ShowString(1, "Zadej zpravu:         ");
            printf("Zadej zpravu:\n");
            break;
        case EV_EXIT:
            printf("Exit state 1_UART\n");
            LCD_Clear();
            break;
        case EV_BTN4_PRESSED:
            fsm_transition(fsm, &state1);
            break;
        case EV_RC_MSG:
            msg_rev.idx = 0;
            while (msg.last > -1) {
                msg_rev.data[msg_rev.idx++] = msg.data[msg.last--];
            }
            msg_rev.data[msg_rev.idx] = '\0';
            printf("Zprava: %s\n", msg_rev.data);
    }
}

void pwm_state2(fsm_t *fsm, uint8_t event){

    static uint8_t leds = 0;
    switch(event){
        case EV_ENTRY:
            printf("Enter state 2_PWM\n");
            LCD_ShowString(1, "Toceni motoru         ");
            LCD_ShowString(2, "toc potenciometrem      ");
            leds = 0b11000000;
            PSTR1CON |= 0b0011;               // steering na P1B a P1A
            break;
        case EV_EXIT:
            printf("Exit state 2_PWM\n");
            LCD_Clear();
            PSTR1CON &= 0b1100;             // zastavi steering na P1B a P1A
            bsp_drive_led(0);
            break;
        case EV_BTN4_PRESSED:
            fsm_transition(fsm, &state2);
            break;
        case EV_TICK:
            GODONE = 1;                 // spustim konverzi
            while(GODONE){};            // cekam na konverzi
            SETDUTY(ADRESH);            // nastavim stridu pouzivam jen 8hornich bitu
    }
}

void adc_state3(fsm_t *fsm, uint8_t event){

	switch(event){
        case EV_ENTRY:
            LCD_ShowString(1, "Potenciometr:           ");
            printf("Enter state 3_ADC\n");
            break;
        case EV_EXIT:
            printf("Exit state 3_ADC\n");
            LCD_Clear();
            break;
        case EV_BTN4_PRESSED:
            fsm_transition(fsm, &state3);
            break;
        case EV_TICK:
        {
            char str[17];
            sprintf(str, "POT1:%.1fPOT2:%.1f", (float)adc_read(POT1)*3.22265625*0.001, (float)adc_read(POT2)*3.22265625*0.001);
            LCD_ShowString(2, str);
            break;
        }
    }
}

void dac_state4(fsm_t *fsm, uint8_t event){
    
    unsigned char led_state = 0b110000;
    switch(event){
        case EV_ENTRY:
            printf("Enter state 4_DAC\n");
            LCD_ShowString(1, "Rozsvecovani led");
            LCD_ShowString(2, "mackej BTN2     ");
            break;
        case EV_EXIT:
            printf("Exit state 4_DAC\n");
            LCD_Clear();
            bsp_drive_led(0);
            break;
        case EV_BTN2_PRESSED:
            while (!BTN4) {
                for (int i = 0; i<4; i++){
                    led_state = led_state >> 1;
                    bsp_drive_led(led_state);
                    for(long i=1; i<50000; i++);
                } 
                for (int i = 0; i<4; i++){
                    led_state = led_state << 1;
                    bsp_drive_led(led_state);
                    for(long i=1; i<50000; i++);
                }
            }
            break;
        case EV_BTN4_PRESSED:
            fsm_transition(fsm, &state4);
            break;
        case EV_TIMEOUT:
            fsm_transition(fsm, &dac_state4);
            break;
    }
}

void game_state5(fsm_t *fsm, uint8_t event){

    static uint8_t leds = 0;
    switch(event){
        case EV_ENTRY:
            printf("Enter state 5_GAME\n");
            LCD_ShowString(1, "Rozsvecovani led");
            LCD_ShowString(2, "mackej BTN2     ");
            leds = 0b11000000;
            break;
        case EV_EXIT:
            printf("Exit state 5_GAME\n");
            LCD_Clear();
            bsp_drive_led(0);
            break;
        case EV_BTN2_PRESSED:
            leds = (leds >> 1);
            bsp_drive_led(leds);
            leds += 0b10000000;
            if (leds == 0b11111111) leds = 0b10000000;
            break;
        case EV_BTN4_PRESSED:
            fsm_transition(fsm, &state5);
            break;
    }
}

void hw_state6(fsm_t *fsm, uint8_t event){

    static uint8_t leds = 0;
    switch(event){
        case EV_ENTRY:
            printf("Enter state 6_HW\n");
            LCD_ShowString(1, "Rozsvecovani led");
            LCD_ShowString(2, "mackej BTN2     ");
            leds = 0b11000000;
            break;
        case EV_EXIT:
            printf("Exit state 6_HW\n");
            LCD_Clear();
            bsp_drive_led(0);
            break;
        case EV_BTN2_PRESSED:
            leds = (leds >> 1);
            bsp_drive_led(leds);
            leds += 0b10000000;
            if (leds == 0b11111111) leds = 0b10000000;
            break;
        case EV_BTN4_PRESSED:
            fsm_transition(fsm, &state6);
            break;
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
        msg.last = msg.idx - 1;
        msg.idx = 0;
        fsm_add_event(EV_RC_MSG);
    }
}
