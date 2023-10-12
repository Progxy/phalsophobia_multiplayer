#include <stdio.h>
#include "utils.h"
#include "network.h"

int main() {
    // Regex to clear the terminal.
    printf("\e[1;1H\e[2J");

    char confirm;
    printf("\x1b[1;35m\n------------- WELCOME TO PHALSOPHOBIA -------------\n\x1b[1;0m");
    printf("\nThe rules are simple:\n");
    printf("\nTo win the game you need to collect in the caravan,\nall the three different types of evidence: [EMF, SPIRIT_BOX and CAMERA].\n");
    printf("\nBut be careful to not let your mental health decrease to zero!");
    printf("\nOtherwise you will be eliminated!");
    printf("\n\n");
    printf("Said that, good luck!");
    printf("\x1b[1;33m\n\nPress ENTER to continue: \x1b[1;0m");
    scanf("%c", &confirm);

    // Start the game lobby
    if (!startGame()) {
        return 1;
    }

    return 0;
}
