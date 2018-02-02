
/*
 *
 * Pin Connections:
 * PE4 [pin 5]  -> RS
 * PE5 [pin 6]  -> EN
 * PD0 [pin 23] -> D4
 * PD1 [pin 24] -> D5
 * PD2 [pin 25] -> D6
 * PD3 [pin 26] -> D7
 *
 * pin number in [] indicates
 * energia pin reference!
 *
 */
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "display.h"
#include "driverlib/rom_map.h"
#include "buttons.h"
#include "utils/cmdline.h"
#include "driverlib/uart.h"
#include "driverlib/gpio.h"
#include "driverlib/pwm.h"
#include "driverlib/timer.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/interrupt.h"

#include "utils/uartstdio.h"


#define  freq  50
#define Period  80000000/freq
#define dutyCycle1 Period-1-80000 //1ms positive width
#define dutyCycle2  Period-1-160000 //2ms positive width
//dutyCycle1 > dutyCycle2

uint32_t currentPositionFirst = 90;
uint32_t currentPositionSecond = 90;
// Configure UART0 to console data which transmitted
void InitConsole(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTStdioConfig(0, 115200, 16000000);
}


void delayMS(int ms) {
    SysCtlDelay( (SysCtlClockGet()/(3*1000))*ms ) ;
}

void delayus(int ms) {
    SysCtlDelay( (SysCtlClockGet()/(3*1000000))*ms ) ;
}

uint32_t ServoWrite(uint32_t valor, int servoNo) {
    uint32_t i = 0;
    i = Period - ( (valor * 430) + 80000);
    if(servoNo)
        TimerMatchSet(WTIMER0_BASE, TIMER_B, i);
    else
        TimerMatchSet(WTIMER0_BASE, TIMER_A, i);
}

void setServoAngle(uint32_t angle, int servoNo){

    uint32_t currentPosition = 90;
    if(servoNo)
        currentPosition = currentPositionFirst;
    else
        currentPosition = currentPositionSecond;

    if(currentPosition < angle){
        uint32_t i = 1;
        for(i = currentPosition; i < angle; i++)
        {
           //TimerDisable(WTIMER0_BASE, TIMER_B);
           ServoWrite(i,servoNo);
           //TimerEnable(WTIMER0_BASE, TIMER_B);
           SysCtlDelay(26600);

        }

    }else if(currentPosition > angle){
        uint32_t i = 1;
        for(i = currentPosition; i > angle; i--)
        {
           //TimerDisable(WTIMER0_BASE, TIMER_B);
           ServoWrite(i,servoNo);
           //TimerEnable(WTIMER0_BASE, TIMER_B);
           SysCtlDelay(26600);

        }

    }else;


    if(servoNo)
        currentPositionFirst = currentPosition;
    else
       currentPositionSecond = currentPosition;

}


void initServo(){
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    SysCtlDelay(3);
    GPIOPinConfigure(GPIO_PC5_WT0CCP1);
    GPIOPinConfigure(GPIO_PC4_WT0CCP0);

    GPIOPinTypeTimer(GPIO_PORTC_BASE, GPIO_PIN_5);
    GPIOPinTypeTimer(GPIO_PORTC_BASE, GPIO_PIN_4);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_WTIMER0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_WTIMER0);

    SysCtlDelay(3);
    TimerConfigure(WTIMER0_BASE, TIMER_CFG_SPLIT_PAIR|TIMER_CFG_B_PWM|TIMER_CFG_A_PWM);
    TimerLoadSet(WTIMER0_BASE, TIMER_B, Period-1);
    TimerMatchSet(WTIMER0_BASE, TIMER_B, Period-1);
    TimerEnable(WTIMER0_BASE, TIMER_B);

    TimerLoadSet(WTIMER0_BASE, TIMER_A, Period-1);
    TimerMatchSet(WTIMER0_BASE, TIMER_A, Period-1);
    TimerEnable(WTIMER0_BASE, TIMER_A);


}



