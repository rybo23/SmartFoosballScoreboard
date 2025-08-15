/*
 * functions.c
 *
 *  Created on: Jul 14, 2025
 *      Author: Ryan B
 */

#include <msp430.h>
#include <msp430.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "functions.h"

#define DS BIT3                      //"Data" P3.3
#define SHCP BIT1                    //"Shift Clock Pin" P3.1
#define LTCH BIT2                    //"Latch Clock" P3.2
#define DEBOUNCE_TIME 100            //100ms de-bounce time for buttons
#define DEBOUNCE_TIMESENSOR 1        //1s "de-bounce" time for photoelectric sensors
#define HOLD_TIME 2                  //2s required hold time of the reset button
#define SINGLE_PLAYER_LED_TOGGLE 10  //10 half second or 5 second duration for LED toggle
#define DIFFICULTY 5                 //DIFFICULTY adds to min and max duration
#define SPREAD 11                    //SPREAD | 11 gives numbers 0-10
#define BUZZER_DURATION 1            //1 half second buzzer duration

//Declarations can be found at the top of main
extern volatile unsigned int score_playerLeft;
extern volatile unsigned int score_playerRight;
extern volatile unsigned int msTicks;
extern volatile unsigned int sTicks;
extern volatile unsigned int HsTicks;


volatile unsigned char halfSecondFlag = 0;       //Used for blinking 1 during the single player mode indication
unsigned char compScoring = 0;                   //Used for single player mode start button logic

void configPorts(void) {

    //Shift register pin configuration
    P3DIR = P3DIR | (LTCH | SHCP | DS);          //x111x - pins set to 1 for direction - OUTPUT
    P3OUT = P3OUT & ~(LTCH | SHCP | DS);         //x000x - pins set to 0 to ensure ther're off initially

    //Button configuration | Left 6.0 Right 6.1 Reset 6.5
    P6DIR = P6DIR & ~(BIT5 | BIT1 | BIT0);       //0xxx00 - pins set to 0 for direction - INPUT
    P6REN = P6REN | (BIT5 | BIT1 | BIT0);        //1xxx11 - pins set to 1 - enables resistors
    P6OUT = P6OUT | (BIT5 | BIT1 | BIT0);        //1xxx11 - pins set to 1 for PULL UP resistors

    //Buzzer configuration P6.4
    P6DIR = P6DIR | (BIT4);                      //x1xxxx - pin 4 set to 1 for direction - OUTPUT
    P6OUT = P6OUT & ~(BIT4);                     //x0xxxx - Sets Pin 4 to 0 to ensure it is off initially


    //Photoelectric sensor pin configuration | Left P4.6, Right 4.3
    P4DIR = P4DIR & ~(BIT6 | BIT3);              //0xx0xxx - pins set to 0 for direction - Input
    P4REN = P4REN | (BIT6 | BIT3);               //1xx1xxx - pins set to 1 - enables resistors
    P4OUT = P4OUT | (BIT6 | BIT3);               //1xx1xxx - pins set to 1 for PULL UP resistors


}



unsigned char LeftButtonPress(void) {
    static int lastStateL = 0;
    int currentStateL = 0;
    static unsigned int lastPressTime = 0;  //Static allows the variables to be remembered across function calls and only sets it to zero on the first call

    if (!(P6IN & BIT0)) {                   //Button is pressed (active low) / P6.0 is 0
        currentStateL = 1;                  //Current state of the left button is pressed
        } else {
        currentStateL = 0;
        }



    if (currentStateL == 1 && lastStateL == 0) {           //Detect rising edge: 0 (of the previous state) to 1 (current)
        if ((msTicks - lastPressTime) > DEBOUNCE_TIME) {   //Checks for intentional press by checking that the press time in Ms is greater than the desired de-bounce time
            lastPressTime = msTicks;                       //Records the time of this press
            lastStateL = currentStateL;                    //Update to current state (pressed)
        return 1;                                          //The button was pressed
      }
    }
    lastStateL = currentStateL;                            //Update to current state (not pressed)
    return 0;                                              //No new press detected
}

