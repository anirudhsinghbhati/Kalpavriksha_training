#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define BLOCK_SIZE            512
#define DEFAULT_NUM_BLOCKS    1024
#define MIN_NUM_BLOCKS        1
#define MAX_NUM_BLOCKS        5000
#define MAX_NAME_LENGTH       50

typedef struct FreeBlock {
    int blockIndex;
    struct FreeBlock *next;
    struct FreeBlock *prev;
} FreeBlock;

typedef struct FileNode {
    char name[MAX_NAME_LENGTH + 1];
    int isDirectory;
    struct FileNode *parent;
    struct FileNode *next;        
    struct FileNode *prev;
    struct FileNode *childHead;   
    int *blockPointers;           
    int blockCount;
    size_t contentSize;
} FileNode;

static unsigned char **virtualDisk = NULL;
static int totalBlocks = DEFAULT_NUM_BLOCKS;
static int freeBlockCount = 0;
static FreeBlock *freeListHead = NULL;
static FreeBlock *freeListTail = NULL;
static FileNode *rootDirectory = NULL;
static FileNode *currentDirectory = NULL;



static int appendFreeBlock(int blockIndex) {
    FreeBlock *newBlock = malloc(sizeof(FreeBlock));

    if (newBlock == NULL) {
        return 0;
    }

    newBlock->blockIndex = blockIndex;
    newBlock->next = NULL;
    newBlock->prev = freeListTail;

    if (freeListTail != NULL) {
        freeListTail->next = newBlock;
    } 
    else {
        freeListHead = newBlock;
    }

    freeListTail = newBlock;
    freeBlockCount++;
    return 1;
}

static int takeFreeBlock(void) {
    FreeBlock *headBlock = freeListHead;
    int blockIndex;

    if (headBlock == NULL) {
        return -1;
    }

    blockIndex = headBlock->blockIndex;
    freeListHead = headBlock->next;

    if (freeListHead != NULL) {
        freeListHead->prev = NULL;
    } 
    else {
        freeListTail = NULL;
    }

    free(headBlock);
    freeBlockCount--;
    return blockIndex;
}

static void releaseFileBlocks(FileNode *fileNode) {
    int blockPosition;

    for (blockPosition = 0; blockPosition < fileNode->blockCount; blockPosition++) {
        appendFreeBlock(fileNode->blockPointers[blockPosition]);
    }

    free(fileNode->blockPointers);
    fileNode->blockPointers = NULL;
    fileNode->blockCount = 0;
    fileNode->contentSize = 0;
}

static FileNode *createNode(const char *name, int isDirectory, FileNode *parent) {
    FileNode *newNode = calloc(1, sizeof(FileNode));

    if (newNode == NULL) {
        return NULL;
    }

    strncpy(newNode->name, name, MAX_NAME_LENGTH);
    newNode->name[MAX_NAME_LENGTH] = '\0';
    newNode->isDirectory = isDirectory;
    newNode->parent = parent;
    return newNode;
}

static void insertChild(FileNode *directory, FileNode *newNode) {
    if (directory->childHead == NULL) {
        newNode->next = newNode;
        newNode->prev = newNode;
        directory->childHead = newNode;
    } 
    else {
        FileNode *tailNode = directory->childHead->prev;

        tailNode->next = newNode;
        newNode->prev = tailNode;
        newNode->next = directory->childHead;
        directory->childHead->prev = newNode;
    }
}

static void removeChild(FileNode *directory, FileNode *targetNode) {
    if (targetNode->next == targetNode) {
        directory->childHead = NULL;
    } 
    else {
        targetNode->prev->next = targetNode->next;
        targetNode->next->prev = targetNode->prev;

        if (directory->childHead == targetNode) {
            directory->childHead = targetNode->next;
        }
    }

    targetNode->next = NULL;
    targetNode->prev = NULL;
}

