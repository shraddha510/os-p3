#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "btree.h"

static BTree currentTree; // The currently open B-Tree

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

// Function to get yes/no response from user
static int getYesNo(const char *prompt)
{
    char response;
    printf("%s (y/n): ", prompt);
    scanf(" %c", &response);
    clearInputBuffer();
    return (response == 'y' || response == 'Y');
}

// Function to handle creating a new B-Tree file
static void createTree()
{
    char filename[256];
    printf("Enter filename to create: ");
    if (fgets(filename, sizeof(filename), stdin))
    {
        filename[strcspn(filename, "\n")] = 0; // Remove newline

        // Check if file exists
        FILE *test = fopen(filename, "rb");
        if (test)
        {
            fclose(test);
            if (!getYesNo("File exists. Overwrite?"))
            {
                printf("Operation cancelled.\n");
                return;
            }
        }

        // Close current tree if open
        if (currentTree.is_open)
        {
            close_btree(&currentTree);
        }

        // Create new tree
        if (create_btree(&currentTree, filename) == 0)
        {
            printf("B-Tree file created successfully.\n");
        }
        else
        {
            printf("Error creating B-Tree file.\n");
        }
    }
}

// Function to handle opening an existing B-Tree file
static void openTree()
{
    char filename[256];
    printf("Enter filename to open: ");
    if (fgets(filename, sizeof(filename), stdin))
    {
        filename[strcspn(filename, "\n")] = 0;

        if (currentTree.is_open)
        {
            close_btree(&currentTree);
        }

        if (open_btree(&currentTree, filename) == 0)
        {
            printf("B-Tree file opened successfully.\n");
        }
        else
        {
            printf("Error opening file. Check if file exists and is valid.\n");
        }
    }
}

// Function to handle inserting a key-value pair
static void insertPair()
{
    if (!currentTree.is_open)
    {
        printf("Error: No index file is currently open.\n");
        return;
    }

    uint64_t key, value;
    printf("Enter key (unsigned integer): ");
    if (scanf("%llu", &key) != 1)
    {
        printf("Invalid key format.\n");
        clearInputBuffer();
        return;
    }

    printf("Enter value (unsigned integer): ");
    if (scanf("%llu", &value) != 1)
    {
        printf("Invalid value format.\n");
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    if (insert_key(&currentTree, key, value) == 0)
    {
        printf("Key-value pair inserted successfully.\n");
    }
    else
    {
        printf("Error: Key already exists or insertion failed.\n");
    }
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
            if (strcmp(choice, "1") == 0 || strcmp(choice, "create") == 0)
            {
                // put function here
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