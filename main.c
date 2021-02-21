/*
   Adaptation of unicorn lamp for different combinations of flashing
   See:
   https://www.amazon.co.uk/AIZESI-Unicorn-Marquee-Decoration-Operated/dp/B073WX5FWJ

   The LED lamps are wired in parallel and appear to have about 2.7v across them so they
   obviously have current limiting resistors in the LEDs.  They can be directly driven from
   atmega
*/


#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>

#include <avr/sleep.h>
#include <inttypes.h>
#include <avr/interrupt.h>



//*************************************************************************************
// LED functions

// Ensure all LEDs set to appropriate DIRN.  They are on a mix of ports.
// takes care to leave the non-LED ports at default values.
void led_setup()
{
   DDRB |= 0b11000000;
   DDRC |= 0b00111111;
   DDRD |= 0b01110000;
}


// Direct the supplied 11-bit value to the relevant ports, low values switch lights on
// NB:  Inefficient, just done in a way to make it easier for me to get right.
void led_update(uint16_t value)
{
   // Current port values that we should keep.
   uint8_t for_portb = PORTB & 0b00111111;
   uint8_t for_portc = PORTC & 0b11000000;
   uint8_t for_portd = PORTD & 0b10001111;
   for_portc |= value & 0b0000000000111111;
   value >>= 6;
   for_portd |= (value & 1) << 4;
   value >>= 1;
   for_portb |= (value & 1) << 6;
   value >>= 1;
   for_portb |= (value & 1) << 7;
   value >>= 1;
   for_portd |= (value & 1) << 5;
   value >>= 1;
   for_portd |= (value & 1) << 6;
   // Probably won't make any difference, but I want to update all of these at the 'same' time, so do the write last.
   PORTB = for_portb;
   PORTC = for_portc;
   PORTD = for_portd;
}


// All the LEDs off
void led_off()
{
   led_update(0xffff);
}

// All the LEDs on
void led_on()
{
   led_update(0x0000);
}


//*************************************************************************************
// The low level button functions.

volatile uint8_t button_history = 0;


// Set the button port direction as input (the default)
// Also, enable pull-up.
void button_setup()
{
   DDRD &= 0b11111011;  // ensure pin2 is input 
   PORTD |= (1 << 2);  // pull up 
}


// Reset the button history, as though it hasn't been pressed in the last ~1 sec.
void button_reset(uint8_t history)
{
   cli();
   button_history = history;
   sei();
}


// Take a sample of the button state, add it to the history.  This should only be called by the ISR,
// unless the ISR is inactive.
void button_sample()
{
   button_history <<= 1;  // shift bits along
   button_history |= ((PIND & (1 << 2)) >> 2);   // sample the current value at BIT2
}


// Read the sequence of last 8 values, the actual value returned will determine long/short press.
// if the button has been sampled at correct interval
uint8_t button_read()
{
   uint8_t tmp;
   cli();
   tmp = button_history;
   sei();
   return tmp;
}


uint8_t button_has_history(uint8_t expected, uint8_t mask)
{
   uint8_t tmp = button_read();
   if ((tmp & mask) == expected)
   {
      return 1;
   }
   return 0;
}


// Wait until the history of past values takes given sequence
// Note this will never happen unless the timer is active, taking samples.
// Returns true if encountering a long press somehow.
uint8_t button_wait_history(uint8_t expected, uint8_t mask)
{
   uint8_t tmp;
   uint8_t long_press = 0;
   while (1)
   {
      tmp = button_read();
      if (tmp == 0)
      {
         long_press = 1;
      }
      if ((tmp & mask) == expected)
      {
         return long_press;
      }
   }
}


// Interrupt on overflow means we need to set the time interval to approx: <overflow> - <interval>
static const uint16_t TIMER_PERIOD = (65535 - 802);   // 0.1 second approx @8MHz

volatile uint16_t timer_count = 0;


// start the timer.
void timer_start()
{
   cli();
   timer_count = 0;
   TCNT1 = TIMER_PERIOD;   
   TCCR1A = 0x00;
   //TCCR1B = (1<<CS10) | (1<<CS12);  // Timer mode with 1024 prescler
   TCCR1B = (1<<CS10) | (1<<CS12);  // Timer mode with 1024 prescler
   TIMSK1 = (1 << TOIE1);   // Enable timer1 overflow interrupt(TOIE1)
   sei();        // Enable global interrupts by setting global interrupt enable bit in SREG
}


// Do we have to stop the timer during sleep? <shrug> I'm going to disable it just in case.
void timer_stop()
{
   TIMSK1 &= ~(1 << TOIE1);
}