static FileNode *findChild(FileNode *directory, const char *name) {
    FileNode *currentNode = directory->childHead;

    if (currentNode == NULL) {
        return NULL;
    }

    do {
        if (strcmp(currentNode->name, name) == 0) {
            return currentNode;
        }
        currentNode = currentNode->next;
    } while (currentNode != directory->childHead);

    return NULL;
}

static void freeTree(FileNode *node) {
    if (node == NULL) {
        return;
    }

    if (node->isDirectory) {
        while (node->childHead != NULL) {
            FileNode *childNode = node->childHead;

            removeChild(node, childNode);
            freeTree(childNode);
        }
    } 
    else {
        free(node->blockPointers);
    }

    free(node);
}

static int isValidName(const char *name) {
    size_t nameLength = strlen(name);
    size_t charPosition;

    if (nameLength == 0 || nameLength > MAX_NAME_LENGTH) {
        printf("Error: name must be 1-%d characters.\n", MAX_NAME_LENGTH);
        return 0;
    }

    if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
        printf("Error: '%s' is a reserved name.\n", name);
        return 0;
    }

    for (charPosition = 0; charPosition < nameLength; charPosition++) {
        unsigned char character = (unsigned char)name[charPosition];

        if (!isprint(character) || character == '/' || character == '"') {
            printf("Error: name contains invalid character.\n");
            return 0;
        }
    }

    return 1;
}

static char *readLine(void)
{
    size_t capacity = 128;
    size_t length = 0;
    int character;
    char *buffer = malloc(capacity);

    if (buffer == NULL) {
        return NULL;
    }

    while ((character = fgetc(stdin)) != EOF && character != '\n') {
        if (length + 1 >= capacity) {
            char *grownBuffer;

            capacity *= 2;
            grownBuffer = realloc(buffer, capacity);

            if (grownBuffer == NULL) {
                free(buffer);
                return NULL;
            }

            buffer = grownBuffer;
        }

        buffer[length++] = (char)character;
    }

    if (character == EOF && length == 0) {
        free(buffer);
        return NULL;
    }

    buffer[length] = '\0';

    if (length > 0 && buffer[length - 1] == '\r') {
        buffer[length - 1] = '\0';
    }

    return buffer;
}

static char *nextToken(char **cursor) {
    char *tokenStart;

    while (**cursor != '\0' && isspace((unsigned char)**cursor)) {
        (*cursor)++;
    }

    if (**cursor == '\0') {
        return NULL;
    }

    tokenStart = *cursor;

    while (**cursor != '\0' && !isspace((unsigned char)**cursor)) {
        (*cursor)++;
    }

    if (**cursor != '\0') {
        **cursor = '\0';
        (*cursor)++;
    }

    return tokenStart;
}

static int hasExtraArguments(char *cursor) {
    if (nextToken(&cursor) != NULL) {
        printf("Error: too many arguments.\n");
        return 1;
    }

    return 0;
}

static void printPrompt(void) {
    if (currentDirectory == rootDirectory) {
        printf("/ > ");
    } 
    else {
        printf("%s > ", currentDirectory->name);
    }

    fflush(stdout);
}

static void commandCreateNode(char *arguments, int isDirectory) {
    char *name = nextToken(&arguments);
    FileNode *newNode;

    if (name == NULL) {
        printf("Usage: %s <name>\n", isDirectory ? "mkdir" : "create");
        return;
    }

    if (hasExtraArguments(arguments)) {
        return;
    }

    if (!isValidName(name)) {
        return;
    }

    if (findChild(currentDirectory, name) != NULL) {
        printf("Error: '%s' already exists.\n", name);
        return;
    }

    newNode = createNode(name, isDirectory, currentDirectory);

    if (newNode == NULL) {
        printf("Error: out of memory.\n");
        return;
    }

    insertChild(currentDirectory, newNode);
    printf("%s '%s' created successfully.\n", isDirectory ? "Directory" : "File", name);
}

