#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>

#define color_green "\x1b[32m"
#define color_red "\x1b[31m"
#define color_blue "\x1b[34m"
#define color_orange "\x1b[33m"
#define color_reset "\x1b[0m"

const char *word, *category;
char guess[100];
char letter;
int wrongGuessCount = 0;
int points = 0;

// Function to display Tom chasing Jerry
void chaseJerry(int num) {
    int tomPos = num;
    int jerryPos = 5;
    char line[7];
    
    for (int i = 0; i < 6; i++) {
        if (i == tomPos && i != jerryPos) {
            line[i] = 'T';
        } else {
            line[i] = (i < tomPos || i > jerryPos) ? ' ' : '-';
        }
    }
    
    if (tomPos == jerryPos) {
        printf("%sTom%s chased %sJerry%s!\n", color_blue, color_reset, color_orange, color_reset);
        line[jerryPos] = 'T';
    } else {
        printf("%sTom%s is chasing %sJerry%s!\n", color_blue, color_reset, color_orange, color_reset);
        line[jerryPos] = 'J';
    }
    
    line[6] = '\0';
    printf("%s\n", line);
}

// Function to set the initial guess word
void setWord() {
    printf("CATEGORY: %s\n", category);
    for (int i = 0; i < strlen(word); i++) {
        if (word[i] == ' ') {
            guess[i] = ' ';
            printf("%c", guess[i]);
        } else {
            guess[i] = '_';
            printf("%c", guess[i]);
        }
    }
    printf("\n");
}

// Function to print the current guess word
void printWord() {
    printf("\nCATEGORY: %s\n", category);
    for (int i = 0; i < strlen(word); i++) {
        printf("%c", guess[i]);
    }
    printf("\n");
}

// Function to check if the game is won or lost
int checkWinner() {
    return (strcmp(guess, word) == 0 && wrongGuessCount != 5) || (strcmp(guess, word) != 0 && wrongGuessCount == 5);
}

// Function to handle player's turn
int takeTurn(char l) {
    int letterFound = 0;
    for (int i = 0; i < strlen(word); i++) {
        if (tolower(l) == tolower(word[i])) {
            guess[i] = word[i];
            letterFound = 1;
        }
    }
    if (!letterFound) {
        wrongGuessCount++;
        printf("\nTries left: %d\n\n", 5 - wrongGuessCount);
        chaseJerry(wrongGuessCount);
    }
    return letterFound;
}

// Function to handle errors and exit
void die_with_error(char *error_msg) {
    printf("%s", error_msg);
    exit(-1);
}

int main(int argc, char *argv[]) {
    int client_sock, port_no, n;
    struct sockaddr_in server_addr;
    struct hostent *server;
    char buffer[256];

    // Check for correct usage
    if (argc < 3) {
        printf("Usage: %s hostname port_no", argv[0]);
        exit(1);
    }

    // Create a socket
    client_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (client_sock < 0)
        die_with_error("Error: socket() Failed.");

    // Get server details
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        die_with_error("Error: No such host.");
    }

    // Fill server address structure
    port_no = atoi(argv[2]);
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
         (char *)&server_addr.sin_addr.s_addr,
         server->h_length);

    server_addr.sin_port = htons(port_no);

    // Connect to the server
    if (connect(client_sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
        die_with_error("Error: connect() Failed.");

    char playAgain = 'y';

    while (tolower(playAgain) == 'y') {
        // Initialize game variables
        wrongGuessCount = 0;
        memset(guess, 0, sizeof(guess));

        // Receive category, word, and points from server
        char letter[256];

        // Receive word, category, and points from the server
        bzero(letter, 256);
        n = recv(client_sock, letter, 255, 0);
        if (n < 0) die_with_error("Error: recv() Failed.");

        printf("\n----- Points: %d -----\n\n", points);

        // Parse the category, word, and points
        char *token = strtok(letter, ";");
        if (token != NULL) {
            category = strdup(token);
            token = strtok(NULL, ";");
            if (token != NULL) {
                word = strdup(token);
                token = strtok(NULL, ";");
                if (token != NULL) {
                    points = atoi(token);
                }
            }
        }

        setWord();
        printf("\nTries left: %d\n\n", 5 - wrongGuessCount);
        chaseJerry(wrongGuessCount);

        while (1) {
            // Receive letter from teammate
            bzero(letter, 256);
            puts("\nYour teammate's turn.\n");
            n = recv(client_sock, letter, 255, 0);
            if (n < 0) die_with_error("Error: recv() Failed.");
            takeTurn(letter[0]);
            system("clear");
            printWord();
            printf("\nTries left: %d\n\n", 5 - wrongGuessCount);
            chaseJerry(wrongGuessCount);

            if (checkWinner()) {
                if ((wrongGuessCount == 5) && strcmp(guess, word) != 0) {
                    printf("\n%sGAME OVER!\n%sYou failed to guess %s%s%s.\n", color_red, color_reset, color_red, word, color_reset);
                } else {
                    printf("\n%sAMAZING TEAMWORK!\n%sThe word is %s%s%s.\n", color_green, color_reset, color_green, word, color_reset);
                    points += 100; // Increment points on correct guess
                }
                break;
            }

            // Player's turn
            printf("\nEnter a letter: ");
            bzero(letter, 256);
            fgets(letter, 255, stdin);

            if (!isalpha(tolower(letter[0]))) {
                system("clear");
                wrongGuessCount++;
                printf("\nTries left: %d\n\n", 5 - wrongGuessCount);
                chaseJerry(wrongGuessCount);
                printWord();
            } else {
                system("clear");
                takeTurn(letter[0]);
            }

            // Send letter to server
            n = send(client_sock, letter, 1, 0);
            if (n < 0) die_with_error("Error: send() Failed");

            system("clear");
            printWord();
            printf("\nTries left: %d\n\n", 5 - wrongGuessCount);
            chaseJerry(wrongGuessCount);

            if (checkWinner()) {
                if (wrongGuessCount == 5) {
                    printf("\n%sGAME OVER!\n%sYou failed to guess %s%s%s.\n\n", color_red, color_reset, color_red, word, color_reset);
                } else {
                    printf("\n%sAMAZING TEAMWORK!\n%sThe word is %s%s%s.\n\n", color_green, color_reset, color_green, word, color_reset);
                    points += 100; // Increment points on correct guess
                }
                break;
            }
        }

        if (wrongGuessCount == 5) {
            // Reset points to 0 if player loses
            points = 0;
            printf("\nDo you want to play again? (y/n): ");
            fgets(buffer, 255, stdin);
            playAgain = tolower(buffer[0]);

            // Send play again status to server
            n = send(client_sock, &playAgain, 1, 0);
            if (n < 0) die_with_error("Error: send() Failed");

            // Receive response from server
            bzero(buffer, 256);
            n = recv(client_sock, buffer, 255, 0);
            if (n < 0) die_with_error("Error: recv() Failed");
            char serverResponse = tolower(buffer[0]);

            // Check if either player wants to end the game
            if (playAgain != 'y' || serverResponse != 'y') {
                char goodbye_msg[] = "\nThank you for playing.\nGoodbye!";
                printf("%s\n", goodbye_msg);  // Print goodbye message to client console
                break;
            }

            // If both want to play again, continue the game logic here
            printf("\nBoth players want to continue. Starting new round...\n");
        }
    }

    close(client_sock);

    return 0;
}

