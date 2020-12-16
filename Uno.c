#include "Uno.h"



card clientTurnFromHost(Player player, card cardInPlay, GSock* playerSocket, int drawTwod, int drawFourd, int skipped) {
	playerSocket->data[0] = cardInPlay.data[0];
	playerSocket->data[1] = cardInPlay.data[1];
	playerSocket->data[2] = player.handsize;
	playerSocket->data[3] = (drawTwod) | (drawFourd * 2) | (skipped * 3);


	for (int currentCard = 0; currentCard < player.handsize; currentCard++) {
		playerSocket->data[currentCard * 2 + 4] = player.hand[currentCard].data[0];
		playerSocket->data[currentCard * 2 + 5] = player.hand[currentCard].data[1];
	}
	playerSocket->data[DSIZE - 1] = 0;

	gsend(playerSocket);
	grecv(playerSocket);


	if (playerSocket->data[2] == 0) {
		card playedCard;
		playedCard.data[0] = playerSocket->data[0];
		playedCard.data[1] = playerSocket->data[1];
		return(playedCard);
	}
	else {
		card playedCard;
		playedCard.data[0] = 0;
		playedCard.data[1] = 0;
		return(playedCard);
	}
}

void clientCardPlayed(GSock* ourSock, int pNum, int pHS, card played, char* name) {
	ourSock->data[0] = pNum;
	ourSock->data[1] = pHS;
	ourSock->data[2] = played.data[0];
	ourSock->data[3] = played.data[1];
	memmove(&ourSock->data[4], name, strlen(name));
	ourSock->data[4 + strlen(name)] = '\0';//fuuuuuuuuuuuuuuu
	ourSock->data[DSIZE - 1] = 2;
	gsend(ourSock);
}

void preTurn(GSock* ourSock, Player* players, int ourNumber, int playerCount) {
	ourSock->data[0] = ourNumber;
	ourSock->data[1] = playerCount;
	int offset = 2;

	for (int current = 0; current < playerCount; current++) {
		if (current != ourNumber) {
			int nameLength = strlen(players[current].playerName);
			ourSock->data[offset] = players[current].handsize;
			ourSock->data[offset + 1] = nameLength + 1;
			memmove(&ourSock->data[offset + 2], players[current].playerName, nameLength);
			offset += 2 + nameLength + 1;
			ourSock->data[offset - 1] = '\0';
		}
	}
	ourSock->data[DSIZE - 1] = 1;
	gsend(ourSock);
}

void playClient(int port, char* address) {
	GSock ourSocket;
	ourSocket.dataSize = DSIZE;
	createSock(&ourSocket, port, address);
	printf("Connecting.\n");
	int connecting = gconnect(&ourSocket);
	if (connecting == 0) {
		printf("Connection successful.\n\n\n\n");
	}
	else {
		printf("Error in connection. %d", connecting);
	}


	printf("What would you like to call yourself?     ");
	gets_s(&ourSocket.data, 50);
	gsend(&ourSocket);


	while (1) {
		grecv(&ourSocket);//receive our update
		if (ourSocket.data[DSIZE - 1] == 4) {//We won in glory with honor
			printf("Congrats, you have won the game!\n");
			printf("Enter a victory message: ");
			char message[DSIZE];
			gets_s(&message, DSIZE - 1);
			memmove(&ourSocket.data, message, DSIZE - 1);
			gsend(&ourSocket);
			return;
		}
		else if (ourSocket.data[DSIZE - 1] == 3) {//someone fucking asshole won
			printf("%s won the game!\n", &ourSocket.data[strlen(&ourSocket.data[0] + 1)]);
			printf("%s\n", &ourSocket.data[0]);
			return;
		}
		else if (ourSocket.data[DSIZE - 1] == 2) {//Someone played a card
			printf("Player #%d, %s just took their turn and has %d cards.\n", ourSocket.data[0], &ourSocket.data[4], ourSocket.data[1]);
			card temp;
			temp.data[0] = ourSocket.data[2];
			temp.data[1] = ourSocket.data[3];
			printf("The current card in play: ");
			detailedPrint(temp);
			printf("\n\n\n");
		}
		else if (ourSocket.data[DSIZE - 1] == 1) {//If last entry is 1, it's right before our turn
			printf("\n\n\n\n\nIt is your turn.\n");
			int cOff = 2;
			for (int cP = 0; cP < ourSocket.data[1]; cP++) {
				if (cP != ourSocket.data[0]) {//make sure it isn't us
					printf("Player #%d, %s has %d cards.\n", cP, &ourSocket.data[cOff + 2], ourSocket.data[cOff]);
					cOff += 2 + ourSocket.data[cOff + 1];
				}
			}
			printf("\n");
		}
		else if (ourSocket.data[DSIZE - 1] == 0) {//our turn
			clienting(&ourSocket);
		}
		else {
			printf("Packet corrupted. No solution currently in place. F");
		}
	}
}