static void commandWrite(char *arguments) {
    char *name = nextToken(&arguments);
    char *contentStart;
    char *contentEnd;
    FileNode *fileNode;
    size_t contentSize;
    size_t bytesWritten;
    int blocksNeeded;
    int blockPosition;
    int *newBlockPointers = NULL;

    if (name == NULL) {
        printf("Usage: write <file> \"content\"\n");
        return;
    }

    while (*arguments != '\0' && isspace((unsigned char)*arguments)) {
        arguments++;
    }

    contentStart = arguments;

    if (*contentStart != '"') {
        printf("Error: content must be enclosed in double quotes.\n");
        return;
    }

    contentEnd = contentStart + strlen(contentStart) - 1;

    while (contentEnd > contentStart && isspace((unsigned char)*contentEnd)) {
        contentEnd--;
    }

    if (contentEnd == contentStart || *contentEnd != '"') {
        printf("Error: content must be enclosed in double quotes.\n");
        return;
    }

    contentStart++;
    contentSize = (size_t)(contentEnd - contentStart);

    fileNode = findChild(currentDirectory, name);

    if (fileNode == NULL) {
        printf("Error: file '%s' not found.\n", name);
        return;
    }

    if (fileNode->isDirectory) {
        printf("Error: '%s' is a directory.\n", name);
        return;
    }

    blocksNeeded = (int)((contentSize + BLOCK_SIZE - 1) / BLOCK_SIZE);

    if (blocksNeeded > freeBlockCount + fileNode->blockCount) {
        printf("Error: disk full. Need %d blocks, only %d available.\n",
               blocksNeeded, freeBlockCount + fileNode->blockCount);
        return;
    }

    if (blocksNeeded > 0) {
        newBlockPointers = malloc((size_t)blocksNeeded * sizeof(int));

        if (newBlockPointers == NULL) {
            printf("Error: out of memory.\n");
            return;
        }
    }

    releaseFileBlocks(fileNode);

    bytesWritten = 0;

    for (blockPosition = 0; blockPosition < blocksNeeded; blockPosition++) {
        int blockIndex = takeFreeBlock();
        size_t chunkSize = contentSize - bytesWritten;

        if (chunkSize > BLOCK_SIZE) {
            chunkSize = BLOCK_SIZE;
        }

        newBlockPointers[blockPosition] = blockIndex;
        memset(virtualDisk[blockIndex], 0, BLOCK_SIZE);
        memcpy(virtualDisk[blockIndex], contentStart + bytesWritten, chunkSize);
        bytesWritten += chunkSize;
    }

    fileNode->blockPointers = newBlockPointers;
    fileNode->blockCount = blocksNeeded;
    fileNode->contentSize = contentSize;
    printf("Data written successfully (size=%zu bytes).\n", contentSize);
}

static void commandRead(char *arguments) {
    char *name = nextToken(&arguments);
    FileNode *fileNode;
    size_t bytesRemaining;
    int blockPosition;

    if (name == NULL) {
        printf("Usage: read <file>\n");
        return;
    }

    if (hasExtraArguments(arguments)) {
        return;
    }

    fileNode = findChild(currentDirectory, name);

    if (fileNode == NULL) {
        printf("Error: file '%s' not found.\n", name);
        return;
    }

    if (fileNode->isDirectory) {
        printf("Error: '%s' is a directory.\n", name);
        return;
    }

    if (fileNode->contentSize == 0) {
        printf("(empty file)\n");
        return;
    }

    bytesRemaining = fileNode->contentSize;

    for (blockPosition = 0; blockPosition < fileNode->blockCount; blockPosition++) {
        size_t chunkSize = bytesRemaining > BLOCK_SIZE ? BLOCK_SIZE : bytesRemaining;

        fwrite(virtualDisk[fileNode->blockPointers[blockPosition]], 1, chunkSize, stdout);
        bytesRemaining -= chunkSize;
    }

    printf("\n");
}