unsigned char LgoalSensor(void){
        static int lastStateL = 0;
        int currentStateL = 0;
        static unsigned int lastTriggerTime = 0;


        if (!(P4IN & BIT1)) {      //Sensor is activated (active low) / P4.1 is 0
            currentStateL = 1;     //Current state of the left sensor is triggered
        } else {
            currentStateL = 0;     //Current state of the left sensor is not triggered
        }


        if (currentStateL == 1 && lastStateL == 0) {                //Detect rising edge: 0 (of the previous state) to 1 (current)
            if ((sTicks - lastTriggerTime) > DEBOUNCE_TIMESENSOR) {
                lastTriggerTime = sTicks;                           //Records the time of this trigger
                lastStateL = currentStateL;                         //Update to current state (triggered)
                return 1;                                           //The sensor was triggered
                }
        }
            lastStateL = currentStateL;                             //Update to current state (not triggered)
            return 0;                                               //No new activation detected
    }


unsigned char RightButtonPress(void){
    static int lastStateR = 0;
    int currentStateR = 0;
    static unsigned int lastPressTime = 0;


       if (!(P6IN & BIT1)) {       //Button is pressed (active low) / P6.1 is 0
           currentStateR = 1;      //Current state of the right button is pressed
       } else {
           currentStateR = 0;      //Current state of the button is un-pressed
       }

                                                                   //Detect rising edge: 0 (of the previous state) to 1 (current)
       if (currentStateR == 1 && lastStateR == 0) {                //Checks for intentional press by checking that the press time in Ms is greater than the desired de-bounce time
           if ((msTicks - lastPressTime) > DEBOUNCE_TIME) {
               lastPressTime = msTicks;                            //Records the time of this press
               lastStateR = currentStateR;                         //Update to current state (pressed)
               return 1;                                           //The button was recently pressed intentionally
                     }
       }
           lastStateR = currentStateR;                             //Update to current state (not pressed)
           return 0;                                               //No new press detected
   }

unsigned char RgoalSensor(void){
    static int lastStateR = 0;
    int currentStateR = 0;
    static unsigned int lastPressTime = 0;

            if (!(P4IN & BIT3)) {         //Sensor is activated (active low) / P4.3 is 0
                currentStateR = 1;        //Current state of the right sensor is pressed
            }else{
                currentStateR = 0;
            }

                                                                       //Detect rising edge: 0 (of the previous state) to 1 (current)
            if (currentStateR == 1 && lastStateR == 0) {
                if ((sTicks - lastPressTime) > DEBOUNCE_TIMESENSOR) {
                    lastPressTime = sTicks;                            //Keeps track of the last trigger time so sTicks can be measured from it
                    lastStateR = currentStateR;                        //Update to current state (triggered)
                    return 1;                                          //The sensor was activated
                    }
            }
            lastStateR = currentStateR;                                //Update to current state (not triggered)
            return 0;                                                  //No new press detected
}

unsigned char ResetButtonPress(void){
    static int lastStateRe = 0;
    int currentStateRe = 0;
    static unsigned int lastPressTime = 0;


    if (!(P6IN & BIT5)) {                                      //Button is pressed (active low) / P6.5 is 0
        if ((msTicks - lastPressTime) > DEBOUNCE_TIME) {       //checks for intentional press by checking that the press time in Ms is greater than the desired de-bounce time
                currentStateRe = 1;                            //current state of the reset button is pressed
                lastPressTime = msTicks;                       //Keeps track of the last press time so msTicks can be measured from it
            }
        }
        if (currentStateRe == 1 && lastStateRe == 0){          //Detect rising edge: 0 (of the previous state) to 1 (current)
            lastStateRe = currentStateRe;                      //Keeps track of recent press
            return 1;                                          //The button was recently pressed intentionally

        }

        lastStateRe = currentStateRe;                          //Update to current state
        return 0;                                              //No new press detected
}

unsigned char ResetButtonHold(void){
    static int holding = 0;
    static int pressStartTime = 0;
    if (!(P6IN & BIT5)) {                                        //Button is pressed (active low)
          if (holding == 0){                                     //Not already timing a press
              pressStartTime = sTicks;                           //pressStartTime = sTicks so the number of seconds the button is being held for can be tracked
              holding = 1;                                       //Hold is now underway
          } else if ((sTicks - pressStartTime) > HOLD_TIME) {    //Checks for hold press by checking that the press time in s is greater than the required hold time for mode change
              return 1;                                          //The button has been held for the required amount of time
          }
    } else {
        holding = 0;                                             //Holding goes back to 0 when the button gets released
    }

  return 0;
}