// Getting a count number is handy for timing things.  Stop and start the timer to reset this.
uint16_t timer_get_count()
{
   uint16_t tmp;
   cli();
   tmp = timer_count;
   sei();
   return tmp;
}


// Only to be used immediately after timer reset, or count can wrap around.
void timer_delay(uint8_t delay)
{
   uint16_t start = timer_get_count();
   uint16_t finish = start + delay;
   while (timer_get_count() < finish)
   {

   }
}


// Wait for the timer count to tick over
void timer_tick()
{
   uint16_t tick = timer_get_count();
   while (tick == timer_get_count())
   {

   }
}


ISR(TIMER1_OVF_vect)    // Timer1 ISR
{
   TCNT1 = TIMER_PERIOD;   // for 0.1 sec at 8 MHz
   button_sample();
   timer_count ++;
}


void circle()
{
    uint16_t position = 1;
    button_wait_history(0x11, 0x11);   // ensure button not pressed.
    timer_tick();   // sync the clock before the loop.
    
    for(;;) {
      led_update(~position);
      position <<= 1;
      if (position > 0b0000010000000000)
      {
         position = 1;
      }
      timer_tick();
      if (button_has_history(0x000, 0x111))
      {
         button_wait_history(0x11, 0x11);   // ensure button not pressed.
         return;
      }
    }    
}


void wave_position(uint8_t position)
{
      switch (position)
      {
        case 0:
           led_update(~0b00000011000);
           return;
        case 1:
           led_update(~0b00000100111);
           return;
        case 2:
           led_update(~0b10001000000);
           return;
        case 3:
           led_update(~0b01010000000);
           return;
        case 4:
           led_update(~0b00100000000);           
           return;
        case 5:
           led_update(~0b00000000000);           
           return;
        case 6:
           led_update(~0b00000000000);           
           return;
      }
      return;
}


void wave()
{
    uint8_t position = 0;
    button_wait_history(0x11, 0x11);   // ensure button not pressed.
    timer_tick();
    
    for(;;) {
      wave_position(position);
      position ++;
      if (position>6)
      {
        position = 0;
      }
      timer_tick();

      if (button_has_history(0x000, 0x111))
      {
         button_wait_history(0x11, 0x11);   // ensure button not pressed.
         return;
      }
    }    
}


void random_pattern()
{
    button_wait_history(0x11, 0x11);   // ensure button not pressed.
    timer_tick();
    for(;;) {
      led_update(rand());
      timer_tick();
      if (button_has_history(0x000, 0x111))
      {
         button_wait_history(0x11, 0x11);   // ensure button not pressed before we move on.
         return;
      }
    }          
}


void short_delay(uint8_t count)
{
   uint16_t volatile index = 0;
   for (index = 0; index < count; index++)
   {
   }
}



volatile uint8_t button_got_interrupt;

// One-shot interrupt
void button_irq_enable()
{
    cli();
    EIMSK |= (1 << INT0);                       // enable int0
    button_got_interrupt = 0;
    sei();    
}


void sleep()
{
   set_sleep_mode(SLEEP_MODE_PWR_DOWN);
   sleep_enable();
   sleep_cpu();
   sleep_disable();
}


ISR(INT0_vect)
{
   EIMSK &= ~(1 << INT0);  // no more please
   button_got_interrupt = 1;
}



void fun_time()
{
       // main program loop
   circle();
   wave();
   random_pattern();
}




void step(uint16_t count)
{
   led_update(count);
}


int main(void)
{
   int long_press;
   led_setup();    
   button_setup();
   //ADCSRA = 0;  // disable ADC
   

   //step(1);

   while (1)
   {
      led_off();
      timer_stop();
      //step(2);
      button_irq_enable();
      //step(3);
      sleep();   // sleep puts out light
      led_on();

      // Once out of sleep, let's assume the timer is stopped, and the button is pressed and has been for a while.
      // We want to wait for it to be released.
      // Set up the history such that it's been pressed (held low) for the last 0.5 second.  It's not a real history, because in sleep the 
      // button wasn't sampled.  It makes the following logic work, however.
      button_reset(0b11110000);
      timer_start();   // now start sampling.
      //step(4);
      button_wait_history(0b1111, 0b1111);   // released for 0.5 sec.
      //step(5);
      button_wait_history(0b00, 0b11);             // pressed for 3/10 sec.
      //step(6);
      long_press = button_wait_history(0b11, 0b11);             // released for 3/10 sec.
      if (long_press)
      {
         fun_time();
      }
   }
   step(7);  // should never get here.
    
   return 0;
}