int clienting(GSock* ourSocket) {
	card data;//card in play
	data.data[0] = ourSocket->data[0];
	data.data[1] = ourSocket->data[1];
	int skippage = ourSocket->data[3];
	if (skippage == 3) {
		printf("You have been skipped by a ");
		detailedPrint(data);
		printf("\n");
		ourSocket->data[2] = 1;
		gsend(ourSocket);
		printf("\n\n\n\n\n");
		return(-1);
	}
	else if (skippage == 1) {
		printf("You have been plus two'd by a ");
		detailedPrint(data);
		printf("\n");
		ourSocket->data[2] = 1;
		gsend(ourSocket);
		printf("\n\n\n\n\n");
		return(-1);
	}
	else if (skippage == 2) {
		printf("You have been plus four'd by a ");
		detailedPrint(data);
		printf("\n");
		ourSocket->data[2] = 1;
		gsend(ourSocket);
		printf("\n\n\n\n\n");
		return(-1);
	}
	else {
		//populate our hand since we're going to play
		Player ourHand;
		ourHand.handsize = ourSocket->data[2];
		for (int currentcard = 0; currentcard < ourHand.handsize; currentcard++) {
			ourHand.hand[currentcard].data[0] = ourSocket->data[(currentcard * 2) + 4];
			ourHand.hand[currentcard].data[1] = ourSocket->data[(currentcard * 2) + 5];
		}

		printf("Card in play: ");
		detailedPrint(data);
		printf("\nYour Hand:   ");
		for (int currentC = 0; currentC < ourHand.handsize; currentC++) {
			detailedPrint(ourHand.hand[currentC]);
			printf(",  ");
		}
		printf("\n");

		//Get the client to enter a card
		while (1) {
			char userin[3];
			card userInput;
			printf("What card would you like to play?\n");
			gets_s(&userin, 3);
			userInput.data[0] = userin[0];
			userInput.data[1] = userin[1];

			if (userInput.data[0] == 'D') {
				ourSocket->data[0] = 0;
				ourSocket->data[1] = 0;
				ourSocket->data[2] = -1;//we drew
				gsend(ourSocket);//send back our sorrow
				printf("\n\n\n\n\n");
				return(1);
			}
			else {
				if (legalPlay(ourHand, data, userInput)) {
					ourSocket->data[0] = userInput.data[0];
					ourSocket->data[1] = userInput.data[1];
					ourSocket->data[2] = 0;//we did in fact play
					gsend(ourSocket);//send back the card we want to play.
					printf("\n\n\n\n\n");
					break;//we've played so breakout now
				}
				else {
					printf("Illegal play, either you do not have the card or it's not playable.\n");
				}
			}
		}
	}
}



