#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
unsigned time=0;
unsigned char V_A0, V_A1;
volatile unsigned int segundos = 0;
volatile unsigned char conteo_activo = 0;
volatile unsigned char z = 0;
volatile float valve = 0.0; // Para enviar ADC si se desea
volatile unsigned int segundos_a_enviar = 0;
volatile unsigned int contador_ms = 0;

void config_USART(void){
    UCSR0C |= (1<<UCSZ00)|(1<<UCSZ01); 
    UCSR0B |= (1<<TXEN0)|(1<<RXEN0)|(1<<RXCIE0); 
    UBRR0 = 103; 
}

ISR(USART_UDRE_vect){
    unsigned int val = (int)(valve );  
    unsigned int d = (val / 100);  
    unsigned int u = (val / 10) % 10;    
    unsigned int de = (val % 10);

    unsigned int c_seg, d_seg, u_seg;

    switch (z) {
        case 0: 
            UDR0 = d + '0';
            z = 1;
            break;
        case 1:
            UDR0 = 46; // '.'
            z = 2;
            break;
        case 2:
            UDR0 = u+ '0';
            z = 3;
            break;
        case 3:
            UDR0 = de + '0';
            z = 4;
            break;
        case 4:
            UDR0 = 10; // '\n'
            z = 0;
            UCSR0B &= ~(1<<UDRIE0); // Detiene interrupción TX
            ADCSRA |= (1<<ADSC);    // Inicia nueva conversión ADC
            break;
        case 5: // Enviar segundos
            c_seg = segundos_a_enviar / 100;
            d_seg = (segundos_a_enviar / 10) % 10;
            u_seg = segundos_a_enviar % 10;

            UDR0 = c_seg + '0';
            z = 6;
            break;
        case 6:
            UDR0 = d_seg + '0';
            z = 7;
            break;
        case 7:
            UDR0 = u_seg + '0';
            z = 8;
            break;
        case 8:
            UDR0 = 10;
            z = 0;
            UCSR0B &= ~(1<<UDRIE0); 
            break;
    }
}







void config_timer2(void){ // Timer2 
    TCCR2A |= (1 << WGM21); // CTC
    TCCR2B |= (1 << CS22);  
    TIMSK2 |= (1 << OCIE2A); 
    OCR2A = 250; // 1ms 
}

ISR(TIMER2_COMPA_vect){

  time++;
  TCNT2=0;
  /*
  if(conteo_activo){
        contador_ms++;
        if(contador_ms >= 1000){ // 1 segundo acumulado
            contador_ms = 0;
            segundos++;
            segundos_a_enviar = segundos; // Copia segura
            z = 5; // Preparar para enviar segundos
             // Habilita transmisión USART
        }
    }*/
}






void config_INT0(void){
    EIMSK |= (1<<INT0);   
    EICRA |= (1<<ISC01);  
    DDRD &= ~(1<<PD2);
}

ISR(INT0_vect){
    valve=time/1000; 
    time=0; 
    UCSR0B |= (1<<UDRIE0);
}

int main(void){

    config_USART();
    config_timer2();
    config_INT0();
    sei(); 

    while(1){

    }
    return 0;
}