static void commandDelete(char *arguments) {
    char *name = nextToken(&arguments);
    FileNode *fileNode;

    if (name == NULL) {
        printf("Usage: delete <file>\n");
        return;
    }

    if (hasExtraArguments(arguments)) {
        return;
    }

    fileNode = findChild(currentDirectory, name);

    if (fileNode == NULL) {
        printf("Error: file '%s' not found.\n", name);
        return;
    }

    if (fileNode->isDirectory) {
        printf("Error: '%s' is a directory. Use rmdir.\n", name);
        return;
    }

    releaseFileBlocks(fileNode);
    removeChild(currentDirectory, fileNode);
    free(fileNode);
    printf("File deleted successfully.\n");
}

static void commandRmdir(char *arguments) {
    char *name = nextToken(&arguments);
    FileNode *directoryNode;

    if (name == NULL) {
        printf("Usage: rmdir <directory>\n");
        return;
    }

    if (hasExtraArguments(arguments)) {
        return;
    }

    directoryNode = findChild(currentDirectory, name);

    if (directoryNode == NULL) {
        printf("Error: directory '%s' not found.\n", name);
        return;
    }

    if (!directoryNode->isDirectory) {
        printf("Error: '%s' is a file. Use delete.\n", name);
        return;
    }

    if (directoryNode->childHead != NULL) {
        printf("Error: directory '%s' is not empty.\n", name);
        return;
    }

    removeChild(currentDirectory, directoryNode);
    free(directoryNode);
    printf("Directory removed successfully.\n");
}

static void commandLs(char *arguments) {
    FileNode *currentNode;

    if (hasExtraArguments(arguments)) {
        return;
    }

    currentNode = currentDirectory->childHead;

    if (currentNode == NULL) {
        printf("(empty)\n");
        return;
    }

    do {
        printf("%s%s\n", currentNode->name, currentNode->isDirectory ? "/" : "");
        currentNode = currentNode->next;
    } while (currentNode != currentDirectory->childHead);
}

static void commandPwd(char *arguments) {
    int depth = 0;
    int depthPosition;
    FileNode *walkNode;
    FileNode **pathNodes;

    if (hasExtraArguments(arguments)) {
        return;
    }

    if (currentDirectory == rootDirectory) {
        printf("/\n");
        return;
    }

    for (walkNode = currentDirectory; walkNode != rootDirectory; walkNode = walkNode->parent) {
        depth++;
    }

    pathNodes = malloc((size_t)depth * sizeof(FileNode *));

    if (pathNodes == NULL) {
        printf("Error: out of memory.\n");
        return;
    }

    walkNode = currentDirectory;

    for (depthPosition = depth - 1; depthPosition >= 0; depthPosition--) {
        pathNodes[depthPosition] = walkNode;
        walkNode = walkNode->parent;
    }

    for (depthPosition = 0; depthPosition < depth; depthPosition++) {
        printf("/%s", pathNodes[depthPosition]->name);
    }

    printf("\n");
    free(pathNodes);
}

static void commandCd(char *arguments) {
    char *target = nextToken(&arguments);
    FileNode *targetNode;

    if (target == NULL) {
        printf("Usage: cd <directory|..|/>\n");
        return;
    }

    if (hasExtraArguments(arguments)) {
        return;
    }

    if (strcmp(target, "/") == 0) {
        currentDirectory = rootDirectory;
    } 
    else if (strcmp(target, "..") == 0) {
        if (currentDirectory->parent != NULL) {
            currentDirectory = currentDirectory->parent;
        }
    } 
    else {
        targetNode = findChild(currentDirectory, target);

        if (targetNode == NULL) {
            printf("Error: directory '%s' not found.\n", target);
            return;
        }

        if (!targetNode->isDirectory) {
            printf("Error: '%s' is not a directory.\n", target);
            return;
        }

        currentDirectory = targetNode;
    }

    printf("Moved to ");
    commandPwd("");
}

