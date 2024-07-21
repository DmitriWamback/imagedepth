#include <stdio.h>
#include <string.h>
#include <stdbool.h>

void printline() {
    printf("------------------------------------------\n");
}

int main() {

    char name[120];
    printline(); printline();
    int attempts = 1;

    while (true) {

        printf("Insert Key: ");
        fgets(name, 120, stdin);
        if (strcmp("]H(WIO|_))W(--0-0{W{D]]]][]pkOWADOPHPWAD4923hd93w-1pqlOHO-0-\n", name) == 0) { printf("Access Granted.\n"); break; }
        printf("Invalid Key. %i attempts remaining...\n", 10 - attempts);
        printline();
        attempts++;
        if (attempts == 11) {
            printf("Attempted login failiure, exiting program.");
            return -1;
        }
    }

    printline(); printline();
}