void removeCardFromHand(Player* player, card removedCard) {
	int current = 0;

	//ascertain the cards position in hand
	for (current = 0; current < player->handsize; current++) {
		if (player->hand[current].data[0] == removedCard.data[0] && player->hand[current].data[1] == removedCard.data[1]) {
			break;
		}
		if (player->hand[current].data[0] == removedCard.data[0] && player->hand[current].data[1] == 'w') {
			break;
		}
	}

	while (current < player->handsize - 1) {
		player->hand[current].data[0] = player->hand[current + 1].data[0];
		player->hand[current].data[1] = player->hand[current + 1].data[1];
		current++;
	}
	player->handsize--;
}

card drawCard(card* data, card deck[DECKLENGTH]) {
	int pos = rand() % DECKLENGTH;
	data[0].data[0] = deck[pos].data[0];
	data[0].data[1] = deck[pos].data[1];
}

void handDrawCard(Player* data, card deck[DECKLENGTH]) {
	int pos = rand() % DECKLENGTH;
	data->hand[data->handsize].data[0] = deck[pos].data[0];
	data->hand[data->handsize].data[1] = deck[pos].data[1];
	data->handsize++;
}

void directionUpdate(int* direction, char type) {
	if (type == 'R') {
		direction[0] *= -1;
	}
}

void displayOthersCount(Player* players, int playercount) {
	for (int counter = 0; counter < playercount; counter++) {
		printf("Player %s has %d cards.\n", players[counter].playerName, players[counter].handsize);
	}
}

int legalPlay(Player player, card cardInPlay, card cardToPlay) {
	for (int currentCard = 0; currentCard < player.handsize; currentCard++) {
		if ((player.hand[currentCard].data[0] == 'F' && player.hand[currentCard].data[0] == cardToPlay.data[0]) || (player.hand[currentCard].data[0] == 'W' && player.hand[currentCard].data[0] == cardToPlay.data[0])) {
			return(2);//It's a wild so we can play it always
		}
		else if (player.hand[currentCard].data[0] == cardToPlay.data[0] && player.hand[currentCard].data[1] == cardToPlay.data[1]) {
			if (cardToPlay.data[0] == cardInPlay.data[0] || cardToPlay.data[1] == cardInPlay.data[1]) {
				return(1);//legal play, we both have it and it matches the card in play somehow
			}
		}
	}
	return(0);
}









//Prints out the cards raw data
void print(card deck) {//Prints out the cards raw data
	printf("%c%c", deck.data[0], deck.data[1]);
}


//Prints the card readibly, will print Colour then Number, or just Wild or Wild Pluf Four
void detailedPrint(card deck) {// Will print Colour then Number, or just Wild or Wild Pluf Four
	switch (deck.data[1]) {
	case('r'):
		printf("Red ");
		break;
	case('b'):
		printf("Blue ");
		break;
	case('y'):
		printf("Yellow ");
		break;
	case('g'):
		printf("Green ");
		break;
	case('w'):
		//No printing, hand/start wilds handled purely in the other switch
		break;
	default:
		printf("Data corruption noticed when hitting: Detailed Print A");
	}

	switch (deck.data[0]) {
	case('0'):
		printf("Zero");
		break;
	case('1'):
		printf("One");
		break;
	case('2'):
		printf("Two");
		break;
	case('3'):
		printf("Three");
		break;
	case('4'):
		printf("Four");
		break;
	case('5'):
		printf("Five");
		break;
	case('6'):
		printf("Six");
		break;
	case('7'):
		printf("Seven");
		break;
	case('8'):
		printf("Eight");
		break;
	case('9'):
		printf("Nine");
		break;
	case('T'):
		printf("Plus Two");
		break;
	case('S'):
		printf("Skip");
		break;
	case('R'):
		printf("Reverse");
		break;
	case('F'):
		printf("Plus Four");
		break;
	case('W'):
		printf("Wild");
		break;
	default:
		printf("Data corruption noticed when hitting: Detailed Print B");
	}
}