static void commandDf(char *arguments)
{
    int usedBlocks = totalBlocks - freeBlockCount;

    if (hasExtraArguments(arguments)) {
        return;
    }

    printf("Total Blocks: %d\n", totalBlocks);
    printf("Used Blocks: %d\n", usedBlocks);
    printf("Free Blocks: %d\n", freeBlockCount);
    printf("Disk Usage: %.2f%%\n", 100.0 * usedBlocks / totalBlocks);
}

static int initializeSystem(void) {
    int blockIndex;

    virtualDisk = calloc((size_t)totalBlocks, sizeof(unsigned char *));

    if (virtualDisk == NULL) {
        return 0;
    }

    for (blockIndex = 0; blockIndex < totalBlocks; blockIndex++) {
        virtualDisk[blockIndex] = calloc(BLOCK_SIZE, 1);

        if (virtualDisk[blockIndex] == NULL) {
            return 0;
        }

        if (!appendFreeBlock(blockIndex)) {
            return 0;
        }
    }

    rootDirectory = createNode("/", 1, NULL);

    if (rootDirectory == NULL) {
        return 0;
    }

    currentDirectory = rootDirectory;
    return 1;
}

static void cleanupSystem(void) {
    int blockIndex;

    freeTree(rootDirectory);

    while (freeListHead != NULL) {
        takeFreeBlock();
    }

    if (virtualDisk != NULL) {
        for (blockIndex = 0; blockIndex < totalBlocks; blockIndex++) {
            free(virtualDisk[blockIndex]);
        }

        free(virtualDisk);
    }
}

int main(int argc, char *argv[]) {
    int keepRunning = 1;

    if (argc > 2) {
        fprintf(stderr, "Usage: %s [num_blocks (%d-%d)]\n", argv[0], MIN_NUM_BLOCKS, MAX_NUM_BLOCKS);
        return 1;
    }

    if (argc == 2) {
        char *endPointer;
        long parsedBlocks = strtol(argv[1], &endPointer, 10);

        if (*argv[1] == '\0' || *endPointer != '\0' ||
            parsedBlocks < MIN_NUM_BLOCKS || parsedBlocks > MAX_NUM_BLOCKS) {
            fprintf(stderr, "Error: num_blocks must be an integer in %d-%d.\n",
                    MIN_NUM_BLOCKS, MAX_NUM_BLOCKS);
            return 1;
        }

        totalBlocks = (int)parsedBlocks;
    }

    if (!initializeSystem()) {
        fprintf(stderr, "Error: initialization failed (out of memory).\n");
        cleanupSystem();
        return 1;
    }

    printf("Compact VFS - ready. Type 'exit' to quit.\n");

    while (keepRunning) {
        char *inputLine;
        char *cursor;
        char *command;

        printPrompt();
        inputLine = readLine();

        if (inputLine == NULL) {
            printf("\n");
            break;
        }

        cursor = inputLine;
        command = nextToken(&cursor);

        if (command == NULL) {
        } else if (strcmp(command, "mkdir") == 0) {
            commandCreateNode(cursor, 1);
        } else if (strcmp(command, "create") == 0) {
            commandCreateNode(cursor, 0);
        } else if (strcmp(command, "write") == 0) {
            commandWrite(cursor);
        } else if (strcmp(command, "read") == 0) {
            commandRead(cursor);
        } else if (strcmp(command, "delete") == 0) {
            commandDelete(cursor);
        } else if (strcmp(command, "rmdir") == 0) {
            commandRmdir(cursor);
        } else if (strcmp(command, "ls") == 0) {
            commandLs(cursor);
        } else if (strcmp(command, "cd") == 0) {
            commandCd(cursor);
        } else if (strcmp(command, "pwd") == 0) {
            commandPwd(cursor);
        } else if (strcmp(command, "df") == 0) {
            commandDf(cursor);
        } else if (strcmp(command, "exit") == 0) {
            if (!hasExtraArguments(cursor)) {
                keepRunning = 0;
            }
        } else {
            printf("Error: unknown command '%s'.\n", command);
        }

        free(inputLine);
    }

    cleanupSystem();
    printf("Memory released. Exiting program...\n");
    return 0;
}