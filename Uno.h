#pragma once
#include <stdlib.h>
#include "GlorvNet.h"
//#include "UnoMPMain.c"
#define DECKLENGTH 108//108 normal, 76 numbers only, 100 no wilds
#define NUMBERDEALT 1
#define SHUFFLETHOROUGHNESS 5000
#define MAXNUMBEROFPLAYERS 5


struct cardData {//Holds the data of each card
	char data[2];//Holds the data of a single card
};
typedef struct cardData card;

struct player {
	card hand[DECKLENGTH];
	card data;
	int handsize;
	char playerName[DSIZE];//Player name, means nothing really just for reference
	GSock playerSockets;
};
typedef struct player Player;



void clientCardPlayed(GSock* ourSock, int pNum, int pHS, card played, char* name);
void preTurn(GSock* ourSock, Player* players, int ourNumber, int playerCount);

card drawCard(card* data, card deck[DECKLENGTH]);
void handDrawCard(Player* data, card deck[DECKLENGTH]);

void displayOthersCount(Player* players, int playercount);
int legalPlay(Player player, card cardInPlay, card cardToPlay);


card clientTurnFromHost(Player player, card cardInPlay, GSock* playerSocket, int drawTwod, int drawFourd, int skipped);

void playClient(int port, char* address);
int clienting(GSock* ourSocket);
void removeCardFromHand(Player *player, card removedCard);

void directionUpdate(int* direction, char type);

void dataShifter(card input, card* writeto);//Easy way of slapping the data from one card entry into another
void printDeck(card deck[DECKLENGTH]);//Prints the deck, debug feature
void print(card deck);//Prints the card (Less typing in code lol)
void detailedPrint(card deck);//Prints the card readibly, will print Colour then Number, or just Wild or Wild Pluf Four
void clearscreen();//'Clears' the screen
int updateCurrentPlayer(int currentPlayer, int direction, int playerCount);//Returns the new current player value
void intToPrint(int input, int capitalized);//Takes the input int and prints it.