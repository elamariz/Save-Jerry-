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

// Function to select a word from the word bank based on category
void wordBank() {
    char *food[] = {"pizza", "chocolate", "hotdog", "chicken", "spaghetti", "carbonara", "bread", "burger", "fries", "egg"};
    char *animals[] = {"cat", "lion", "snake", "dog", "shark", "rabbit", "tiger", "monkey", "bear", "pig", "penguin", "giraffe"};
    char *colors[] = {"red", "green", "blue", "pink", "violet", "black", "white", "indigo", "yellow", "orange", "fucshia"};
    char *AdNU[] = {"Dolan", "Phelan", "Adriatico", "Santos", "Xavier", "Arrupe", "Burns", "Alingal", "Bonoan"};
    char *fastfood[] = {"Jollibee", "Mcdonalds", "KFC", "Wendys", "Chowking", "Greenwich", "Starbucks"};
    
    int numFoodWords = sizeof(food) / sizeof(food[0]);
    int numAnimalWords = sizeof(animals) / sizeof(animals[0]);
    int numColorWords = sizeof(colors) / sizeof(colors[0]);
    int numAdNUWords = sizeof(AdNU) / sizeof(AdNU[0]);
    int numFastfoodWords = sizeof(fastfood) / sizeof(fastfood[0]);

    srand(time(0));
    int r = rand() % 5;

    if (r == 0) {
        word = food[rand() % numFoodWords];
        category = "Food";
    } else if (r == 1) {
        word = animals[rand() % numAnimalWords];
        category = "Animal";
    } else if (r == 2){
        word = colors[rand() % numColorWords];
        category = "Color";
    } else if (r == 3){
        word = AdNU[rand() % numAdNUWords];
        category = "AdNU Building";
    } else {
        word = fastfood[rand() % numFastfoodWords];
        category = "Fastfood Chain";
    }
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
    int server_sock, client_sock, port_no, client_size, n;
    char buffer[256];
    struct sockaddr_in server_addr, client_addr;

    // Check for correct usage
    if (argc < 2) {
        printf("Usage: %s port_no", argv[0]);
        exit(1);
    }

    // Create a socket for incoming connections
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0)
        die_with_error("Error: socket() Failed.");

    // Bind socket to a port
    bzero((char *) &server_addr, sizeof(server_addr));
    port_no = atoi(argv[1]);
    server_addr.sin_family = AF_INET; // Internet address
    server_addr.sin_addr.s_addr = INADDR_ANY; // Any incoming interface
    server_addr.sin_port = htons(port_no); // Local port

    if (bind(server_sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
        die_with_error("Error: bind() Failed.");

    // Mark the socket so it will listen for incoming connections
    listen(server_sock, 5);

    // Accept new connection
    client_size = sizeof(client_addr);
    client_sock = accept(server_sock, (struct sockaddr *) &client_addr, &client_size);
    if (client_sock < 0) die_with_error("Error: accept() Failed.");

    char playAgain = 'y';

    while (tolower(playAgain) == 'y') {
        // Initialize game variables
        wrongGuessCount = 0;
        memset(guess, 0, sizeof(guess));

        // Choose a word from the word bank
        wordBank();

        // Send category, word, and points to client
        snprintf(buffer, sizeof(buffer), "%s;%s;%d", category, word, points);
        n = send(client_sock, buffer, strlen(buffer), 0);
        if (n < 0) die_with_error("Error: send() failed");

        printf("\n----- Points: %d -----\n\n", points);
        setWord();
        printf("\nTries left: %d\n\n", 5 - wrongGuessCount);
        chaseJerry(wrongGuessCount);

        while (1) {
            // Player's turn
            printf("\nEnter a letter: ");
            bzero(buffer, 256);
            fgets(buffer, 255, stdin);

            if (!isalpha(tolower(buffer[0]))) {
                system("clear");
                wrongGuessCount++;
                printf("\nTries left: %d\n\n", 5 - wrongGuessCount);
                chaseJerry(wrongGuessCount);
                printWord();
            } else {
                system("clear");
                takeTurn(buffer[0]);
            }

            // Send letter to client
            n = send(client_sock, buffer, 1, 0);
            if (n < 0) die_with_error("Error: send() Failed");

            system("clear");
            printWord();
            printf("\nTries left: %d\n\n", 5 - wrongGuessCount);
            chaseJerry(wrongGuessCount);

            // Check if game is won or lost
            if (checkWinner()) {
                if ((wrongGuessCount == 5) && strcmp(guess, word) != 0) {
                    printf("\n%sGAME OVER!\n%sYou failed to guess %s%s%s.\n", color_red, color_reset, color_red, word, color_reset);
                } else {
                    printf("\n%sAMAZING TEAMWORK!\n%sThe word is %s%s%s.\n", color_green, color_reset, color_green, word, color_reset);
                    points += 100; // Increment points on correct guess
                }
                break;
            }

            // Receive letter from teammate
            bzero(buffer, 256);
            puts("\nYour teammate's turn.\n");
            n = recv(client_sock, buffer, 255, 0);
            if (n < 0) die_with_error("Error: recv() Failed.");
            takeTurn(buffer[0]);
            system("clear");
            printWord();
            printf("\nTries left: %d\n\n", 5 - wrongGuessCount);
            chaseJerry(wrongGuessCount);

            // Check if game is won or lost
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

            // Send play again status to client
            n = send(client_sock, &playAgain, 1, 0);
            if (n < 0) die_with_error("Error: send() Failed");

            // Receive response from client
            bzero(buffer, 256);
            n = recv(client_sock, buffer, 255, 0);
            if (n < 0) die_with_error("Error: recv() Failed");
            char clientResponse = tolower(buffer[0]);

            // Check if either player wants to end the game
            if (playAgain != 'y' || clientResponse != 'y') {
                char goodbye_msg[] = "\nThank you for playing.\nGoodbye!";
                printf("%s\n", goodbye_msg);  // Print goodbye message to server console
                n = send(client_sock, goodbye_msg, strlen(goodbye_msg), 0);
                if (n < 0) die_with_error("Error: send() Failed");
                break;
            }

            // If both want to play again, continue the game logic here
            printf("\nBoth players want to continue. Starting new round...\n");
        }
    }

    close(client_sock);
    close(server_sock);

    return 0;
}

