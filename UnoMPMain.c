#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <time.h>
#include "Uno.h"
#include "GlorvNet.h"

#define PORT 80

int dataCorruption = 0;//Global flag for marking if there has been data corruption.
int currentTopdeck = 0;



FILE* debugDump;

void main() {
	startup();
	time_t t;
	srand((unsigned)time(&t));
	//Throughout the game variables

	//create the deck variable
	card deck[DECKLENGTH + 2];//The array that contains the deck
	int playerCount = 1;//How many players
	int gameRunning = 1;//Is the game set to continue in the loop after this one finishes?
	int AIOnly = 0;//Did the player make it an AI only game?
	card cardInPlay;//The current card in play
	cardInPlay.data[0] = 'E';
	cardInPlay.data[1] = 'E';
	Player players[MAXNUMBEROFPLAYERS];
	//End of mane variables


	do {//yay scope
		char userin[50];
		printf("Do you want to host a game? Y/N\n");
		gets_s(&userin, 10);
		if (userin[0] == 'N') {
			printf("Give the hosts IP address:   ");
			gets_s(&userin, 30);
			playClient(80, userin);//gtfo out of here
			return;
		}
	} while (0);


	//NETWORKING OPENING
	
	printf("\n\n\nWhat do you want to call yourself?\n");
	gets_s(&players[0].playerName, DSIZE);

	GSock listenSock;
	GSock tempsock;
	createSock(&listenSock, 80, INADDR_ANY);
	glisten(&listenSock);
	
	while (1) {
		players[playerCount].playerSockets.dataSize = DSIZE;
		gaccept(&listenSock, &players[playerCount].playerSockets);
		grecv(&players[playerCount].playerSockets);
		for (int currentChar = 0; currentChar < DSIZE - 1; currentChar++) {
			players[playerCount].playerName[currentChar] = players[playerCount].playerSockets.data[currentChar];
		}
		printf("\n\nPlayer \"%s\" has joined. %d players in total. Are you waiting on more? Y/N\n", players[playerCount].playerName, playerCount);
		memmove(&players[playerCount].playerSockets.data, players[0].playerName, strlen(&players[0].playerName) + 1);
		gsend(&players[playerCount].playerSockets);

		char tempin[10];
		playerCount++;
		gets_s(&tempin, 10);
		if (tempin[0] == 'N') {
			break;
		}
		if (playerCount == MAXNUMBEROFPLAYERS) {
			printf("Max number of players reached.\n\n");
			break;
		}
	}
	

	for (int cur = 1; cur < playerCount; cur++) {
		//No actual data needs to be sent, they're just waiting on anything
		gsend(&players[cur].playerSockets);
	}
	//END OF NETWORKING OPENING
	


	//_________________________________________
	//PRE GAME SETUP
	//_________________________________________


	//load from file because legacy code copying makes it easier to select a random card
	FILE* deckfile;
	deckfile = fopen("Deck.txt", "r");
	for (int fill = 0; fill < DECKLENGTH; fill++) {
		fscanf(deckfile, "%s", &deck[fill].data);
	}


	//deal cards
	for (int currentPlayer = 0; currentPlayer < playerCount; currentPlayer++) {
		players[currentPlayer].handsize = 0;
		for (int currentCard = 0; currentCard < NUMBERDEALT; currentCard++) {
			 handDrawCard(&players[currentPlayer], deck);
		}
		
	}

	do {
		drawCard(&cardInPlay, deck);//Put a card in play
	} while (cardInPlay.data[1] == 'w');
	


	//yeet the hands to the clients so that they can display them



//_________________________________________
//END OF PRE GAME SETUP
//_________________________________________



//game loop
	int currentPlayer = 0;//Keeps track of the current player
	int direction = 1;//Keeps track of which direction and whether to increase or decrease the current player at the end of each players turn, 1 is normal -1 is antinormal.
	int drawTwod = 0;
	int skipped = 0;
	int drawFourd = 0;


	while (gameRunning == 1) {


		if (currentPlayer == 0) {
			//hosts turn
			printf("\n\n\n\n");
			if (skipped == 1) {//Did we get skipped?
				//networking
				printf("You were skipped\n");
				currentPlayer = updateCurrentPlayer(currentPlayer, direction, playerCount);
				skipped = 0;
				printf("\n\n\n\n");
			}
			else if (drawTwod == 1) {//Did we get plus 2?
				printf("You were Draw Twoed\n");
				handDrawCard(&players[0], deck);
				handDrawCard(&players[0], deck);
				currentPlayer = updateCurrentPlayer(currentPlayer, direction, playerCount);
				drawTwod = 0;
				printf("\n\n\n\n");
			}
			else if (drawFourd == 1) {
				printf("You were Draw Foured\n");
				handDrawCard(&players[0], deck);
				handDrawCard(&players[0], deck);
				handDrawCard(&players[0], deck);
				handDrawCard(&players[0], deck);
				currentPlayer = updateCurrentPlayer(currentPlayer, direction, playerCount);//End of turn, move on
				drawFourd = 0;
				printf("\n\n\n\n");
			}
			else {//normal turn
				printf("Card in play: ");
				detailedPrint(cardInPlay);
				printf("\nYour Hand:");
				for (int currentC = 0; currentC < players[0].handsize; currentC++) {
					detailedPrint(players[0].hand[currentC]);
					printf("   ");
				}
				printf("\n");

				//Get the host to play a card
				while (1) {
					char input[30];
					printf("What card would you like to play?\n");
					gets_s(&input, 30);
					card userInput;
					userInput.data[0] = input[0];
					userInput.data[1] = input[1];

					if (userInput.data[0] == 'D') {
						handDrawCard(&players[0], deck);
						break;
					}
					else if (userInput.data[0] == 'E') {//e for end game
						return;
					}
					else {
						if (legalPlay(players[0], cardInPlay, userInput) ){
							directionUpdate(&direction, userInput.data[0]);
							removeCardFromHand(&players[0], userInput);
							dataShifter(userInput, &cardInPlay);
							if (userInput.data[0] == 'S') {
								skipped = 1;
							}
							else if (userInput.data[0] == 'T') {
								drawTwod = 1;
							}
							else if (userInput.data[0] == 'F') {
								drawFourd = 1;
							}
							break;
						}
						else {
							printf("Illegal play, either you do not have the card or it's not playable.\n");
						}
					}
				}

				currentPlayer = updateCurrentPlayer(currentPlayer, direction, playerCount);//End of turn, move on
			}

			for (int cp = 1; cp < playerCount; cp++) {
				clientCardPlayed(&players[cp].playerSockets, currentPlayer - 1, players[currentPlayer - 1].handsize, cardInPlay, players[currentPlayer - 1].playerName);
			}
			printf("\n\n\n\n");
		}//end of hosts turn


		//host won
		if (players[0].handsize == 0) {
			printf("Congrats, you have won the game!\n");
			printf("Enter a victory message: ");
			char message[DSIZE];
			gets_s(&message, DSIZE - 1);
			for (int cp = 1; cp < playerCount; cp++) {
				memmove(&players[cp].playerSockets.data, &message, DSIZE - 1);
				memmove(&players[cp].playerSockets.data[strlen(&message) + 1], &players[0].playerName, strlen(&players[0].playerName) + 1);
				players[cp].playerSockets.data[DSIZE - 1] = 3;
				gsend(&players[cp].playerSockets);
			}
			printf("Enter anything to exit.");
			gets_s(&message, DSIZE - 1);
			return;
		}
		
		//check to see if any clients won
		for (int cp = 1; cp < playerCount; cp++) {
			if (players[cp].handsize == 0) {
				players[cp].playerSockets.data[DSIZE - 1] = 4;
				gsend(&players[cp].playerSockets);
				grecv(&players[cp].playerSockets);
				char message[DSIZE];
				memmove(&message, &players[cp].playerSockets.data, DSIZE - 1);
				clearscreen();
				printf("%s has won the game!\n", players[cp].playerName);
				printf("%s", message);
				for (int currentP = 1; currentP < playerCount; currentP++) {
					if (currentP != cp) {
						memmove(&players[currentP].playerSockets.data, &message, DSIZE - 1);
						memmove(&players[currentP].playerSockets.data[strlen(&message)+1], &players[cp].playerName,strlen(&players[cp].playerName) + 1);
						players[currentP].playerSockets.data[DSIZE - 1] = 3;
						gsend(&players[currentP].playerSockets);
					}
				}
				return;
			}
		}

		//clients turns
		if (currentPlayer != 0){
			preTurn(&players[currentPlayer].playerSockets, &players, currentPlayer, playerCount);
			card playedCard = clientTurnFromHost(players[currentPlayer], cardInPlay, &players[currentPlayer].playerSockets, drawTwod, drawFourd, skipped);
			if (drawTwod) {
				handDrawCard(&players[currentPlayer], deck);
				handDrawCard(&players[currentPlayer], deck);
				printf("Player #%d, %s was plus two'd and has %d cards.\n", currentPlayer, players[currentPlayer].playerName, players[currentPlayer].handsize);
				drawTwod = 0;
				printf("The current card in play is a ");
				detailedPrint(cardInPlay);
				printf("\n\n\n\n");
			}
			else if (drawFourd) {
				handDrawCard(&players[currentPlayer], deck);
				handDrawCard(&players[currentPlayer], deck);
				handDrawCard(&players[currentPlayer], deck);
				handDrawCard(&players[currentPlayer], deck);
				printf("Player #%d, %s was plus four'd and has %d cards.\n", currentPlayer, players[currentPlayer].playerName, players[currentPlayer].handsize);
				drawFourd = 0;
				printf("The current card in play is a ");
				detailedPrint(cardInPlay);
				printf("\n\n\n\n");
			}
			else if (skipped) {
				printf("Player #%d, %s was skipped and has %d cards.\n", currentPlayer, players[currentPlayer].playerName, players[currentPlayer].handsize);
				skipped = 0;
				printf("The current card in play is a ");
				detailedPrint(cardInPlay);
				printf("\n\n\n\n");
			}
			else {
				skipped = 0;
				drawTwod = 0;
				drawFourd = 0;
				if (players[currentPlayer].playerSockets.data[2] == 0) {//if they actually played we check their cards and stuff
					dataShifter(playedCard, &cardInPlay);//well, they played the card
					removeCardFromHand(&players[currentPlayer], playedCard);
					printf("Player #%d, %s played a ", currentPlayer, players[currentPlayer].playerName);
					detailedPrint(playedCard);
					printf(" and has %d cards.\n", players[currentPlayer].handsize);
					directionUpdate(&direction, playedCard.data[0]);
					if (playedCard.data[0] == 'S') {
						skipped = 1;
					}
					else if (playedCard.data[0] == 'T') {
						drawTwod = 1;
					}
					else if (playedCard.data[0] == 'F') {
						drawFourd = 1;
					}
				}
				else{
					handDrawCard(&players[currentPlayer], deck);
					printf("Player #%d, %s drew and has %d cards.\n", currentPlayer, players[currentPlayer].playerName, players[currentPlayer].handsize);
				}
				printf("The current card in play is a ");
				detailedPrint(cardInPlay);
				printf("\n\n\n\n");
			}

			for (int cp = 1; cp < playerCount; cp++) {
				if (cp != currentPlayer) {
					clientCardPlayed(&players[cp].playerSockets, currentPlayer, players[currentPlayer].handsize, cardInPlay, players[currentPlayer].playerName);
				}
			}
			currentPlayer = updateCurrentPlayer(currentPlayer, direction, playerCount);
		}

		//check to see if any clients won
		for (int cp = 1; cp < playerCount; cp++) {
			if (players[cp].handsize == 0) {
				players[cp].playerSockets.data[DSIZE - 1] = 4;
				gsend(&players[cp].playerSockets);
				grecv(&players[cp].playerSockets);
				char message[DSIZE];
				memmove(&message, &players[cp].playerSockets.data, DSIZE - 1);
				clearscreen();
				printf("%s has won the game!\n", players[cp].playerName);
				printf("%s", message);
				for (int currentP = 1; currentP < playerCount; currentP++) {
					if (currentP != cp) {
						memmove(&players[currentP].playerSockets.data, &message, DSIZE - 1);
						memmove(&players[currentP].playerSockets.data[strlen(&message) + 1], &players[cp].playerName, strlen(&players[cp].playerName) + 1);
						players[currentP].playerSockets.data[DSIZE - 1] = 3;
						gsend(&players[currentP].playerSockets);
					}
				}
				return;
			}
		}

	}//END OF LOOP


	printf("\nEnter anything to end ");
	scanf("%d", &currentPlayer);
}

