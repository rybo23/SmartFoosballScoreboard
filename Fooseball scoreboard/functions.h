/*
 * functions.h
 *
 *  Created on: Jul 14, 2025
 *  Author: Ryan Boyle
 */

#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_

typedef enum {
    START,
    PLAYING,
    RESET,
    STARTSINGLEPLAYERMODE,
    SINGLEPLAYERMODE,
    RESETSINGLEPLAYERMODE
} GameState_t;                           //For meaningful state names

extern GameState_t state;

unsigned char LeftButtonPress(void);
unsigned char RightButtonPress(void);
unsigned char ResetButtonPress(void);
unsigned char ResetButtonHold(void);
unsigned char LgoalSensor(void);
unsigned char RgoalSensor(void);
unsigned char checkScoresSP(void);
unsigned char blinkSinglePlayerModeIndic(void);
unsigned char blinkTwoPlayerModeIndic(void);
void configPorts(void);
void buzzScore(void);
void singlePlayerModeComp(void);
void shiftOut(uint8_t data);
void updateScores(int leftscore, int rightscore);
void checkScores(void);
void configTimerA0();
void configTimerA1 ();
void configTimerA2();
extern const uint8_t segmentMap[11];
extern volatile unsigned char halfSecondFlag;
extern unsigned char compScoring;

#endif



