//name : Emam Samara
//ID : 1220022
//section : 1

// i added this because i use windows on my pc and linux on my laptop
#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// i use both windows and linux so i kept these includes
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#define NAME_LEN 100
#define ADDRESS_LEN 150
#define PAID_LEN 8
#define LINE_LEN 512
#define PATH_LEN 4096
#define INFO_FILE "info.txt"
#define HASH_FILE "hash.txt"
#define MIN_HASH_TABLE_SIZE 11
#define MIN_YEAR 1800
#define MAX_YEAR 2026

typedef struct {
    char name[NAME_LEN];
    int number;
    char address[ADDRESS_LEN];
    int apartments;
    int year;
    char paid[PAID_LEN];
} Building;

typedef struct AVLNode {
    Building data;
    int height;
    struct AVLNode *left;
    struct AVLNode *right;
} AVLNode;

typedef enum {
    EMPTY,
    OCCUPIED,
    DELETED
} EntryStatus;

typedef struct {
    Building data;
    EntryStatus status;
} HashEntry;

typedef struct {
    HashEntry *entries;
    int size;
    int count;
    int loaded;
} HashTable;

static void removeNewline(char *str);
static void trimSpaces(char *str);
static int parseIntStrict(const char *text, int minValue, int maxValue, int *out);
static int readIntInRange(const char *prompt, int minValue, int maxValue);
static void readString(const char *prompt, char *buffer, int size);
static int readYesNo(const char *prompt, char *buffer, int size);
static FILE *openProjectFile(const char *filename, const char *mode);
static int parseBuildingLine(char *line, Building *building);
static void printBuilding(const Building *building);
static Building inputBuilding(void);
static void writeBuilding(FILE *file, const Building *building);

static AVLNode *createNode(Building data);
static int getHeight(const AVLNode *node);
static void updateHeight(AVLNode *node);
static int getBalance(const AVLNode *node);
static AVLNode *rightRotate(AVLNode *y);
static AVLNode *leftRotate(AVLNode *x);
static AVLNode *insertAVL(AVLNode *node, Building data, int *inserted);
static AVLNode *minValueNode(AVLNode *node);
static AVLNode *deleteAVL(AVLNode *root, const char *name, int *deleted);
static AVLNode *searchAVL(AVLNode *root, const char *name);
static void inorderTraversal(const AVLNode *root);
static void listApartmentsGreaterThan(const AVLNode *root, int minApartments, int *matches);
static void listUnpaidBuildings(const AVLNode *root, int *matches);
static void saveAVLInorder(FILE *file, const AVLNode *root);
static void freeTree(AVLNode *root);
static AVLNode *loadAVLFromInfo(AVLNode *root);
static int saveAVLToHashFile(const AVLNode *root);
static int searchAndMaybeUpdate(AVLNode *root);

static int initializeTable(HashTable *table, int size);
static int isPrime(int number);
static int nextPrime(int number);
static unsigned int hashFunction(const char *name, int tableSize);
static double loadFactor(const HashTable *table);
static int hashTableLoaded(const HashTable *table);
static void freeHashTable(HashTable *table);
static int insertHash(HashTable *table, Building data, int *collisions);
static int searchHash(const HashTable *table, const char *name, int *index, int *collisions);
static int deleteHash(HashTable *table, const char *name, int *collisions);
static void printHashTable(const HashTable *table);
static int saveHashToFile(const HashTable *table);
static int countValidHashRecords(FILE *file);
static int loadHashFromFile(HashTable *table);

static AVLNode *avlMenu(AVLNode *root, HashTable *table, int *exitProgram);
static int hashMenu(HashTable *table);
static AVLNode *mainMenu(AVLNode *root, HashTable *table);

static void removeNewline(char *str) {
    if (str != NULL) {
        str[strcspn(str, "\n")] = '\0';
    }
}

static void trimSpaces(char *str) {
    int start = 0;
    int end;
    int i = 0;

    if (str == NULL || str[0] == '\0') {
        return;
    }

    while (str[start] != '\0' && isspace((unsigned char)str[start])) {
        start++;
    }

    while (str[start] != '\0') {
        str[i++] = str[start++];
    }
    str[i] = '\0';

    end = (int)strlen(str) - 1;
    while (end >= 0 && isspace((unsigned char)str[end])) {
        str[end--] = '\0';
    }
}

