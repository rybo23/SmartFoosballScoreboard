/**********************************************************************************************
 * Project:      Foosball Table Scoreboard with a Smart Single Player Mode
 *
 * Description:  This is a self-directed project to design and implement a digital score board
 *               system with support for two player and single player game modes.
 *               The two player game mode represents a digital version of typical game
 *               play with the additional ability to increment score using buttons,
 *               reset button functionality, and photo electric sensors for goal
 *               detection. The single player mode uses the left button and
 *               photoelectric sensor to increment the user's score. The right button
 *               is used to begin computer scoring, in which a real opponent's scoring
 *               is simulated. The first to 10 wins. 
 *  
 * Platform:     MSP430F5529
 * Author:       Ryan Boyle
 * Date:         July–August 2025
 ***********************************************************************************************/


#include <msp430.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "functions.h"



//global variables
volatile unsigned int score_playerLeft = 0;
volatile unsigned int score_playerRight = 0;
volatile unsigned int msTicks = 0;
volatile unsigned int sTicks = 0;
volatile unsigned int HsTicks = 0;



const uint8_t segmentMap[11] = {
//G F E D C B A DP
    0b01111110,  // 0: A B C D E F
    0b00001100,  // 1: B C
    0b10110110,  // 2: A B D E G
    0b10011110,  // 3: A B C D G
    0b11001100,  // 4: B C F G
    0b11011010,  // 5: A C D F G
    0b11111010,  // 6: A C D E F G
    0b00001110,  // 7: A B C
    0b11111110,  // 8: A B C D E F G
    0b11011111,  // 9: DP A B C D F G
    0b00000000   //10: all are off
};


GameState_t state = START;

int main(void){

    WDTCTL = WDTPW | WDTHOLD;            //Stop watchdog timer

    configPorts();                       //Configure ports
    configTimerA0();                     //Sets up timer A0 (ms), A1 (.5 s), A2 (s)
    configTimerA1();
    configTimerA2();
    srand(sTicks);                       //Seed random number generator
    __enable_interrupt();                //Necessary for clock usage





        while (1){                       //Infinite loop

            switch(state){

            case START:

               updateScores(score_playerLeft, score_playerRight);           //Calls function passing score 0-0 to shiftOut function where its loaded into shift registers and handles latching
               state = PLAYING;
                break;


            case PLAYING:

                if (LeftButtonPress()) {
                    score_playerLeft++;                                     //Left score goes up 1
                    checkScores();                                          //Sets the scores back to zero after maxing out the displays
                    updateScores(score_playerLeft, score_playerRight);      //Updates scores with increased (+1) left score
                }

                if (RightButtonPress()) {
                    score_playerRight++;                                    //Right score board goes up 1
                    checkScores();                                          //Sets the scores back to zero after maxing out the displays
                    updateScores(score_playerLeft, score_playerRight);      //Updates scores with increased (+1) right score
                }

               if (ResetButtonPress()) {
                    state = RESET;                                          //RESET state takes the program back to the START state after resetting score keeping variables to 0-0
                }

               if (ResetButtonHold()) {                                     //Handles mode change
                   state = RESETSINGLEPLAYERMODE;

               }

               if (LgoalSensor()) {
                   score_playerLeft++;                                     //Left score goes up 1
                   checkScores();                                          //Sets the scores back to zero after maxing out the displays
                   updateScores(score_playerLeft, score_playerRight);      //Updates scores with increased (+1) left score
                   buzzScore();                                            //Buzzer for score indication
               }

               if (RgoalSensor()) {
                   score_playerRight++;                                     //Right score goes up 1
                   checkScores();                                           //Sets the scores back to zero after maxing out the displays
                   updateScores(score_playerLeft, score_playerRight);       //Updates scores with increased (+1) right score
                   buzzScore();                                             //Buzzer for score indication
               }


               break;

            case RESET:

                score_playerLeft = 0;                //Resets scores to zero to bring back to beginning state of the game
                score_playerRight = 0;
                state = START;                       //Back to START

               break;


            case STARTSINGLEPLAYERMODE:

                updateScores(score_playerLeft, score_playerRight);     //Calls function passing score 0-0 to shiftOut function where its loaded into shift registers and handles latching
                state = SINGLEPLAYERMODE;

                break;


            case  SINGLEPLAYERMODE:

                singlePlayerModeComp();                               //Code for single player mode is stored here - sensor, buttons, reset, buzzer, and return to two player mode

                break;


            case RESETSINGLEPLAYERMODE:

               blinkSinglePlayerModeIndic();                       //Blinks "1" on both score boards to indicate to the user that they are now in single player mode
               score_playerLeft = 0;                               //Scores set to zero for restart
               score_playerRight = 0;
               compScoring = 0;                                    //Stops computer scoring
                                                                   //Also resets compScoring back to zero to require a new right button press to ensue scoring from the computer

               state = STARTSINGLEPLAYERMODE;                      //Updates scores and starts the game

               break;
            }

        }

      return 0;

}