//debug feature to print deck
void printDeck(card deck[DECKLENGTH]) {//debug feature to print deck
	for (int counter = 0; counter < DECKLENGTH; counter++) {
		printf("%s\n", deck[counter].data);
	}
}


//'Clears' the screen
void clearscreen() {
	printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
}

//Easy way of slapping the data from one card entry into another
void dataShifter(card input, card* writeto) {//Easy way of slapping the data from one card entry into another
	writeto->data[0] = input.data[0];
	writeto->data[1] = input.data[1];
}


//Returns the new current player value
int updateCurrentPlayer(int currentPlayer, int direction, int playerCount) {
	//printf("%d, %d, %d", currentPlayer, direction, playerCount);//DEBUG REMOVE

	//If its going normal
	if (direction == 1 && currentPlayer == playerCount - 1) {//and we are at the last player
		//printf("Go to zero\n");//DEBUG REMOVE
		return(0);//we go back to the first.
	}
	if (direction == 1 && currentPlayer < playerCount - 1) {//and we are not at the last player
		//printf("Add one\n");//DEBUG REMOVE
		currentPlayer++;
		return(currentPlayer);//we go to the next player.
	}



	//If its going antinormal
	if (direction == -1 && currentPlayer == 0) {//and we are at the first player
		//printf("Go to last\n");//DEBUG REMOVE
		return(playerCount - 1);//go to the last player.
	}
	if (direction == -1 && currentPlayer > 0) {//and we are not at the first player
		//printf("Go previous\n");//DEBUG REMOVE
		currentPlayer--;
		return(currentPlayer);//go back a player
	}
}


//Takes the input int and prints it.
void intToPrint(int input, int capitalized) {
	if (capitalized == 0) {
		if (input == 0) {
			printf("zero");
		}
		else if (input == 1) {
			printf("one");
		}
		else if (input == 2) {
			printf("two");
		}
		else if (input == 3) {
			printf("three");
		}
		else if (input == 4) {
			printf("four");
		}
		else if (input == 5) {
			printf("five");
		}
		else if (input == 6) {
			printf("six");
		}
		else if (input == 7) {
			printf("seven");
		}
		else if (input == 8) {
			printf("eight");
		}
		else if (input == 9) {
			printf("nine");
		}
	}
	else {
		if (input == 0) {
			printf("Zero");
		}
		else if (input == 1) {
			printf("One");
		}
		else if (input == 2) {
			printf("Two");
		}
		else if (input == 3) {
			printf("Three");
		}
		else if (input == 4) {
			printf("Four");
		}
		else if (input == 5) {
			printf("Five");
		}
		else if (input == 6) {
			printf("Six");
		}
		else if (input == 7) {
			printf("Seven");
		}
		else if (input == 8) {
			printf("Eight");
		}
		else if (input == 9) {
			printf("Nine");
		}
	}

}

//Takes the hand input and counts the # of cards of each colour, returning the colour with the most cards
char cardsOfColourCounter(Player player) {
	int red = 0;
	int blue = 0;
	int green = 0;
	int yellow = 0;
	for (int counter = 0; counter < player.handsize; counter++) {//Run through each card and count the colours
		if (player.hand[counter].data[1] == 'r') {
			red++;
		}
		else if (player.hand[counter].data[1] == 'b') {
			blue++;
		}
		else if (player.hand[counter].data[1] == 'g') {
			green++;
		}
		else if (player.hand[counter].data[1] == 'y') {
			yellow++;
		}
	}
	//Super mega high tech find the highest value code block, then return the one with the most
	if (red >= blue && red >= green && red >= yellow) {
		return('r');
	}
	else if (blue >= red && blue >= green && blue >= yellow) {
		return('b');
	}
	else if (green >= red && green >= blue && green >= yellow) {
		return('g');
	}
	else if (yellow >= red && yellow >= blue && yellow >= green) {
		return('y');
	}
	else {
		return('r');//Just make it red if theres literally no other colours in the deck (Only wilds for example, or the last card)
	}
}