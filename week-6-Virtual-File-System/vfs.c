#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DISK_BLOCKS 1024
#define BLOCK_CAPACITY 512
#define NAME_LIMIT 50
#define INPUT_LIMIT 8192

typedef struct FSNode
{
    char filename[NAME_LIMIT + 1];
    int directory;
    size_t fileSize;

    int totalBlocks;
    int *blockMap;

    struct FSNode *parent;
    struct FSNode *next;
    struct FSNode *firstChild;
} FSNode;

typedef struct
{
    unsigned char storage[DISK_BLOCKS][BLOCK_CAPACITY];
    int available[DISK_BLOCKS];
    int top;
} VirtualDisk;

static VirtualDisk disk;
static FSNode *rootDir = NULL;
static FSNode *currentDir = NULL;

void initializeDisk(void)
{
    memset(&disk, 0, sizeof(disk));

    disk.top = DISK_BLOCKS;

    for (int i = 0; i < DISK_BLOCKS; i++)
    {
        disk.available[i] = i;
    }
}

int allocateBlock(void)
{
    if (disk.top == 0)
    {
        return -1;
    }

    return disk.available[--disk.top];
}

void returnBlock(int block)
{
    if (block >= 0 && block < DISK_BLOCKS && disk.top < DISK_BLOCKS)
    {
        disk.available[disk.top++] = block;
    }
}

int availableBlocks(void)
{
    return disk.top;
}

FSNode *createEntry(const char *name, int isDirectory, FSNode *parent)
{
    FSNode *item = calloc(1, sizeof(FSNode));

    if (item == NULL)
    {
        perror("Memory allocation failed");
        return NULL;
    }

    snprintf(item->filename, sizeof(item->filename), "%s", name);

    item->directory = isDirectory;
    item->parent = parent;

    return item;
}

FSNode *searchEntry(FSNode *directory, const char *name)
{
    if (directory == NULL || directory->firstChild == NULL)
    {
        return NULL;
    }

    FSNode *item = directory->firstChild;

    do
    {
        if (strcmp(item->filename, name) == 0)
        {
            return item;
        }

        item = item->next;

    } while (item != directory->firstChild);

    return NULL;
}

void attachEntry(FSNode *directory, FSNode *item)
{
    if (directory->firstChild == NULL)
    {
        directory->firstChild = item;
        item->next = item;
        return;
    }

    FSNode *last = directory->firstChild;

    while (last->next != directory->firstChild)
    {
        last = last->next;
    }

    last->next = item;
    item->next = directory->firstChild;
}

void detachEntry(FSNode *directory, FSNode *item)
{
    if (item->next == item)
    {
        directory->firstChild = NULL;
    }
    else
    {
        FSNode *previous = directory->firstChild;

        while (previous->next != item)
        {
            previous = previous->next;
        }

        previous->next = item->next;

        if (directory->firstChild == item)
        {
            directory->firstChild = item->next;
        }
    }

    item->next = NULL;
}

int checkName(const char *name)
{
    size_t length = strlen(name);

    return length > 0 &&
           length <= NAME_LIMIT &&
           strchr(name, '/') == NULL &&
           strcmp(name, ".") != 0 &&
           strcmp(name, "..") != 0;
}

void createObject(const char *name, int isDirectory)
{
    if (!checkName(name))
    {
        printf("Error: Invalid name. Maximum length is %d characters.\n",
               NAME_LIMIT);
        return;
    }

    if (searchEntry(currentDir, name) != NULL)
    {
        printf("Error: '%s' already exists.\n", name);
        return;
    }

    FSNode *item = createEntry(name, isDirectory, currentDir);

    if (item == NULL)
    {
        return;
    }

    attachEntry(currentDir, item);

    printf("%s '%s' created successfully.\n",
           isDirectory ? "Directory" : "File", name);
}

void clearFileStorage(FSNode *file)
{
    for (int i = 0; i < file->totalBlocks; i++)
    {
        returnBlock(file->blockMap[i]);
    }

    free(file->blockMap);

    file->blockMap = NULL;
    file->totalBlocks = 0;
    file->fileSize = 0;
}

