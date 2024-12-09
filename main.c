// main.c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "btree.h"

static BTree currentTree; // The currently open B-Tree

// Function to clear input buffer
static void clearInputBuffer()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

// Function to print menu options
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

// Function to handle inserting each key-value pair into the tree
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

// Function to handle searching for a key
static void searchForKey()
{
    if (!currentTree.is_open)
    {
        printf("Error: No index file is currently open.\n");
        return;
    }

    uint64_t key, value;
    printf("Enter key to search: ");
    if (scanf("%llu", &key) != 1)
    {
        printf("Invalid key format.\n");
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    if (search_key(&currentTree, key, &value) == 0)
    {
        printf("Found: Key = %llu, Value = %llu\n",
               (unsigned long long)key, (unsigned long long)value);
    }
    else
    {
        printf("Key not found.\n");
    }
}

// Function to handle loading data from a file
static void loadFromFile()
{
    if (!currentTree.is_open)
    {
        printf("Error: No index file is currently open.\n");
        return;
    }

    char filename[256];
    printf("Enter filename to load from: ");
    if (fgets(filename, sizeof(filename), stdin))
    {
        filename[strcspn(filename, "\n")] = 0;

        if (load_data(&currentTree, filename) == 0)
        {
            printf("Data loaded successfully.\n");
        }
        else
        {
            printf("Error loading data from file.\n");
        }
    }
}

// Function to handle extracting data to a file
static void extractToFile()
{
    if (!currentTree.is_open)
    {
        printf("Error: No index file is currently open.\n");
        return;
    }

    char filename[256];
    printf("Enter filename to extract to: ");
    if (fgets(filename, sizeof(filename), stdin))
    {
        filename[strcspn(filename, "\n")] = 0;

        FILE *test = fopen(filename, "r");
        if (test)
        {
            fclose(test);
            if (!getYesNo("File exists. Overwrite?"))
            {
                printf("Operation cancelled.\n");
                return;
            }
        }

        if (extract_data(&currentTree, filename) == 0)
        {
            printf("Data extracted successfully.\n");
        }
        else
        {
            printf("Error extracting data to file.\n");
        }
    }
}

// main function that controls the flow
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
            else if (strcmp(choice, "1") == 0 || strcmp(choice, "create") == 0)
            {
                createTree();
            }
            else if (strcmp(choice, "2") == 0 || strcmp(choice, "open") == 0)
            {
                openTree();
            }
            else if (strcmp(choice, "3") == 0 || strcmp(choice, "insert") == 0)
            {
                insertPair();
            }
            else if (strcmp(choice, "4") == 0 || strcmp(choice, "search") == 0)
            {
                searchForKey();
            }
            else if (strcmp(choice, "5") == 0 || strcmp(choice, "load") == 0)
            {
                loadFromFile();
            }
            else if (strcmp(choice, "6") == 0 || strcmp(choice, "print") == 0)
            {
                if (!currentTree.is_open)
                {
                    printf("Error: No index file is currently open.\n");
                }
                else
                {
                    print_tree(&currentTree);
                }
            }
            else if (strcmp(choice, "7") == 0 || strcmp(choice, "extract") == 0)
            {
                extractToFile();
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

    if (currentTree.is_open)
    {
        close_btree(&currentTree);
    }
    printf("Goodbye!\n");
    return 0;
}