int main()
{
    char resultStr[16]  ="";
    uint32_t current,leftScore=0,rightScore=0, ballPos;
    int i;
    int totalScore;
    int direction;
    uint32_t reflexArray[6] = {200,400,600,800,1000,1200};
    uint32_t angleArray[6] = {90,120,160,200,240,270};

    SysCtlClockSet(SYSCTL_SYSDIV_2_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
    initLCD();
    InitConsole();
    initServo();
    ButtonsInit();
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);

    //uint32_t current = GPIOPinRead(BUTTONS_GPIO_BASE, ALL_BUTTONS);


    printLCD("Welcome to 2 in 1");
    setCursorPositionLCD(1,0);
    printLCD("atari games!");

    setServoAngle(270, 0);
    delayMS(200);
    setServoAngle(270, 1);
    delayMS(200);
    setServoAngle(0, 0);
    delayMS(200);
    setServoAngle(0, 1);
    delayMS(200);
    setServoAngle(180, 0);
    delayMS(200);
    setServoAngle(180, 1);
    delayMS(200);

    SysCtlDelay(40000000);
    clearLCD();
    printLCD("Left: Pong");
    setCursorPositionLCD(1,0);
    printLCD("Right: Reflex");
    current = 17; // Both right and left are inactive

    while(current == 17 || current == 0){
        current = GPIOPinRead(BUTTONS_GPIO_BASE, ALL_BUTTONS);
    }

    clearLCD();

    // REFLEX GAME STARTS
    if(current == 16){ // Right, Reflex
        printLCD("Reflex");
        setCursorPositionLCD(1,0);
        printLCD("Press after led");
        SysCtlDelay(30000000);
        clearLCD();
        printLCD("Be rdy to start!");
        SysCtlDelay(30000000);
        clearLCD();
        printLCD("Go!");
        // Game initialization
        totalScore = 6;
        setServoAngle(10,0);
        setServoAngle(10, 1);
        current = 17; // Both right and left are inactive
        while(totalScore != 0){
            while(current == 17 || current == 0){
                GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 4);
                delayMS(reflexArray[totalScore-1]);
                GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0);
                current = GPIOPinRead(BUTTONS_GPIO_BASE, ALL_BUTTONS);
                delayMS(reflexArray[totalScore-1]);
            }
            // Update winner score
            if(current == 16){ // Right
                rightScore++;
                setServoAngle(angleArray[rightScore], 1);
            }else{
                leftScore++;
                setServoAngle(angleArray[leftScore], 0);
            }
            totalScore--;
            current=17;
            clearLCD();
            if(totalScore){
                printLCD("Be rdy to start!");
                SysCtlDelay(30000000);
                clearLCD();
                printLCD("Another round!");
                delayMS(500);
            }

        }

        if(leftScore > rightScore)
            printLCD("Player L Wins!");
        else if(rightScore > leftScore)
            printLCD("Player R Wins!");
        else
            printLCD("Even!");
        delayMS(4000);
        clearLCD();
        printLCD("Restart for");
        setCursorPositionLCD(1,0);
        printLCD("another game!");


        // PONG GAME STARTS
    }else{  // Left, Pong
        printLCD("Pong");
        setCursorPositionLCD(1,0);
        printLCD("Press on edges");
        SysCtlDelay(30000000);
        clearLCD();
        printLCD("Be ready to start!");
        SysCtlDelay(30000000);
        clearLCD();
        // Game initialization
        totalScore = 6;
        setServoAngle(0,0);
        setServoAngle(0, 1);
        current = 17; // Both right and left are inactive
        while(totalScore != 0){
            ballPos = 7;
            direction = 1;
            while(direction){
                for(i=0;i<16;++i)
                    resultStr[i] = '.';
                resultStr[ballPos] = 'o';

                clearLCD();
                printLCD(resultStr);
                delayMS(130);
                // Edge check
                if(ballPos == 0 || ballPos == 15){
                    current = GPIOPinRead(BUTTONS_GPIO_BASE, ALL_BUTTONS);
                    if( ballPos == 15 && direction == 1 && current == 16){ // Right
                        direction=-1;
                    }else if( ballPos == 0 && direction == -1 && current == 1){ // Left
                        direction=1;
                    }else if(ballPos == 15 && direction == 1 && current != 16){
                        direction=0;
                        leftScore++;
                        setServoAngle(angleArray[leftScore], 0);
                    }if( ballPos == 0 && direction == -1 && current != 1){
                        direction=0;
                        rightScore++;
                        setServoAngle(angleArray[rightScore], 1);
                    }else;

                }
                ballPos += direction;
                delayMS(80);
            }
            totalScore--;
        }
        clearLCD();
        if(leftScore > rightScore)
            printLCD("Player L Wins!");
        else if(rightScore > leftScore)
            printLCD("Player R Wins!");
        else
            printLCD("Even!");
        delayMS(4000);
        clearLCD();
        printLCD("Restart for");
        setCursorPositionLCD(1,0);
        printLCD("another game!");



    }
    while(1);


}