static int parseIntStrict(const char *text, int minValue, int maxValue, int *out) {
    char *endPtr;
    long value;

    if (text == NULL || out == NULL) {
        return 0;
    }

    value = strtol(text, &endPtr, 10);
    if (text == endPtr) {
        return 0;
    }

    while (*endPtr != '\0') {
        if (!isspace((unsigned char)*endPtr)) {
            return 0;
        }
        endPtr++;
    }

    if (value < INT_MIN || value > INT_MAX || value < minValue || value > maxValue) {
        return 0;
    }

    *out = (int)value;
    return 1;
}

static void readString(const char *prompt, char *buffer, int size) {
    while (1) {
        printf("%s", prompt);
        if (fgets(buffer, size, stdin) == NULL) {
            clearerr(stdin);
            buffer[0] = '\0';
            continue;
        }
        removeNewline(buffer);
        trimSpaces(buffer);
        if (buffer[0] != '\0') {
            return;
        }
        printf("Input cannot be empty. Please try again.\n");
    }
}

static int readIntInRange(const char *prompt, int minValue, int maxValue) {
    char buffer[64];
    int value;

    while (1) {
        printf("%s", prompt);
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            clearerr(stdin);
            continue;
        }
        removeNewline(buffer);
        trimSpaces(buffer);
        if (parseIntStrict(buffer, minValue, maxValue, &value)) {
            return value;
        }
        printf("Invalid number. Please enter a value from %d to %d.\n", minValue, maxValue);
    }
}

static int readYesNo(const char *prompt, char *buffer, int size) {
    char temp[32];
    int i;

    while (1) {
        readString(prompt, temp, sizeof(temp));
        for (i = 0; temp[i] != '\0'; i++) {
            temp[i] = (char)tolower((unsigned char)temp[i]);
        }
        if (strcmp(temp, "yes") == 0 || strcmp(temp, "no") == 0) {
            strncpy(buffer, temp, (size_t)size - 1);
            buffer[size - 1] = '\0';
            return 1;
        }
        printf("Please enter yes or no only.\n");
    }
}

//open file beside program
static FILE *openProjectFile(const char *filename, const char *mode) {
    char executablePath[PATH_LEN];
    char filePath[PATH_LEN];
    char *lastSlash;

#ifdef _WIN32
    DWORD length = GetModuleFileNameA(NULL, executablePath, sizeof(executablePath));
    if (length == 0 || length >= sizeof(executablePath)) {
        return fopen(filename, mode);
    }
#else
    ssize_t length = readlink("/proc/self/exe", executablePath, sizeof(executablePath) - 1);
    if (length == -1 || length >= (ssize_t)sizeof(executablePath) - 1) {
        return fopen(filename, mode);
    }
    executablePath[length] = '\0';
#endif

    lastSlash = strrchr(executablePath, '/');
#ifdef _WIN32
    {
        char *lastBackslash = strrchr(executablePath, '\\');
        if (lastBackslash != NULL && (lastSlash == NULL || lastBackslash > lastSlash)) {
            lastSlash = lastBackslash;
        }
    }
#endif

    if (lastSlash == NULL) {
        return fopen(filename, mode);
    }

    *(lastSlash + 1) = '\0';
    if (snprintf(filePath, sizeof(filePath), "%s%s", executablePath, filename) >= (int)sizeof(filePath)) {
        return NULL;
    }

    return fopen(filePath, mode);
}

