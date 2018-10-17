#include <msp430.h> 


/**
 * Author: Jordan Alberico
 * Date Created: 10/10/18
 * Last Modified: 10/10/18
 * Milestone_Strangerthings_F5529
 */

extern void LEDSetup();
extern void TimerASetup();
extern void UartSetup();

int length;
int count;

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	LEDSetup(); // Configures LEDs
	TimerASetup(); // Configures TimerA
	UartSetup(); // Configures Uart
	__no_operation();
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
 //   TA0CCR1 = 0x7E; // Sets TA0CCR1
 //   TA0CCR2 = 0x00; // Sets TA0CCR2
 //   TA0CCR3 = 0xFF; // Sets TA0CCR3
}

void UartSetup()
{
    P4SEL |= BIT4+BIT5;                       // P3.3,4 = USCI_A0 TXD/RXD
     UCA1CTL1 |= UCSWRST;                      // **Put state machine in reset**
     UCA1CTL1 |= UCSSEL_2;                     // SMCLK
     UCA1BR0 = 9;                              // 1MHz 115200 (see User's Guide)
     UCA1BR1 = 0;                              // 1MHz 115200
     UCA1MCTL |= UCBRS_1 + UCBRF_0;            // Modulation UCBRSx=1, UCBRFx=0
     UCA1CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
     UCA1IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt
}

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
    while (!(UCA1IFG&UCTXIFG));             // USCI_A0 TX buffer ready?
    UCA1TXBUF = UCA1RXBUF;                  // TX -> RXed character
    break;
  case 4:break;                             // Vector 4 - TXIFG
  default: break;
  }
}












/*void UartSetup()
{
    P4SEL |= BIT4+BIT5;                       // P3.3,4 = USCI_A1 TXD/RXD
    UCA1CTL1 |= UCSWRST;                      // **Put state machine in reset**
    UCA1CTL1 |= UCSSEL_1;                     // ACLK
    UCA1BR0 = 104;                            // 1MHz 9600
    UCA1BR1 = 0;                              // 1MHz 9600
    UCA1MCTL |= UCBRS_1 + UCBRF_0;            // Modulation UCBRSx=1, UCBRFx=0
    UCA1CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
    UCA1IE |= UCRXIE;                         // Enable USCI_A1 RX interrupt

    //UART configuration provided by TI Resource Explorer Code
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_A1_VECTOR))) USCI_A1_ISR (void)
#else
#error Compiler not supported!
#endif
{
    char data = UCA1RXBUF;
    TA0CCR1 = data;
}



#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_A0_VECTOR))) USCI_A0_ISR (void)
#else
#error Compiler not supported!
#endif
{

       TA0CCR1 = 0x7E; // Sets TA0CCR1
       TA0CCR2 = 0x00; // Sets TA0CCR2
       TA0CCR3 = 0xFF; // Sets TA0CCR3
    switch(__even_in_range(UCA0IV, 4))
   //     {
     //       case USCI_NONE: break;
            case 2:
                while(!(UCA0IFG & UCTXIFG))
                    switch (count)
                    {
                    case 0 :
                        UCA0TXBUF -= 3;
                        length = UCA0TXBUF;
                        count++;
                        break;
                    case 1 :
                        TA0CCR1 = UCA0RXBUF;
                        count++;
                        break;
                    case 2 :
                        TA0CCR2 = UCA0RXBUF;
                        count++;
                        break;
                    case 3 :
                        TA0CCR3 = UCA0RXBUF;
                        count++;
                        break;
                    default :
                        //Send rest of packet to next node (Tx = Rx)
                        UCA1TXBUF = UCA1RXBUF;
                        break;
                    }
     //       case USCI_UART_UCTXIFG: break;
      //      case USCI_UART_UCSTTIFG: break;
       //     case USCI_UART_UCTXCPTIFG: break;
        //    default: break;
        }



//}



#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
    switch(UCA1IV)
    {
        case 0 :
            // calculate and send length byte
            length = UCA1RXBUF;
            break;
        case 1 :
            // set red LED PWM
            TA0CCR1 = UCA1RXBUF;
            length--;
            break;
        case 2 :
            // Set Green LED PWM
            TA0CCR2 = UCA1RXBUF;
            length--;
            break;
        case 3 :
            //set blue led and transmit buffer == package length
            TA0CCR3 = UCA1RXBUF;
            length--;
            UCA1TXBUF = length;
            break;
        default:
            //Send rest of packet to next node (Tx = Rx)
            UCA1TXBUF = UCA1RXBUF;
        break;
    }
    // increment/reset byte counter
} */