void writeData(const char *name, const char *content)
{
    FSNode *file = searchEntry(currentDir, name);

    if (file == NULL)
    {
        printf("Error: file '%s' not found.\n", name);
        return;
    }

    if (file->directory)
    {
        printf("Error: '%s' is a directory.\n", name);
        return;
    }

    size_t length = strlen(content);

    int required = (int)(
        length / BLOCK_CAPACITY +
        (length % BLOCK_CAPACITY != 0)
    );

    int oldBlocks = file->totalBlocks;
    int freeSpace = availableBlocks();

    if (required > freeSpace + oldBlocks)
    {
        printf("Error: not enough disk space.\n");
        return;
    }

    int *replacement = NULL;

    if (required > 0)
    {
        replacement = malloc(required * sizeof(int));

        if (replacement == NULL)
        {
            perror("Memory allocation failed");
            return;
        }
    }

    int fromFree = required < freeSpace ? required : freeSpace;

    for (int i = 0; i < fromFree; i++)
    {
        replacement[i] = disk.available[disk.top - 1 - i];
    }

    for (int i = fromFree; i < required; i++)
    {
        replacement[i] = file->blockMap[i - fromFree];
    }

    disk.top -= fromFree;

    size_t position = 0;

    for (int i = 0; i < required; i++)
    {
        int block = replacement[i];

        memset(disk.storage[block], 0, BLOCK_CAPACITY);

        size_t remaining = length - position;

        size_t amount = remaining < BLOCK_CAPACITY
                        ? remaining : BLOCK_CAPACITY;

        memcpy(disk.storage[block], content + position, amount);

        position += amount;
    }

    for (int i = required - fromFree; i < oldBlocks; i++)
    {
        if (i >= 0)
        {
            returnBlock(file->blockMap[i]);
        }
    }

    free(file->blockMap);

    file->blockMap = replacement;
    file->totalBlocks = required;
    file->fileSize = length;

    printf("Data written successfully (size=%zu bytes).\n", length);
}

void displayFile(const char *name)
{
    FSNode *file = searchEntry(currentDir, name);

    if (file == NULL)
    {
        printf("Error: file '%s' not found.\n", name);
        return;
    }

    if (file->directory)
    {
        printf("Error: '%s' is a directory.\n", name);
        return;
    }

    if (file->fileSize == 0)
    {
        printf("(empty)\n");
        return;
    }

    size_t remaining = file->fileSize;

    for (int i = 0; i < file->totalBlocks && remaining > 0; i++)
    {
        size_t amount = remaining < BLOCK_CAPACITY
                        ? remaining : BLOCK_CAPACITY;

        fwrite(disk.storage[file->blockMap[i]], 1, amount, stdout);

        remaining -= amount;
    }

    putchar('\n');
}

void eraseFile(const char *name)
{
    FSNode *file = searchEntry(currentDir, name);

    if (file == NULL)
    {
        printf("Error: file '%s' not found.\n", name);
        return;
    }

    if (file->directory)
    {
        printf("Error: use rmdir for directories.\n");
        return;
    }

    detachEntry(currentDir, file);
    clearFileStorage(file);
    free(file);

    printf("File deleted successfully.\n");
}

void eraseDirectory(const char *name)
{
    FSNode *folder = searchEntry(currentDir, name);

    if (folder == NULL)
    {
        printf("Error: directory '%s' not found.\n", name);
        return;
    }

    if (!folder->directory)
    {
        printf("Error: '%s' is a file.\n", name);
        return;
    }

    if (folder->firstChild != NULL)
    {
        printf("Error: directory is not empty.\n");
        return;
    }

    detachEntry(currentDir, folder);
    free(folder);

    printf("Directory removed successfully.\n");
}

void printPath(FSNode *directory, char *buffer, size_t capacity)
{
    if (directory == rootDir)
    {
        snprintf(buffer, capacity, "/");
        return;
    }

    char parentPath[INPUT_LIMIT];

    printPath(directory->parent, parentPath, sizeof(parentPath));

    if (strcmp(parentPath, "/") == 0)
    {
        snprintf(buffer, capacity, "/%s", directory->filename);
    }
    else
    {
        snprintf(buffer, capacity, "%s/%s",
                 parentPath, directory->filename);
    }
}

void navigate(const char *name)
{
    if (strcmp(name, "/") == 0)
    {
        currentDir = rootDir;
    }
    else if (strcmp(name, "..") == 0)
    {
        if (currentDir != rootDir)
        {
            currentDir = currentDir->parent;
        }
    }
    else if (strcmp(name, ".") != 0)
    {
        FSNode *target = searchEntry(currentDir, name);

        if (target == NULL)
        {
            printf("Error: '%s' not found.\n", name);
            return;
        }

        if (!target->directory)
        {
            printf("Error: '%s' is not a directory.\n", name);
            return;
        }

        currentDir = target;
    }

    char path[INPUT_LIMIT];

    printPath(currentDir, path, sizeof(path));

    printf("Moved to %s\n", path);
}

