#include <avr/io.h>
#include <util/delay.h>
#define PWM_A    (1<<PB1)
#define PWM_B    (1<<PB2)
int czujniki[7];
int czujnik0 = 0,czujnik1 = 0,czujnik2 = 0,czujnik3 = 0,czujnik4 =0,czujnik5 = 0,czujnik6 = 0;
int blad = 0;
//int wagi[7] = {150, 150, 70, -70, -150, -150, 0};
int wagi[7] = {50, 20, 10, -10, -20, -50, 0};
int flaga = 0;
int prev_err = 0;
int blad, pop_blad = 0,Kp = 4, Kd = 10; // regulator
//inne deklaracje

/* FUNKCJA G£ÓWNA */
int PD(){
    //zmienna blad zawiera aktualny wynik fukcji licz_blad()
    int rozniczka = blad - pop_blad;
    pop_blad = blad;
    return Kp*blad + Kd*rozniczka;
}

void pwm(int lewy,int prawy){
if(lewy>255)
{
	lewy = 255;
}
if(prawy>255)
{
	prawy = 255;
}

OCR1A = lewy; //    lewy
OCR1B = prawy; // prawy

PORTD |=(1<<PD5);
PORTD &=~(1<<PD6); //przód

PORTD |=(1<<PD4);
PORTD &=~(1<<PD3);

}

void przycisk_on(){
	if(!(PIND & (1<<PIND7)))        // reakcja na czujnik nr 2
		  {
		    	    flaga = 1;
		  }
}
void przycisk_kd(){
	if(!(PINB & (1<<PINB0)))        // reakcja na czujnik nr 2
		  {
					Kp = Kp + 1;
					_delay_ms(100);
		  }
}


void czytaj_czujniki()
{

	if(!(PINC & (1<<PINC0)))        // reakcja na czujnik nr 0
	 {
	    	    czujnik0 = 0;
	 }
	 else
	 {
	    	    czujnik0 = 1;
	 }

	if(!(PINC & (1<<PINC1)))        // reakcja na czujnik nr 1
	 {
	    	    czujnik1 = 0;
	 }
	 else
	 {
	    	    czujnik1 = 1;
	 }

	if(!(PINC & (1<<PINC2)))        // reakcja na czujnik nr 2
	  {
	    	    czujnik2 = 0;
	  }
	  else
	  {
	    	    czujnik2 = 1;
	  }
	if(!(PINC & (1<<PINC3)))        // reakcja na czujnik nr 3
	  {
	    	    czujnik3 = 0;
	  }
	  else
	  {
	    	    czujnik3 = 1;
	  }

	if(!(PINC & (1<<PINC4)))        // reakcja na czujnik nr 4
	  {
	    	    czujnik4 = 0;
	  }
	  else
	  {
	    	   	czujnik4 = 1;
	  }
	if(!(PINC & (1<<PINC5)))        // reakcja na czujnik nr 5
	  {
	    	    czujnik5 = 0;
	  }
	  else
	  {
	    	    czujnik5 = 1;
	  }
	if(!(PINC & (1<<PINC2))  || !(PINC & (1<<PINC3)))        // reakcja na czujnik nr 6 czujnik wirtualny
	  {
	    	    czujnik6 = 0;
	  }
	  else
	  {
	    	    czujnik6 = 1;
	  }

	czujniki[0]=czujnik0;czujniki[1]=czujnik1;czujniki[2]=czujnik2;czujniki[3]=czujnik3;czujniki[4]=czujnik4;czujniki[5]=czujnik5;czujniki[6]=czujnik6;
}

int licz_blad()
{
    int err = 0;
    int ilosc = 0;

    for(int i=0; i<7; i++)
        {
            err += czujniki[i]*wagi[i];        // wagi kolejnych czujników (dla i z zakresu [0;7]): -30, -20, -10, 0, 10, 20, 30
            ilosc += czujniki[i];            // czujniki[i] ma wartoœæ 1/0
        }

    if(ilosc != 0)
    {
        err /= ilosc;
        prev_err = err;
    }
    else
    {
        if(prev_err < -20)                // linia ostatanio widziana po lewej stronie - ustalamy ujemny b³¹d wiêkszy od b³êdu skrajnego lewego czujnika
            err = -40;
        else if(prev_err > 20)            // linia ostatanio widziana po prawej stronie - analogicznie do powy¿szego
            err = 40;
        else                            // przerwa w linii - trzeba jechaæ prosto
            err = 0;
    }

    return err;
}




int main(void)
{

    /* USTAWIANIE WYJŒÆ */
    DDRB |= (PWM_A|PWM_B);    //wyjœcia pwm
    DDRD |= (1<<PD6);
    DDRD |= (1<<PD4);
    DDRD |=	(1<<PD1); // dioda zielona
    DDRD |= (1<<PD0); //dioda czerwona
    PORTD = (1<<PD7); //wejscie przysisk S1

    /* INICJALIZACJA PWM - TIMER1 */
    TCCR1A |= (1<<WGM10);                      // Fast PWM 8bit
    TCCR1B |= (1<<WGM12);
    TCCR1A |= (1<<COM1A1)|(1<<COM1B1) ;        //Clear OC1A/OC1B on Compare Match, set OC1A/OC1B at BOTTOM
    TCCR1B |= (1<<CS10)|(1<<CS11);             // Preksaler = 64  fpwm = 976,5Hz
    OCR1A = 0;         //kana³ A = 0
    OCR1B = 0;         //kana³ B = 0


    /* PÊTLA G£ÓWNA */

    while(1)
    {
    	przycisk_on();
    	przycisk_kd();
    	while(flaga){


    		czytaj_czujniki();
    		blad = licz_blad();
    		int regulacja = PD();
    		pwm(255+regulacja, 255-regulacja);
    		//pwm(255+blad, 255-blad);

    		if(czujnik6==1) {
    	   	 PORTD |=(1<<PD1); //swiece diode
    	   	 PORTD |=(1<<PD0); //swiece diode
		}
		else
		{
    	 	   PORTD &= ~(1<<PD1); // gaszê diode
    		   PORTD &= ~(1<<PD0); // gaszê diode

		}


    	}

        //g³ówny program - odczyt czujników, odpowiednie obliczenia, sterowanie silnikami itd.
    }

}