void buzzScore(void) {
    unsigned int BuzzerStartTime = HsTicks;                  //Records start time for check
    while ((HsTicks - BuzzerStartTime) <= BUZZER_DURATION) { //Turns the buzzer on if the time the buzzer has been on is less than the desired time
        P6OUT = P6OUT | BIT4;                                //Turns buzzer on | x1xxxx
    }
    P6OUT = P6OUT & ~(BIT4);                                 //Turns buzzer off |x0xxxx
}

unsigned char blinkTwoPlayerModeIndic(void){
    unsigned int SinglePlayerStartTime = HsTicks;
    unsigned int LastFlag = halfSecondFlag;

    while ((HsTicks - SinglePlayerStartTime) <= SINGLE_PLAYER_LED_TOGGLE) {       //Executes the following code if the time in half seconds is less then the chosen toggle duration
            if (halfSecondFlag != LastFlag) {                                     //Checks for half second pass / halfSecondFlag has toggled
                LastFlag = halfSecondFlag;                                        //Update last flag to current flag for future edge detection

                if (halfSecondFlag == 1) {                                        //Displays 2 to the seven segment displays every half second (flag is toggled in ISR)
                    updateScores(2, 2);
                } else {
                    updateScores(10, 10);                                         //Turns seven segments off for blinking indication
                }
            }
        }
            return 0;
            }

unsigned char blinkSinglePlayerModeIndic(void){
    unsigned int SinglePlayerStartTime = HsTicks;
    unsigned int LastFlag = halfSecondFlag;

    while ((HsTicks - SinglePlayerStartTime) <= SINGLE_PLAYER_LED_TOGGLE) {       //Executes the following code if the time in half seconds is less then the chosen toggle duration
            if (halfSecondFlag != LastFlag) {                                     //Checks for half second pass / halfSecondFlag has toggled
                LastFlag = halfSecondFlag;                                        //Update last flag to current flag for future edge detection

                if (halfSecondFlag == 1) {                                        //Displays 1 to the seven segment displays every half second (flag is toggled in ISR)
                    updateScores(1, 1);
                } else {
                    updateScores(10, 10);                                         //Turns seven segments off for blinking indication
                }
            }
        }
            return 0;
            }

void singlePlayerModeComp(void){
    static unsigned int SinglePlayerCompStartTime = 0;
    static unsigned int CompScoreDuration = 0;


         if(RightButtonPress() && compScoring == 0){                               //Starts computer scoring if right button is pressed for the first time
                 SinglePlayerCompStartTime = sTicks;                               //Records start time
                 CompScoreDuration = (rand() % SPREAD) + DIFFICULTY;               //Gives a number within a range and adds a number to adjust difficulty (adds a number to min and max)
                 compScoring = 1;                                                  //Computer scoring is active - allows for one time computer score initialization
              }
         if(compScoring){                                                          //Executes following code if the computer scoring has already begun
             if((sTicks - SinglePlayerCompStartTime) >= CompScoreDuration){        //Checks to see if the amount of seconds passed has reached the randomly generated duration for the computer to score
                 score_playerRight++;                                              //Right score increases by 1 once the randomly generated duration for the computer to score has been reached
                 if (checkScoresSP()) {                                            //Checks to see if either score has reached 10
                    state = RESETSINGLEPLAYERMODE;                                 //Resets single player mode back to 0-0 and stops computer scoring

                 }

                 updateScores(score_playerLeft, score_playerRight);
                 buzzScore();                                                      //Buzzer for score indication
                 SinglePlayerCompStartTime = sTicks;                               //Records start time for next check
                 CompScoreDuration = (rand() % SPREAD) + DIFFICULTY;               //Gives a number within a range and adds a number to adjust difficulty (adds to min and max)


              }
        }


             if (LgoalSensor()) {
                 score_playerLeft++;                                     //Left score board goes up 1
                 if (checkScoresSP()) {                                  //Resets single player mode if either score reaches 10
                    state = RESETSINGLEPLAYERMODE;
                 }
                 updateScores(score_playerLeft, score_playerRight);      //Updates scores with increased (+1) left score
                 buzzScore();                                            //Buzzer for score indication
              }

             if (LeftButtonPress()) {
                 score_playerLeft++;                                     //left score board goes up 1
                 if (checkScoresSP()) {                                  //Resets single player mode if either score reaches 10
                    state = RESETSINGLEPLAYERMODE;
                 }
                 updateScores(score_playerLeft, score_playerRight);      //Updates scores with increased (+1) left score
              }


             if(ResetButtonHold()){
                 blinkTwoPlayerModeIndic();                              //Blinks "2" on each seven segment to indicate transition back to 2 player mode
                 state = RESET;                                          //RESET is for the two player mode, so it works to start the 2 player mode
             }

             if (ResetButtonPress()) {
                score_playerLeft = 0;                                    //Scores set to zero for restart
                score_playerRight = 0;
                compScoring = 0;                                         //Stops computer scoring
                updateScores(score_playerLeft, score_playerRight);       //Updates scores to 0-0

                            }
      }