void showContents(void)
{
    if (currentDir->firstChild == NULL)
    {
        printf("(empty)\n");
        return;
    }

    FSNode *item = currentDir->firstChild;

    do
    {
        printf("%s%s\n", item->filename,
               item->directory ? "/" : "");

        item = item->next;

    } while (item != currentDir->firstChild);
}

void showDiskInfo(void)
{
    int freeSpace = availableBlocks();
    int occupied = DISK_BLOCKS - freeSpace;

    printf("Total Blocks: %d\n", DISK_BLOCKS);
    printf("Used Blocks: %d\n", occupied);
    printf("Free Blocks: %d\n", freeSpace);

    printf("Disk Usage: %.2f%%\n",
           (double)occupied * 100.0 / DISK_BLOCKS);
}


/* ---------------- INPUT PROCESSING ---------------- */

char *skipWhitespace(char *text)
{
    while (*text == ' ' || *text == '\t')
    {
        text++;
    }

    return text;
}

void extractToken(char **source, char *destination, size_t limit)
{
    *source = skipWhitespace(*source);

    size_t index = 0;

    while (**source != '\0' &&
           **source != ' ' &&
           **source != '\t')
    {
        if (index + 1 < limit)
        {
            destination[index++] = **source;
        }

        (*source)++;
    }

    destination[index] = '\0';
}

void freeTree(FSNode *directory)
{
    if (directory == NULL)
    {
        return;
    }

    if (directory->firstChild != NULL)
    {
        FSNode *start = directory->firstChild;
        FSNode *item = start;

        do
        {
            FSNode *nextItem = item->next;

            freeTree(item);

            item = nextItem;

        } while (item != start);
    }

    free(directory->blockMap);
    free(directory);
}

void shutdownSystem(void)
{
    freeTree(rootDir);

    rootDir = NULL;
    currentDir = NULL;
}

void executeCommand(char *line)
{
    char command[32];
    char argument[INPUT_LIMIT];

    char *cursor = line;

    extractToken(&cursor, command, sizeof(command));

    if (command[0] == '\0')
    {
        return;
    }

    extractToken(&cursor, argument, sizeof(argument));

    if (strcmp(command, "exit") == 0)
    {
        return;
    }
    else if (strcmp(command, "mkdir") == 0)
    {
        createObject(argument, 1);
    }
    else if (strcmp(command, "create") == 0)
    {
        createObject(argument, 0);
    }
    else if (strcmp(command, "ls") == 0)
    {
        showContents();
    }
    else if (strcmp(command, "pwd") == 0)
    {
        char path[INPUT_LIMIT];

        printPath(currentDir, path, sizeof(path));
        puts(path);
    }
    else if (strcmp(command, "df") == 0)
    {
        showDiskInfo();
    }
    else if (strcmp(command, "cd") == 0)
    {
        navigate(argument);
    }
    else if (strcmp(command, "read") == 0)
    {
        displayFile(argument);
    }
    else if (strcmp(command, "delete") == 0)
    {
        eraseFile(argument);
    }
    else if (strcmp(command, "rmdir") == 0)
    {
        eraseDirectory(argument);
    }
    else if (strcmp(command, "write") == 0)
    {
        cursor = skipWhitespace(cursor);

        size_t length = strlen(cursor);

        if (length >= 2 &&
            cursor[0] == '"' &&
            cursor[length - 1] == '"')
        {
            cursor[length - 1] = '\0';
            cursor++;
        }

        writeData(argument, cursor);
    }
    else
    {
        printf("Unknown command: %s\n", command);
    }
}

int main(void)
{
    initializeDisk();

    rootDir = createEntry("/", 1, NULL);

    if (rootDir == NULL)
    {
        return EXIT_FAILURE;
    }

    currentDir = rootDir;

    printf("Virtual File System initialized successfully.\n");
    printf("Type 'exit' to terminate.\n");

    char input[INPUT_LIMIT];

    while (1)
    {
        char path[INPUT_LIMIT];

        printPath(currentDir, path, sizeof(path));

        printf("%s > ", path);
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            break;
        }

        input[strcspn(input, "\r\n")] = '\0';

        if (strcmp(skipWhitespace(input), "exit") == 0)
        {
            break;
        }

        executeCommand(input);
    }

    shutdownSystem();

    printf("Memory released. Exiting program...\n");

    return EXIT_SUCCESS;
}