static int parseBuildingLine(char *line, Building *building) {
    char *token;
    char *fields[6];
    int fieldCount = 0;
    int i;

    if (line == NULL || building == NULL) {
        return 0;
    }

    removeNewline(line);
    token = strtok(line, ":");
    while (token != NULL && fieldCount < 6) {
        trimSpaces(token);
        fields[fieldCount++] = token;
        token = strtok(NULL, ":");
    }

    if (fieldCount != 6 || token != NULL) {
        return 0;
    }

    if (fields[0][0] == '\0' || fields[2][0] == '\0') {
        return 0;
    }

    if (!parseIntStrict(fields[1], 1, INT_MAX, &building->number)) {
        return 0;
    }
    if (!parseIntStrict(fields[3], 0, INT_MAX, &building->apartments)) {
        return 0;
    }
    if (!parseIntStrict(fields[4], MIN_YEAR, MAX_YEAR, &building->year)) {
        return 0;
    }

    strncpy(building->name, fields[0], NAME_LEN - 1);
    building->name[NAME_LEN - 1] = '\0';
    strncpy(building->address, fields[2], ADDRESS_LEN - 1);
    building->address[ADDRESS_LEN - 1] = '\0';
    strncpy(building->paid, fields[5], PAID_LEN - 1);
    building->paid[PAID_LEN - 1] = '\0';

    for (i = 0; building->paid[i] != '\0'; i++) {
        building->paid[i] = (char)tolower((unsigned char)building->paid[i]);
    }

    return strcmp(building->paid, "yes") == 0 || strcmp(building->paid, "no") == 0;
}

static void printBuilding(const Building *building) {
    if (building == NULL) {
        return;
    }

    printf("Name: %s\n", building->name);
    printf("Building number: %d\n", building->number);
    printf("Address: %s\n", building->address);
    printf("Number of apartments: %d\n", building->apartments);
    printf("Establishment year: %d\n", building->year);
    printf("Paid fees: %s\n", building->paid);
}

static Building inputBuilding(void) {
    Building building;

    readString("Enter building name: ", building.name, NAME_LEN);
    building.number = readIntInRange("Enter building number: ", 1, INT_MAX);
    readString("Enter address: ", building.address, ADDRESS_LEN);
    building.apartments = readIntInRange("Enter number of apartments: ", 0, INT_MAX);
    building.year = readIntInRange("Enter establishment year: ", MIN_YEAR, MAX_YEAR);
    readYesNo("Has the building paid fees? (yes/no): ", building.paid, PAID_LEN);

    return building;
}

static void writeBuilding(FILE *file, const Building *building) {
    fprintf(file, "%s:%d:%s:%d:%d:%s\n",
            building->name,
            building->number,
            building->address,
            building->apartments,
            building->year,
            building->paid);
}

