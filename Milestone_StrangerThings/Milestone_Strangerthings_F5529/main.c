#include <msp430.h> 


/**
 * Author: Jordan Alberico and Timothy Duong
 * Date Created: 10/10/18
 * Last Modified: 10/17/18
 * Milestone_Strangerthings_F5529
 */

int count = 0;
int ByteBuffer = 0;

extern void LEDSetup();
extern void TimerASetup();
extern void resetColor();
extern void UartSetup();

int main(void)
{

    WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
    UartSetup();                              // Sets up Uart
    LEDSetup();                               // Sets up LEDs
    TimerASetup();                            // Sets up timers
    __bis_SR_register(LPM0_bits + GIE);       // Enter LPM0, interrupts enabled
    __no_operation();                         // For debugger
}

void LEDSetup()
{
    P1SEL |= BIT2; // Sets port 1.2 to TimerA CCR1
    P1DIR |= BIT2; // Sets port 1.2 to output
    P1SEL |= BIT3; // Sets port 1.3 to TimerA CCR2
    P1DIR |= BIT3; // Sets port 1.3 to output
    P1SEL |= BIT4; // Sets port 1.4 to TimerA CCR3
    P1DIR |= BIT4; // Sets port 1.4 to output
}

void TimerASetup()
{
    TA0CTL = TASSEL_2 + MC_1 + OUTMOD_7;
    // Configures the timer for SMClk, Timer in UP mode , and outmode of Reset/set

    TA0CCTL1 = OUTMOD_2; // Sets TACCR1 to toggle
    TA0CCTL2 = OUTMOD_2; // Sets TACCR2 to toggle
    TA0CCTL3 = OUTMOD_2; // Sets TACCR3 to toggle

    TA0CCR0 = 0xFF; // Sets TA0CCR0
}

void UartSetup()
{
    P4SEL |= BIT4+BIT5;                       // P4.4,5 = USCI_A1 TXD/RXD
    UCA1CTL1 |= UCSWRST;                      // **Put state machine in reset**
    UCA1CTL1 |= UCSSEL_2;                     // SMCLK
    UCA1BR0 = 104;                            // 1MHz 9600 (see User's Guide)
    UCA1BR1 = 0;                              // 1MHz 9600
    UCA1MCTL |= UCBRS_1 + UCBRF_0;            // Modulation UCBRSx=1, UCBRFx=0
    UCA1CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
    UCA1IE |= UCRXIE;                         // Enable USCI_A1 RX interrupt
}

void resetColor()
{
    TA0CCR1 = 0x00; // Sets TA0CCR1
    TA0CCR2 = 0x00; // Sets TA0CCR2
    TA0CCR3 = 0x00; // Sets TA0CCR3
}


// Echo back RXed character, confirm TX buffer is ready first
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_A1_VECTOR))) USCI_A1_ISR (void)
#else
#error Compiler not supported!
#endif
{

  switch(__even_in_range(UCA1IV,4))
  {
  case 0:break;                             // Vector 0 - no interrupt
  case 2:                                   // Vector 2 - RXIFG
        switch(count)
        {
        case 0:
            UCA1TXBUF = UCA1RXBUF - 3;          // Subtract 3 from the length byte and transmit
            ByteBuffer = UCA1RXBUF;             // Store the number of expected  bytes
            count++;                            // Determines what byte we are handling
            break;
        case 1:                                 //red LED
            TA0CCR1 = UCA1RXBUF;                //sets red PWM
            count++;
            break;
        case 2:                                 //green LED
            TA0CCR2 = UCA1RXBUF;                //sets green PWM
            count++;
            break;
        case 3 :                                //blue LED
            TA0CCR3 = UCA1RXBUF;                //sets blue PWM
            count++;
            break;
        default:                                //after the first four bytes
            if(count <= ByteBuffer)             //if number of byes coming in is less than expected bytes,
            {
                UCA1TXBUF = UCA1RXBUF;          //pass on rest of the bytes
                count++;
            }
            else                                //handles next not expected byte as the first byte of a new package
            {
                count = 1;                      //reset count & skips case 0
                UCA1TXBUF = UCA1RXBUF - 3;      //subtract 3 from the length byte
                ByteBuffer = UCA1RXBUF;         //Store number of expected byte
                resetColor();                   //resets LED PWM
            }
            break;
        }

    break;
  case 4:break;                             // Vector 4 - TXIFG
  default: break;
  }
}
