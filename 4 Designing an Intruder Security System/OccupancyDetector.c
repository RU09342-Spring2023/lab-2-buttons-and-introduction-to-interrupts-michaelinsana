/*
 * OccupancyDetector.c
 *
 *  Created on: Jan 31, 2023
 *      Author: Michael Insana
 */

#include <msp430.h>

void gpioInit();
void blinkGLED();
void WarningState();
void AlertState();

char WarningEnable = 0;                       // Global Variable to track if the LED should be on or off
long int i = 0;

int main(){

    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    gpioInit();                 // Initialize all GPIO Pins for the project

    // Disable the GPIO power-on default high-impedance mode
    // to activate previously configured port settings
    PM5CTL0 &= ~LOCKLPM5;

    P2IFG &= ~BIT3;                         // P2.3 IFG cleared

    __bis_SR_register(GIE);                 // Enter LPM3 w/interrupt

    while(1) {

        blinkGLED();                //blinks Green LED
        __delay_cycles(3000000);             // Delay for 100000*(1/MCLK)=3s

        if (WarningEnable) {
           WarningState();

        }
        else
           P1OUT &= ~BIT0;                 // Set P1.0 to 0

    }

}

    void blinkGLED() {
        P6OUT ^= BIT6;              //toggle 1.0

    }

    void WarningState() {
        P6OUT &= ~BIT6;                 // Set P6.6 to 0
        WarningEnable = 0;

        for (i = 0; i < 500000; i++) {
            if ((i % 1000) == 0)          //count to .5 seconds for red LED toggle
                P1OUT ^= BIT0;                  // P1.0 = toggle

            if (i > 20000) {             //when i = 10 seconds
                WarningEnable = 0;
                AlertState();               //start alert State
                i = 500000;
            }

            if (WarningEnable == 1) {
                i = 500000;
                WarningEnable = 0;
            }


        }

    }

    void AlertState() {

        while (WarningEnable == 0)   {         // If S1 (P4.1) is not pressed
            P1OUT |= BIT0;                  // P1.0 = on

        }

        P1OUT &= ~BIT0;                  // P1.0 = off
        WarningEnable = 0;

    }


    void gpioInit()
    {
       // Setting Directions of Pins

           P2DIR &= ~BIT3;             // Configure P2.3 to an Input
           P4DIR &= ~BIT1;             // Configure P4.1 to an Input

           P1DIR |= BIT0;                          // Set P1.0 to output direction
           P1OUT &= BIT0;                         // Clear P1.0 output latch for a defined power-on state

           P6DIR |= BIT6;                          // Set P6.6 to output direction
           P6OUT &= BIT6;                         // Clear P6.6 output latch for a defined power-on state


       // Configuring Pullup Resistors per MSP430FR2355 Family User Guide
       /*
        *   PXDIR | PXREN | PXOUT | I/O Configuration
        *     0       0       X     Input
        *     0       1       0     Input with Pull Down Resistor
        *     0       1       1     Input With Pull Up Resistor
        *     1       X       X     Output
        */

           P2REN |= BIT3;               // Enable Resistor on P2.3
           P2OUT |= BIT3;               // Configure Resistor on P2.3 to be Pullup
           P2IES &= ~BIT3;                         // P2.3 Low --> High edge
           P2IE |= BIT3;                           // P2.3 interrupt enabled



           P4REN |= BIT1;               // Enable Resistor on P4.1
           P4OUT |= BIT1;               // Configure Resistor on P4.1 to be Pullup
    }


    // Port 2 interrupt service routine
    #pragma vector=PORT2_VECTOR
    __interrupt void Port_2(void)
    {
        P2IFG &= ~BIT3;                         // Clear P1.3 IFG
        WarningEnable = 1;                   // Enable if the toggle should be active
    }