//create new avl node
static AVLNode *createNode(Building data) {
    AVLNode *node = (AVLNode *)malloc(sizeof(AVLNode));

    if (node == NULL) {
        printf("Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    node->data = data;
    node->height = 1;
    node->left = NULL;
    node->right = NULL;
    return node;
}

static int getHeight(const AVLNode *node) {
    return node == NULL ? 0 : node->height;
}

static void updateHeight(AVLNode *node) {
    if (node != NULL) {
        int leftHeight = getHeight(node->left);
        int rightHeight = getHeight(node->right);
        node->height = (leftHeight > rightHeight ? leftHeight : rightHeight) + 1;
    }
}

static int getBalance(const AVLNode *node) {
    return node == NULL ? 0 : getHeight(node->left) - getHeight(node->right);
}

//right rotate avl
static AVLNode *rightRotate(AVLNode *y) {
    AVLNode *x = y->left;
    AVLNode *t2 = x->right;

    x->right = y;
    y->left = t2;

    updateHeight(y);
    updateHeight(x);

    return x;
}

//left rotate avl
static AVLNode *leftRotate(AVLNode *x) {
    AVLNode *y = x->right;
    AVLNode *t2 = y->left;

    y->left = x;
    x->right = t2;

    updateHeight(x);
    updateHeight(y);

    return y;
}

//insert building into avl
static AVLNode *insertAVL(AVLNode *node, Building data, int *inserted) {
    int comparison;
    int balance;

    if (node == NULL) {
        *inserted = 1;
        return createNode(data);
    }

    comparison = strcmp(data.name, node->data.name);
    if (comparison < 0) {
        node->left = insertAVL(node->left, data, inserted);
    } else if (comparison > 0) {
        node->right = insertAVL(node->right, data, inserted);
    } else {
        *inserted = 0;
        return node;
    }

    updateHeight(node);
    balance = getBalance(node);

    if (balance > 1 && strcmp(data.name, node->left->data.name) < 0) {
        return rightRotate(node);
    }
    if (balance < -1 && strcmp(data.name, node->right->data.name) > 0) {
        return leftRotate(node);
    }
    if (balance > 1 && strcmp(data.name, node->left->data.name) > 0) {
        node->left = leftRotate(node->left);
        return rightRotate(node);
    }
    if (balance < -1 && strcmp(data.name, node->right->data.name) < 0) {
        node->right = rightRotate(node->right);
        return leftRotate(node);
    }

    return node;
}

static AVLNode *minValueNode(AVLNode *node) {
    AVLNode *current = node;

    while (current != NULL && current->left != NULL) {
        current = current->left;
    }

    return current;
}

//delete building from avl
static AVLNode *deleteAVL(AVLNode *root, const char *name, int *deleted) {
    int comparison;
    int balance;

    if (root == NULL) {
        return NULL;
    }

    comparison = strcmp(name, root->data.name);
    if (comparison < 0) {
        root->left = deleteAVL(root->left, name, deleted);
    } else if (comparison > 0) {
        root->right = deleteAVL(root->right, name, deleted);
    } else {
        AVLNode *temp;
        *deleted = 1;

        if (root->left == NULL || root->right == NULL) {
            temp = root->left != NULL ? root->left : root->right;
            if (temp == NULL) {
                free(root);
                return NULL;
            }
            *root = *temp;
            free(temp);
        } else {
            temp = minValueNode(root->right);
            root->data = temp->data;
            root->right = deleteAVL(root->right, temp->data.name, deleted);
        }
    }

    updateHeight(root);
    balance = getBalance(root);

    if (balance > 1 && getBalance(root->left) >= 0) {
        return rightRotate(root);
    }
    if (balance > 1 && getBalance(root->left) < 0) {
        root->left = leftRotate(root->left);
        return rightRotate(root);
    }
    if (balance < -1 && getBalance(root->right) <= 0) {
        return leftRotate(root);
    }
    if (balance < -1 && getBalance(root->right) > 0) {
        root->right = rightRotate(root->right);
        return leftRotate(root);
    }

    return root;
}

//search building in avl
static AVLNode *searchAVL(AVLNode *root, const char *name) {
    int comparison;

    if (root == NULL) {
        return NULL;
    }

    comparison = strcmp(name, root->data.name);
    if (comparison == 0) {
        return root;
    }
    if (comparison < 0) {
        return searchAVL(root->left, name);
    }
    return searchAVL(root->right, name);
}

static void inorderTraversal(const AVLNode *root) {
    if (root == NULL) {
        return;
    }

    inorderTraversal(root->left);
    printf("----------------------------------------\n");
    printBuilding(&root->data);
    inorderTraversal(root->right);
}

static void listApartmentsGreaterThan(const AVLNode *root, int minApartments, int *matches) {
    if (root == NULL) {
        return;
    }

    listApartmentsGreaterThan(root->left, minApartments, matches);
    if (root->data.apartments > minApartments) {
        printf("----------------------------------------\n");
        printBuilding(&root->data);
        (*matches)++;
    }
    listApartmentsGreaterThan(root->right, minApartments, matches);
}

static void listUnpaidBuildings(const AVLNode *root, int *matches) {
    if (root == NULL) {
        return;
    }

    listUnpaidBuildings(root->left, matches);
    if (strcmp(root->data.paid, "no") == 0) {
        printf("----------------------------------------\n");
        printBuilding(&root->data);
        (*matches)++;
    }
    listUnpaidBuildings(root->right, matches);
}

//save avl in order
static void saveAVLInorder(FILE *file, const AVLNode *root) {
    if (root == NULL) {
        return;
    }

    saveAVLInorder(file, root->left);
    writeBuilding(file, &root->data);
    saveAVLInorder(file, root->right);
}

//free avl memory
static void freeTree(AVLNode *root) {
    if (root == NULL) {
        return;
    }

    freeTree(root->left);
    freeTree(root->right);
    free(root);
}

//load info file into avl
static AVLNode *loadAVLFromInfo(AVLNode *root) {
    FILE *file;
    char line[LINE_LEN];
    Building building;
    int inserted;
    int loaded = 0;
    int skipped = 0;

    file = openProjectFile(INFO_FILE, "r");
    if (file == NULL) {
        printf("info.txt was not found beside the executable.\n");
        return root;
    }

    freeTree(root);
    root = NULL;

    while (fgets(line, sizeof(line), file) != NULL) {
        if (line[0] == '\n' || line[0] == '\0') {
            continue;
        }

        if (parseBuildingLine(line, &building)) {
            inserted = 0;
            root = insertAVL(root, building, &inserted);
            if (inserted) {
                loaded++;
            } else {
                skipped++;
            }
        } else {
            skipped++;
        }
    }

    fclose(file);
    printf("Loaded %d building(s) from %s. Skipped %d invalid or duplicate line(s).\n",
           loaded, INFO_FILE, skipped);
    return root;
}

//save avl to hash file
static int saveAVLToHashFile(const AVLNode *root) {
    FILE *file = openProjectFile(HASH_FILE, "w");

    if (file == NULL) {
        printf("Could not open %s for writing.\n", HASH_FILE);
        return 0;
    }

    saveAVLInorder(file, root);
    fclose(file);
    printf("AVL tree saved to %s successfully.\n", HASH_FILE);
    return 1;
}

//search and update building
static int searchAndMaybeUpdate(AVLNode *root) {
    char name[NAME_LEN];
    char answer[PAID_LEN];
    AVLNode *found;

    readString("Enter building name to search: ", name, NAME_LEN);
    found = searchAVL(root, name);
    if (found == NULL) {
        printf("Building not found.\n");
        return 0;
    }

    printf("Building found:\n");
    printBuilding(&found->data);
    readYesNo("Do you want to update this building? (yes/no): ", answer, PAID_LEN);
    if (strcmp(answer, "yes") != 0) {
        printf("Building was not updated.\n");
        return 0;
    }

    found->data.number = readIntInRange("Enter new building number: ", 1, INT_MAX);
    readString("Enter new address: ", found->data.address, ADDRESS_LEN);
    found->data.apartments = readIntInRange("Enter new number of apartments: ", 0, INT_MAX);
    readYesNo("Has the building paid fees? (yes/no): ", found->data.paid, PAID_LEN);
    printf("Building updated successfully.\n");
    return 1;
}

//create hash table
static int initializeTable(HashTable *table, int size) {
    if (size <= 0) {
        return 0;
    }

    table->size = size;
    table->count = 0;
    table->loaded = 1;
    table->entries = (HashEntry *)calloc((size_t)table->size, sizeof(HashEntry));
    if (table->entries == NULL) {
        printf("Memory allocation failed.\n");
        table->size = 0;
        table->loaded = 0;
        return 0;
    }

    return 1;
}

static int isPrime(int number) {
    int divisor;

    if (number <= 1) {
        return 0;
    }
    if (number <= 3) {
        return 1;
    }
    if (number % 2 == 0 || number % 3 == 0) {
        return 0;
    }

    for (divisor = 5; divisor <= number / divisor; divisor += 6) {
        if (number % divisor == 0 || number % (divisor + 2) == 0) {
            return 0;
        }
    }

    return 1;
}

static int nextPrime(int number) {
    while (!isPrime(number)) {
        number++;
    }

    return number;
}

//hash first four letters
static unsigned int hashFunction(const char *name, int tableSize) {
    unsigned int hash = 0;
    int i;

    for (i = 0; i < 4 && name[i] != '\0'; i++) {
        hash = hash * 31u + (unsigned char)name[i];
    }

    return hash % (unsigned int)tableSize;
}

static double loadFactor(const HashTable *table) {
    if (table == NULL || table->size == 0) {
        return 0.0;
    }
    return (double)table->count / table->size;
}

static int hashTableLoaded(const HashTable *table) {
    return table != NULL && table->entries != NULL && table->loaded;
}

//free hash memory
static void freeHashTable(HashTable *table) {
    if (table != NULL) {
        free(table->entries);
        table->entries = NULL;
        table->size = 0;
        table->count = 0;
        table->loaded = 0;
    }
}

//insert building into hash
static int insertHash(HashTable *table, Building data, int *collisions) {
    unsigned int base;
    int i;
    int index;
    int firstDeleted = -1;
    int target;

    if (collisions != NULL) {
        *collisions = 0;
    }

    if (table == NULL || table->entries == NULL || table->size == 0) {
        return -1;
    }

    for (i = 0; i < table->size; i++) {
        if (table->entries[i].status == OCCUPIED) {
            if (strcmp(table->entries[i].data.name, data.name) == 0) {
                return 0;
            }
        }
    }

    base = hashFunction(data.name, table->size);
    for (i = 0; i < table->size; i++) {
        index = (int)((base + (unsigned int)i) % (unsigned int)table->size);

        if (table->entries[index].status == OCCUPIED) {
            if (collisions != NULL) {
                (*collisions)++;
            }
        } else if (table->entries[index].status == DELETED) {
            if (firstDeleted == -1) {
                firstDeleted = index;
            }
        } else {
            target = firstDeleted != -1 ? firstDeleted : index;
            table->entries[target].data = data;
            table->entries[target].status = OCCUPIED;
            table->count++;
            return 1;
        }
    }

    if (firstDeleted != -1) {
        table->entries[firstDeleted].data = data;
        table->entries[firstDeleted].status = OCCUPIED;
        table->count++;
        return 1;
    }

    return -1;
}

//search building in hash table
static int searchHash(const HashTable *table, const char *name, int *index, int *collisions) {
    unsigned int base;
    int i;
    int current;

    if (index != NULL) {
        *index = -1;
    }
    if (collisions != NULL) {
        *collisions = 0;
    }

    if (table == NULL || table->entries == NULL || table->size == 0) {
        return 0;
    }

    base = hashFunction(name, table->size);
    for (i = 0; i < table->size; i++) {
        current = (int)((base + (unsigned int)i) % (unsigned int)table->size);

        if (table->entries[current].status == EMPTY) {
            return 0;
        }

        if (table->entries[current].status == OCCUPIED) {
            if (strcmp(table->entries[current].data.name, name) == 0) {
                if (index != NULL) {
                    *index = current;
                }
                return 1;
            }
            if (collisions != NULL) {
                (*collisions)++;
            }
        }
    }

    return 0;
}

//delete building from hash table
static int deleteHash(HashTable *table, const char *name, int *collisions) {
    int index;

    if (searchHash(table, name, &index, collisions)) {
        table->entries[index].status = DELETED;
        table->count--;
        return 1;
    }

    return 0;
}

//print all hash table
static void printHashTable(const HashTable *table) {
    int i;

    if (table == NULL || table->entries == NULL) {
        printf("Hash table is not loaded. Load it from %s first.\n", HASH_FILE);
        return;
    }

    for (i = 0; i < table->size; i++) {
        printf("[%d] ", i);
        if (table->entries[i].status == EMPTY) {
            printf("EMPTY\n");
        } else if (table->entries[i].status == DELETED) {
            printf("DELETED\n");
        } else {
            writeBuilding(stdout, &table->entries[i].data);
        }
    }
}

//save hash table to file
static int saveHashToFile(const HashTable *table) {
    FILE *file;
    int i;

    if (table == NULL || table->entries == NULL) {
        printf("Hash table is not loaded. Nothing was saved.\n");
        return 0;
    }

    file = openProjectFile(HASH_FILE, "w");
    if (file == NULL) {
        printf("Could not open %s for writing.\n", HASH_FILE);
        return 0;
    }

    for (i = 0; i < table->size; i++) {
        if (table->entries[i].status == OCCUPIED) {
            writeBuilding(file, &table->entries[i].data);
        }
    }

    fclose(file);
    printf("Hash table saved to %s successfully.\n", HASH_FILE);
    return 1;
}

static int countValidHashRecords(FILE *file) {
    char line[LINE_LEN];
    Building building;
    int count = 0;

    rewind(file);
    while (fgets(line, sizeof(line), file) != NULL) {
        if (line[0] != '\n' && line[0] != '\0' && parseBuildingLine(line, &building)) {
            count++;
        }
    }
    rewind(file);

    return count;
}

//load hash table from file
static int loadHashFromFile(HashTable *table) {
    FILE *file = openProjectFile(HASH_FILE, "r");
    char line[LINE_LEN];
    Building building;
    int inserted;
    int collisions;
    int loaded = 0;
    int skipped = 0;
    int recordCount;
    int tableSize;

    if (file == NULL) {
        printf("hash.txt was not found beside the executable.\n");
        return 0;
    }

    recordCount = countValidHashRecords(file);
    tableSize = recordCount * 2 + 1;
    if (tableSize < MIN_HASH_TABLE_SIZE) {
        tableSize = MIN_HASH_TABLE_SIZE;
    }
    tableSize = nextPrime(tableSize);

    freeHashTable(table);
    if (!initializeTable(table, tableSize)) {
        fclose(file);
        return 0;
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        if (line[0] == '\n' || line[0] == '\0') {
            continue;
        }

        if (parseBuildingLine(line, &building)) {
            inserted = insertHash(table, building, &collisions);
            if (inserted == 1) {
                loaded++;
            } else {
                skipped++;
            }
        } else {
            skipped++;
        }
    }

    fclose(file);
    printf("Loaded %d building(s) into hash table from %s. Skipped %d invalid or duplicate line(s).\n",
           loaded, HASH_FILE, skipped);
    return 1;
}

//show avl menu
static AVLNode *avlMenu(AVLNode *root, HashTable *table, int *exitProgram) {
    int choice;
    int inserted;
    int deleted;
    int minApartments;
    int matches;
    char name[NAME_LEN];
    Building building;

    while (1) {
        printf("\n================ AVL TREE MENU ================\n");
        printf("1. Load %s and create AVL tree\n", INFO_FILE);
        printf("2. Insert a new building\n");
        printf("3. Search for a building and optionally update it\n");
        printf("4. List all buildings alphabetically\n");
        printf("5. List buildings whose apartments are greater than input\n");
        printf("6. List buildings that have not paid fees\n");
        printf("7. Delete a building\n");
        printf("8. Save AVL tree to %s\n", HASH_FILE);
        printf("9. Save AVL tree and open Hash Table Menu\n");
        printf("10. Return to Main Menu\n");
        choice = readIntInRange("Enter your choice: ", 1, 10);

        switch (choice) {
            case 1:
                root = loadAVLFromInfo(root);
                table->loaded = 0;
                break;
            case 2:
                building = inputBuilding();
                inserted = 0;
                root = insertAVL(root, building, &inserted);
                if (inserted) {
                    printf("Building inserted successfully.\n");
                    table->loaded = 0;
                } else {
                    printf("Duplicate building name. Insertion rejected.\n");
                }
                break;
            case 3:
                if (root == NULL) {
                    printf("AVL tree is empty.\n");
                } else {
                    if (searchAndMaybeUpdate(root)) {
                        table->loaded = 0;
                    }
                }
                break;
            case 4:
                if (root == NULL) {
                    printf("AVL tree is empty.\n");
                } else {
                    inorderTraversal(root);
                    printf("----------------------------------------\n");
                }
                break;
            case 5:
                if (root == NULL) {
                    printf("AVL tree is empty.\n");
                } else {
                    minApartments = readIntInRange("Enter number of apartments: ", 0, INT_MAX);
                    matches = 0;
                    listApartmentsGreaterThan(root, minApartments, &matches);
                    if (matches == 0) {
                        printf("No matching buildings found.\n");
                    } else {
                        printf("----------------------------------------\n");
                    }
                }
                break;
            case 6:
                if (root == NULL) {
                    printf("AVL tree is empty.\n");
                } else {
                    matches = 0;
                    listUnpaidBuildings(root, &matches);
                    if (matches == 0) {
                        printf("No unpaid buildings found.\n");
                    } else {
                        printf("----------------------------------------\n");
                    }
                }
                break;
            case 7:
                if (root == NULL) {
                    printf("AVL tree is empty.\n");
                } else {
                    readString("Enter building name to delete: ", name, NAME_LEN);
                    deleted = 0;
                    root = deleteAVL(root, name, &deleted);
                    if (deleted) {
                        printf("Building deleted successfully.\n");
                        table->loaded = 0;
                    } else {
                        printf("Building not found.\n");
                    }
                }
                break;
            case 8:
                if (saveAVLToHashFile(root)) {
                    table->loaded = 0;
                }
                break;
            case 9:
                if (saveAVLToHashFile(root)) {
                    loadHashFromFile(table);
                    if (hashMenu(table)) {
                        *exitProgram = 1;
                        return root;
                    }
                }
                break;
            case 10:
                return root;
        }
    }

    return root;
}

//show hash menu
static int hashMenu(HashTable *table) {
    int choice;
    int collisions;
    int index;
    int inserted;
    char name[NAME_LEN];
    Building building;

    while (1) {
        printf("\n================ HASH TABLE MENU ================\n");
        printf("1. Load hash table from %s\n", HASH_FILE);
        printf("2. Print full hash table including empty spots\n");
        printf("3. Print table size and load factor\n");
        printf("4. Insert a new building\n");
        printf("5. Search for a building and print collision count\n");
        printf("6. Delete a building\n");
        printf("7. Save hash table to %s\n", HASH_FILE);
        printf("8. Return to Main Menu\n");
        printf("9. Save and Exit\n");
        choice = readIntInRange("Enter your choice: ", 1, 9);

        if (choice != 1 && choice != 8 && choice != 9 && !hashTableLoaded(table)) {
            printf("Hash table is not loaded. Choose option 1 to load data from %s first.\n", HASH_FILE);
            continue;
        }

        switch (choice) {
            case 1:
                loadHashFromFile(table);
                break;
            case 2:
                printHashTable(table);
                break;
            case 3:
                printf("Hash table size: %d\n", table->size);
                printf("Load factor: %.2f\n", loadFactor(table));
                break;
            case 4:
                building = inputBuilding();
                inserted = insertHash(table, building, &collisions);
                if (inserted == 1) {
                    printf("Building inserted successfully. Collisions during insertion: %d\n", collisions);
                } else if (inserted == 0) {
                    printf("Duplicate building name. Insertion rejected.\n");
                } else {
                    printf("Hash table is full. Insertion failed.\n");
                }
                break;
            case 5:
                readString("Enter building name to search: ", name, NAME_LEN);
                if (searchHash(table, name, &index, &collisions)) {
                    printf("Building found at index %d.\n", index);
                    printf("Collisions during search: %d\n", collisions);
                    printBuilding(&table->entries[index].data);
                } else {
                    printf("Building not found.\n");
                    printf("Collisions during search: %d\n", collisions);
                }
                break;
            case 6:
                readString("Enter building name to delete: ", name, NAME_LEN);
                if (deleteHash(table, name, &collisions)) {
                    printf("Building deleted successfully. Collisions during delete search: %d\n", collisions);
                } else {
                    printf("Building not found. ");
                    printf("Collisions during delete search: %d\n", collisions);
                }
                break;
            case 7:
                saveHashToFile(table);
                break;
            case 8:
                return 0;
            case 9:
                if (hashTableLoaded(table)) {
                    saveHashToFile(table);
                } else {
                    printf("Hash table is not loaded, so no hash data was saved.\n");
                }
                printf("Program finished.\n");
                return 1;
        }
    }

    return 0;
}

//show main menu
static AVLNode *mainMenu(AVLNode *root, HashTable *table) {
    int choice;
    int exitProgram;

    while (1) {
        printf("\n================ MAIN MENU ================\n");
        printf("1. AVL Tree Section\n");
        printf("2. Hash Table Section\n");
        printf("3. Save Hash Table and Exit\n");
        printf("4. Exit Without Saving Hash Table\n");
        choice = readIntInRange("Enter your choice: ", 1, 4);

        switch (choice) {
            case 1:
                exitProgram = 0;
                root = avlMenu(root, table, &exitProgram);
                if (exitProgram) {
                    return root;
                }
                break;
            case 2:
                printf("Hash table data is loaded from %s. Save the AVL tree to %s before loading if you changed AVL data.\n",
                       HASH_FILE, HASH_FILE);
                if (hashMenu(table)) {
                    return root;
                }
                break;
            case 3:
                if (hashTableLoaded(table)) {
                    saveHashToFile(table);
                } else {
                    printf("Hash table is not loaded, so no hash data was saved.\n");
                }
                return root;
            case 4:
                return root;
        }
    }

    return root;
}

int main(void) {
    AVLNode *root = NULL;
    HashTable table;

    table.entries = NULL;
    table.size = 0;
    table.count = 0;
    table.loaded = 0;

    printf("COMP2421 Data Structures Project 2\n");
    printf("Student: Emam Samara | ID: 1220022 | Section: 1\n");

    root = mainMenu(root, &table);

    freeTree(root);
    freeHashTable(&table);
    return 0;
}
