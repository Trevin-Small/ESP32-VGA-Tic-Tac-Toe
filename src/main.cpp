#include <ESP32Lib.h>
#include <Ressources/Font6x8.h>
#include <string>
#include <stdlib.h>

//pin configuration
const int redPin = 14;
const int greenPin = 19;
const int bluePin = 27;
const int hsyncPin = 0;
const int vsyncPin = 5;

//VGA Device
VGA3Bit vga;

char board[4][4] = {'\0'};
const int width = 200;
const int height = 150;
const int boardSize = 60;
int numTurns = 0;
bool playerOneTurn = true;

void resetBoard() {
	int counter = 1;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			board[i][j] = char (48 + counter);
			counter++;
		}
	}
}

void drawGameBoard() {

	// Horizontal board lines
	vga.line(width / 2 - (boardSize / 2), height / 2 - (boardSize / 6), width / 2 + (boardSize / 2), height / 2 - (boardSize / 6), 0b111);
	vga.line(width / 2 - (boardSize / 2), height / 2 + (boardSize / 6), width / 2 + (boardSize / 2), height / 2 + (boardSize / 6), 0b111);

	// Vertical board lines
	vga.line(width / 2 - (boardSize / 6), height / 2 - (boardSize / 2), width / 2 - (boardSize / 6), height / 2 + (boardSize / 2), 0b111);
	vga.line(width / 2 + (boardSize / 6), height / 2 - (boardSize / 2), width / 2 + (boardSize / 6), height / 2 + (boardSize / 2), 0b111);

	// Draw X's and O's
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			vga.setCursor( (width / 2) - (boardSize / 2) + i * 20 + 8, (height / 2) - (boardSize / 2) + j * 20 + 8);
			vga.print(board[j][i]);
		}
	}
}

int checkWinState() {


	for (int i = 0; i < 3; i++) {
		if (board[i][0] == board[i][1] && board[i][1] == board[i][2]) {
		return board[i][0] == 'X' ? 1 : 2;
		}
	}

	for (int i = 0; i < 3; i++) {
		if (board[0][i] == board[1][i] && board[1][i] == board[2][i]) {
		return board[0][i] == 'X' ? 1 : 2;
		}
	}

	if (board[0][0] == board[1][1] && board[1][1] == board[2][2]) {
		return board[0][0] == 'X' ? 1 : 2;
	}

	if (board[0][3] == board[1][1] && board[1][1] == board[2][0]) {
		return board[0][0] == 'X' ? 1 : 2;
	}

	if (numTurns == 9) {
		return -1;
	}

	return 0;
}

void handleWin(int winner) {
	if (winner == 0) {
		return;
	} else if (winner == -1) {

			char winnerBuf[12] = {'I', 'T', '\'', 'S', ' ', 'A', ' ', 'T', 'I', 'E', '!', '\0'};

			drawGameBoard();

			vga.setCursor(width / 2 - 26, 10);
			vga.print("GAME OVER");
			vga.setCursor(width / 2 - 34, 110);
			vga.print(winnerBuf);

	} else {

		char winnerBuf[15] = {'P', 'L', 'A', 'Y', 'E', 'R', ' ', ' ', ' ', 'W', 'I', 'N', 'S', '!', '\0'};
		winnerBuf[7] = char (48 + winner);

		drawGameBoard();

		vga.setCursor(width / 2 - 26, 10);
		vga.print("GAME OVER");
		vga.setCursor(width / 2 - 40, 110);
		vga.print(winnerBuf);

	}

	vga.setCursor(width / 2 - 56, 130);
	vga.print("PRESS ENTER TO RESTART");
	vga.show();

	while(!Serial.available()){ delay(10); }

	resetBoard();
	numTurns = 0;
	playerOneTurn = true;
}

void setup()
{
  Serial.begin(115200);

	//we need double buffering for smooth animations
	vga.setFrameBufferCount(2);

	//initializing i2s vga (with only one framebuffer)
	vga.init(vga.MODE200x150, redPin, greenPin, bluePin, hsyncPin, vsyncPin);

	//setting the font
	vga.setFont(Font6x8);

	Serial.println("ESP32 VGA TIC TAC TOE");

	resetBoard();
}

//just draw each frame
void loop()
{

	// Process input and update board state
  if (Serial.available()) {
		String data = Serial.readStringUntil('\n');

		Serial.println(data);

		if (data.length() == 1 && data[0] >= 49 && data[0] <= 57) {

			char input[2];
			data.toCharArray(input, 2);

			int moveX = (input[0] - 49) / 3;
			int moveY = (input[0] - 49) % 3;

			if (board[moveX][moveY] == 'X' || board[moveX][moveY] == 'O') {
				Serial.print("Invalid input. ");
				Serial.print(input[0]);
				Serial.println(" is already taken. Choose an open space.");
			} else {

				if (playerOneTurn) {
					board[moveX][moveY] = 'X';
				} else {
					board[moveX][moveY] = 'O';
				}

				playerOneTurn = !playerOneTurn;
				numTurns++;
				Serial.print("Turns: ");
				Serial.println(numTurns);

				handleWin(checkWinState());
			}

		} else {
			Serial.println("Invalid input. Enter a number 1-9 corresponding to an open space.");
		}

		if (playerOneTurn) {
			Serial.print("Player one - choose: ");
		} else {
			Serial.print("Player two - choose: ");
		}

  }

	// Draw title
	vga.setCursor(width / 2 - 30, 10);
	vga.print("TIC TAC TOE");

	drawGameBoard();

	//show the rendering
	vga.show();

	// Delay 2 ms
	delay(2);

	// Clear the screen
	vga.clear();
}