#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "btree.h"

static void printMenu()
{
    printf("\nB-Tree Index Manager\n");
    printf("===================\n");
    printf("1. create  - Create a new index file\n");
    printf("2. open    - Open an existing index file\n");
    printf("3. insert  - Insert a key-value pair\n");
    printf("4. search  - Search for a key\n");
    printf("5. load    - Load pairs from file\n");
    printf("6. print   - Print all pairs\n");
    printf("7. extract - Extract pairs to file\n");
    printf("8. quit    - Exit program\n");
}

int main()
{
    char choice[10];
    int running = 1;

    printf("Welcome to B-Tree Index Manager\n");
    printf("Type 'menu' to see available commands\n");

    while (running)
    {
        printf("\n> ");
        if (fgets(choice, sizeof(choice), stdin))
        {
            choice[strcspn(choice, "\n")] = 0; // Remove newline

            // Convert choice to lowercase for case-insensitive comparison
            for (char *p = choice; *p; p++)
            {
                *p = tolower(*p);
            }

            if (strcmp(choice, "menu") == 0 || strcmp(choice, "help") == 0)
            {
                printMenu();
            }
            else if (strcmp(choice, "8") == 0 || strcmp(choice, "quit") == 0)
            {
                running = 0;
            }
            else
            {
                printf("Unknown command. Type 'menu' to see available commands.\n");
            }
        }
    }
    printf("Goodbye!\n");
    return 0;
}