void updateScores(int leftScore, int rightScore) {       //Takes the decimal scores as input

    P3OUT = P3OUT & ~BIT2;               //Latch set LOW (prepare to send data)

    shiftOut(segmentMap[rightScore]);    //Sends right digit binary value first (chained second)
    shiftOut(segmentMap[leftScore]);     //Sends left digit segments second

    P3OUT |= BIT2;                       //Latch HIGH (outputs update)
    P3OUT &= ~BIT2;                      //Latch LOW (ready for next)
}




void shiftOut(uint8_t data) {        //Takes in the binary sequence from the 'segmentMap'
       int i;
       for  (i = 0; i < 8; i++) {    //Iterates this section of code 8 times (8 bits in data)
                                     //*Sends MSB first*
           if (data & 0x80){         //0x80 - 128 in decimal - checks bit 7 (MSB) 10000000
               P3OUT = P3OUT | DS;   //Sets the data pin high (1)
           }
           else {
               P3OUT &= ~DS;           //Sets the data pin low (0)
           }
           P3OUT = P3OUT | BIT1;       //SHCP HIGH
           P3OUT = P3OUT & ~BIT1;      //SHCP reset
           data <<= 1;                 //Shifts data bits one position to the left to check the next most significant bit
       }
    }



void checkScores(){
    if (score_playerLeft > 9) score_playerLeft = 0;                   //Sets the scores back to zero after maxing out the displays
    if (score_playerRight > 9) score_playerRight = 0;
}

unsigned char checkScoresSP(){
    if (score_playerLeft >= 10 || score_playerRight >= 10){           //Returns 1 if either score reaches 10 - used in single player mode
       return 1;
    }
    return 0;
}


void configTimerA0 (){                            //1 ms clock
      TA0CCTL0 = CCIE;                            //Enables interrupt on TA0CCR0 / enables interrupt on capture compare register 0 of Timer A0
      TA0CCR0 = 1048 - 1;                         //x / 1.048 MHz = .001 --- x = 1048
      TA0CTL = TASSEL_2 | ID_0 | MC_1 | TACLR;    //Uses SMCLK, Sets divider to 1, Up mode, clear timer upon config
}

void configTimerA1 (){                            //.5 sec clock
      TA1CCTL0 = CCIE;                            //Enables interrupt on TA1CCR0 / enables interrupt on capture compare register 0 of Timer A1
      TA1CCR0 = 65500 - 1;                        //x / 1.048 MHz = .5 --- x = 524000  | /8 = 65500
      TA1CTL = TASSEL_2 | ID_3 | MC_1 | TACLR;    //Uses SMCLK, Sets divider to 8, Up mode, clear timer upon config
}



void configTimerA2(){                            //1s clock
    TA2CCTL0 = CCIE;                             //Enables interrupt on CCR0
    TA2CCR0 = 16375 - 1;                         //x/ 1.048 Mhz = 1 --- x = 1,048,000 | /64 = 16375
    TA2EX0 = TAIDEX_7;                           //Extra /8 (TAIDEX_7 = divide by 8) | so divider is effectively 64
    TA2CTL = TASSEL_2 | ID_3 | MC_1 | TACLR;     //Uses SMCLK, /8, up mode, clear timer
}


#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer0_A0_ISR(void) {
    msTicks++;                                   //Every millisecond the ISR runs and adds one millisecond to the ms count
}

#pragma vector=TIMER1_A0_VECTOR
__interrupt void Timer1_A0_ISR(void) {
        halfSecondFlag = halfSecondFlag ^ 1;     //XOR equals toggles HalfSecondFlag
        HsTicks++;                               //Every half second the ISR runs and adds one half second to the half second count


}


#pragma vector=TIMER2_A0_VECTOR
__interrupt void Timer2_A0_ISR(void) {
    sTicks++;                                   //Every second the ISR runs and adds one second to the s count



}
