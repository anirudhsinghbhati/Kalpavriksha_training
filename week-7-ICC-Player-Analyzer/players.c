#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "Players_data.h"

#define MAX_TEAMS               10
#define MAX_PLAYERS_PER_TEAM    50
#define MIN_ID                  1
#define MAX_ID                  1000
#define MAX_NAME_LENGTH         50
#define INPUT_BUFFER_SIZE       256

typedef enum {
    ROLE_BATSMAN = 1,
    ROLE_BOWLER = 2,
    ROLE_ALL_ROUNDER = 3
} PlayerRole;

typedef struct PlayerNode {
    int playerId;
    char name[MAX_NAME_LENGTH + 1];
    char teamName[MAX_NAME_LENGTH + 1];
    PlayerRole role;
    int totalRuns;
    float battingAverage;
    float strikeRate;
    int wickets;
    float economyRate;
    float performanceIndex;
    struct PlayerNode *nextInRole;   
    struct PlayerNode *nextInTeam;   
} PlayerNode;

typedef struct {
    int teamId;
    char name[MAX_NAME_LENGTH + 1];
    int totalPlayers;
    double strikeRateSum;
    int strikeRateCount;
    double averageBattingStrikeRate;
    PlayerNode *roleHeads[4];       
    PlayerNode *teamHead;
    PlayerNode *teamTail;
} Team;

static Team teamTable[MAX_TEAMS];    
static int teamTotal = 0;

static int readLine(char *buffer, int size)
{
    if (fgets(buffer, size, stdin) == NULL) {
        return 0;
    }
    size_t length = strlen(buffer);
    if (length > 0 && buffer[length - 1] == '\n') {
        buffer[length - 1] = '\0';
    } else {
        int discardedChar;
        while ((discardedChar = getchar()) != '\n' && discardedChar != EOF) {
        }
    }
    return 1;
}

static void exitOnEndOfInput(void)
{
    printf("\nInput closed. Exiting.\n");
    exit(0);
}

static int readIntegerInRange(const char *prompt, int minimum, int maximum)
{
    char buffer[INPUT_BUFFER_SIZE];
    while (1) {
        printf("%s", prompt);
        if (!readLine(buffer, sizeof(buffer))) {
            exitOnEndOfInput();
        }
        char *endPointer;
        long value = strtol(buffer, &endPointer, 10);
        if (endPointer == buffer) {
            printf("Invalid input. Enter a whole number.\n");
            continue;
        }
        while (isspace((unsigned char)*endPointer)) {
            endPointer++;
        }
        if (*endPointer != '\0') {
            printf("Invalid input. Enter a whole number.\n");
            continue;
        }
        if (value < minimum || value > maximum) {
            printf("Value must be between %d and %d.\n", minimum, maximum);
            continue;
        }
        return (int)value;
    }
}

static float readFloatInRange(const char *prompt, float minimum, float maximum)
{
    char buffer[INPUT_BUFFER_SIZE];
    while (1) {
        printf("%s", prompt);
        if (!readLine(buffer, sizeof(buffer))) {
            exitOnEndOfInput();
        }
        char *endPointer;
        double value = strtod(buffer, &endPointer);
        if (endPointer == buffer) {
            printf("Invalid input. Enter a number.\n");
            continue;
        }
        while (isspace((unsigned char)*endPointer)) {
            endPointer++;
        }
        if (*endPointer != '\0') {
            printf("Invalid input. Enter a number.\n");
            continue;
        }
        if (value < minimum || value > maximum) {
            printf("Value must be between %.1f and %.1f.\n", minimum, maximum);
            continue;
        }
        return (float)value;
    }
}

static void readPlayerName(const char *prompt, char *destination)
{
    char buffer[INPUT_BUFFER_SIZE];
    while (1) {
        printf("%s", prompt);
        if (!readLine(buffer, sizeof(buffer))) {
            exitOnEndOfInput();
        }
        size_t start = 0;
        while (isspace((unsigned char)buffer[start])) {
            start++;
        }
        size_t length = strlen(buffer + start);
        while (length > 0 && isspace((unsigned char)buffer[start + length - 1])) {
            length--;
        }
        if (length < 1) {
            printf("Name cannot be empty.\n");
            continue;
        }
        if (length > MAX_NAME_LENGTH) {
            printf("Name too long (max %d characters).\n", MAX_NAME_LENGTH);
            continue;
        }
        memcpy(destination, buffer + start, length);
        destination[length] = '\0';
        return;
    }
}

static const char *roleToText(PlayerRole role)
{
    switch (role) {
        case ROLE_BATSMAN:     return "Batsman";
        case ROLE_BOWLER:      return "Bowler";
        case ROLE_ALL_ROUNDER: return "All-rounder";
    }
    return "Unknown";
}

static PlayerRole textToRole(const char *roleText)
{
    char firstLetter = (char)tolower((unsigned char)roleText[0]);
    char secondLetter = (char)tolower((unsigned char)roleText[1]);
    if (firstLetter == 'a') {
        return ROLE_ALL_ROUNDER;
    }
    if (firstLetter == 'b' && secondLetter == 'a') {
        return ROLE_BATSMAN;
    }
    return ROLE_BOWLER;
}

static int countsForBattingStrikeRate(PlayerRole role)
{
    return role == ROLE_BATSMAN || role == ROLE_ALL_ROUNDER;
}

static float calculatePerformanceIndex(PlayerRole role, float battingAverage, float strikeRate,
                                       int wickets, float economyRate)
{
    float battingScore = (battingAverage * strikeRate) / 100.0f;
    switch (role) {
        case ROLE_BATSMAN:     return battingScore;
        case ROLE_BOWLER:      return (wickets * 2.0f) + (100.0f - economyRate);
        case ROLE_ALL_ROUNDER: return battingScore + (wickets * 2.0f);
    }
    return 0.0f;
}

static Team *findTeamById(int teamId)
{
    int lowIndex = 0;
    int highIndex = teamTotal - 1;
    while (lowIndex <= highIndex) {
        int middleIndex = lowIndex + (highIndex - lowIndex) / 2;
        if (teamTable[middleIndex].teamId == teamId) {
            return &teamTable[middleIndex];
        }
        if (teamTable[middleIndex].teamId < teamId) {
            lowIndex = middleIndex + 1;
        } else {
            highIndex = middleIndex - 1;
        }
    }
    return NULL;
}

static int playerIdExists(int playerId)
{
    for (int teamIndex = 0; teamIndex < teamTotal; teamIndex++) {
        for (PlayerNode *current = teamTable[teamIndex].teamHead; current != NULL; current = current->nextInTeam) {
            if (current->playerId == playerId) {
                return 1;
            }
        }
    }
    return 0;
}

static void insertPlayerIntoTeam(Team *team, PlayerNode *newPlayer)
{
    PlayerNode **link = &team->roleHeads[newPlayer->role];
    while (*link != NULL && (*link)->performanceIndex >= newPlayer->performanceIndex) {
        link = &(*link)->nextInRole;
    }
    newPlayer->nextInRole = *link;
    *link = newPlayer;

    newPlayer->nextInTeam = NULL;
    if (team->teamTail == NULL) {
        team->teamHead = newPlayer;
    } else {
        team->teamTail->nextInTeam = newPlayer;
    }
    team->teamTail = newPlayer;

    team->totalPlayers++;
    if (countsForBattingStrikeRate(newPlayer->role)) {
        team->strikeRateSum += newPlayer->strikeRate;
        team->strikeRateCount++;
        team->averageBattingStrikeRate = team->strikeRateSum / team->strikeRateCount;
    }
}

static PlayerNode *createPlayerNode(int playerId, const char *name, const char *teamName, PlayerRole role,
                                    int totalRuns, float battingAverage, float strikeRate,
                                    int wickets, float economyRate)
{
    PlayerNode *node = (PlayerNode *)calloc(1, sizeof(PlayerNode));
    if (node == NULL) {
        printf("Memory allocation failed.\n");
        exit(1);
    }
    node->playerId = playerId;
    strncpy(node->name, name, MAX_NAME_LENGTH);
    node->name[MAX_NAME_LENGTH] = '\0';
    strncpy(node->teamName, teamName, MAX_NAME_LENGTH);
    node->teamName[MAX_NAME_LENGTH] = '\0';
    node->role = role;
    node->totalRuns = totalRuns;
    node->battingAverage = battingAverage;
    node->strikeRate = strikeRate;
    node->wickets = wickets;
    node->economyRate = economyRate;
    node->performanceIndex = calculatePerformanceIndex(role, battingAverage, strikeRate, wickets, economyRate);
    return node;
}

static void initializeData(void)
{
    teamTotal = teamCount < MAX_TEAMS ? teamCount : MAX_TEAMS;
    for (int teamIndex = 0; teamIndex < teamTotal; teamIndex++) {
        Team *team = &teamTable[teamIndex];
        memset(team, 0, sizeof(Team));
        team->teamId = teamIndex + 1;
        strncpy(team->name, teams[teamIndex], MAX_NAME_LENGTH);
        team->name[MAX_NAME_LENGTH] = '\0';
    }

    for (int playerIndex = 0; playerIndex < playerCount; playerIndex++) {
        Team *owningTeam = NULL;
        for (int teamIndex = 0; teamIndex < teamTotal; teamIndex++) {
            if (strcmp(teamTable[teamIndex].name, players[playerIndex].team) == 0) {
                owningTeam = &teamTable[teamIndex];
                break;
            }
        }
        if (owningTeam == NULL) {
            printf("Warning: skipped player '%s' (unknown team).\n",
                   players[playerIndex].name);
            continue;
        }
        PlayerNode *node = createPlayerNode(
            players[playerIndex].id,
            players[playerIndex].name,
            owningTeam->name,
            textToRole(players[playerIndex].role),
            players[playerIndex].totalRuns,
            players[playerIndex].battingAverage,
            players[playerIndex].strikeRate,
            players[playerIndex].wickets,
            players[playerIndex].economyRate);
        insertPlayerIntoTeam(owningTeam, node);
    }
}

static void printPlayerHeaderWithoutTeam(void)
{
    printf("====================================================================================\n");
    printf("%-6s %-26s %-12s %8s %6s %6s %5s %5s %10s\n",
           "ID", "Name", "Role", "Runs", "Avg", "SR", "Wkts", "ER", "Perf.Index");
    printf("====================================================================================\n");
}

static void printPlayerRowWithoutTeam(const PlayerNode *player)
{
    printf("%-6d %-26s %-12s %8d %6.1f %6.1f %5d %5.1f %10.2f\n",
           player->playerId, player->name, roleToText(player->role), player->totalRuns,
           player->battingAverage, player->strikeRate, player->wickets,
           player->economyRate, player->performanceIndex);
}

static void printPlayerHeaderWithTeam(void)
{
    printf("==============================================================================================================\n");
    printf("%-6s %-24s %-16s %-12s %7s %6s %6s %5s %5s %10s\n",
           "ID", "Name", "Team", "Role", "Runs", "Avg", "SR", "Wkts", "ER", "Perf.Index");
    printf("==============================================================================================================\n");
}

static void printPlayerRowWithTeam(const PlayerNode *player)
{
    printf("%-6d %-24s %-16s %-12s %7d %6.1f %6.1f %5d %5.1f %10.2f\n",
           player->playerId, player->name, player->teamName, roleToText(player->role),
           player->totalRuns, player->battingAverage, player->strikeRate, player->wickets,
           player->economyRate, player->performanceIndex);
}

static int readValidTeam(Team **foundTeam)
{
    int teamId = readIntegerInRange("Enter Team ID: ", MIN_ID, MAX_ID);
    *foundTeam = findTeamById(teamId);
    if (*foundTeam == NULL) {
        printf("Team with ID %d does not exist.\n", teamId);
        return 0;
    }
    return 1;
}

static PlayerRole readRole(void)
{
    return (PlayerRole)readIntegerInRange("Enter Role (1-Batsman, 2-Bowler, 3-All-rounder): ", 1, 3);
}

static void addPlayerToTeam(void)
{
    int teamId = readIntegerInRange("Enter Team ID to add player: ", MIN_ID, MAX_ID);
    Team *team = findTeamById(teamId);
    if (team == NULL) {
        printf("Team with ID %d does not exist.\n", teamId);
        return;
    }
    if (team->totalPlayers >= MAX_PLAYERS_PER_TEAM) {
        printf("Team %s already has the maximum of %d players.\n", team->name, MAX_PLAYERS_PER_TEAM);
        return;
    }

    printf("Enter Player Details:\n");
    int playerId;
    while (1) {
        playerId = readIntegerInRange("Player ID: ", MIN_ID, MAX_ID);
        if (playerIdExists(playerId)) {
            printf("Player ID %d already exists. Enter a unique ID.\n", playerId);
            continue;
        }
        break;
    }

    char name[MAX_NAME_LENGTH + 1];
    readPlayerName("Name: ", name);
    PlayerRole role = (PlayerRole)readIntegerInRange("Role (1-Batsman, 2-Bowler, 3-All-rounder): ", 1, 3);
    int totalRuns = readIntegerInRange("Total Runs: ", 0, 100000);
    float battingAverage = readFloatInRange("Batting Average: ", 0.0f, 200.0f);
    float strikeRate = readFloatInRange("Strike Rate: ", 0.0f, 500.0f);
    int wickets = readIntegerInRange("Wickets: ", 0, 2000);
    float economyRate = readFloatInRange("Economy Rate: ", 0.0f, 50.0f);

    PlayerNode *newPlayer = createPlayerNode(playerId, name, team->name, role, totalRuns,
                                             battingAverage, strikeRate, wickets, economyRate);
    insertPlayerIntoTeam(team, newPlayer);
    printf("Player added successfully to Team %s!\n", team->name);
}

static void displayPlayersOfTeam(void)
{
    Team *team;
    if (!readValidTeam(&team)) {
        return;
    }
    printf("Players of Team %s:\n", team->name);
    printPlayerHeaderWithoutTeam();
    for (PlayerNode *current = team->teamHead; current != NULL; current = current->nextInTeam) {
        printPlayerRowWithoutTeam(current);
    }
    printf("====================================================================================\n");
    printf("Total Players: %d\n", team->totalPlayers);
    printf("Average Batting Strike Rate: %.2f\n", team->averageBattingStrikeRate);
}

static void displayTeamsByAverageStrikeRate(void)
{
    Team *sortedTeams[MAX_TEAMS];
    for (int teamIndex = 0; teamIndex < teamTotal; teamIndex++) {
        sortedTeams[teamIndex] = &teamTable[teamIndex];
    }
    /* insertion sort, descending (only 10 teams) */
    for (int outerIndex = 1; outerIndex < teamTotal; outerIndex++) {
        Team *keyTeam = sortedTeams[outerIndex];
        int innerIndex = outerIndex - 1;
        while (innerIndex >= 0 &&
               sortedTeams[innerIndex]->averageBattingStrikeRate < keyTeam->averageBattingStrikeRate) {
            sortedTeams[innerIndex + 1] = sortedTeams[innerIndex];
            innerIndex--;
        }
        sortedTeams[innerIndex + 1] = keyTeam;
    }

    printf("Teams Sorted by Average Batting Strike Rate\n");
    printf("=========================================================\n");
    printf("%-4s %-20s %-14s %s\n", "ID", "Team Name", "Avg Bat SR", "Total Players");
    printf("=========================================================\n");
    for (int teamIndex = 0; teamIndex < teamTotal; teamIndex++) {
        printf("%-4d %-20s %-14.2f %d\n", sortedTeams[teamIndex]->teamId, sortedTeams[teamIndex]->name,
               sortedTeams[teamIndex]->averageBattingStrikeRate, sortedTeams[teamIndex]->totalPlayers);
    }
    printf("=========================================================\n");
}

static void displayTopKPlayersOfTeam(void)
{
    Team *team;
    if (!readValidTeam(&team)) {
        return;
    }
    PlayerRole role = readRole();
    int requestedCount = readIntegerInRange("Enter number of players: ", 1, MAX_PLAYERS_PER_TEAM);

    if (team->roleHeads[role] == NULL) {
        printf("Team %s has no %s players.\n", team->name, roleToText(role));
        return;
    }

    printf("Top %d %s players of Team %s:\n", requestedCount, roleToText(role), team->name);
    printPlayerHeaderWithoutTeam();
    int printedCount = 0;
    for (PlayerNode *current = team->roleHeads[role];
         current != NULL && printedCount < requestedCount;
         current = current->nextInRole) {
        printPlayerRowWithoutTeam(current);
        printedCount++;
    }
    printf("====================================================================================\n");
    if (printedCount < requestedCount) {
        printf("Only %d %s player(s) available in this team.\n", printedCount, roleToText(role));
    }
}

static int hasHigherPriority(const PlayerNode *first, const PlayerNode *second)
{
    if (first->performanceIndex != second->performanceIndex) {
        return first->performanceIndex > second->performanceIndex;
    }
    return first->playerId < second->playerId;
}

static void heapSiftDown(PlayerNode **heap, int heapSize, int startIndex)
{
    int parentIndex = startIndex;
    while (1) {
        int leftChildIndex = 2 * parentIndex + 1;
        int rightChildIndex = 2 * parentIndex + 2;
        int largestIndex = parentIndex;
        if (leftChildIndex < heapSize && hasHigherPriority(heap[leftChildIndex], heap[largestIndex])) {
            largestIndex = leftChildIndex;
        }
        if (rightChildIndex < heapSize && hasHigherPriority(heap[rightChildIndex], heap[largestIndex])) {
            largestIndex = rightChildIndex;
        }
        if (largestIndex == parentIndex) {
            return;
        }
        PlayerNode *temporaryNode = heap[parentIndex];
        heap[parentIndex] = heap[largestIndex];
        heap[largestIndex] = temporaryNode;
        parentIndex = largestIndex;
    }
}

static void displayAllPlayersOfRole(void)
{
    PlayerRole role = readRole();

    PlayerNode *heap[MAX_TEAMS];
    int heapSize = 0;
    for (int teamIndex = 0; teamIndex < teamTotal; teamIndex++) {
        if (teamTable[teamIndex].roleHeads[role] != NULL) {
            heap[heapSize++] = teamTable[teamIndex].roleHeads[role];
        }
    }
    if (heapSize == 0) {
        printf("No %s players found.\n", roleToText(role));
        return;
    }
    for (int buildIndex = heapSize / 2 - 1; buildIndex >= 0; buildIndex--) {
        heapSiftDown(heap, heapSize, buildIndex);
    }

    printf("%s players of all teams:\n", roleToText(role));
    printPlayerHeaderWithTeam();
    while (heapSize > 0) {
        PlayerNode *bestPlayer = heap[0];
        printPlayerRowWithTeam(bestPlayer);
        if (bestPlayer->nextInRole != NULL) {
            heap[0] = bestPlayer->nextInRole;
        } else {
            heap[0] = heap[heapSize - 1];
            heapSize--;
        }
        if (heapSize > 0) {
            heapSiftDown(heap, heapSize, 0);
        }
    }
    printf("==============================================================================================================\n");
}

static void freeAllData(void)
{
    for (int teamIndex = 0; teamIndex < teamTotal; teamIndex++) {
        PlayerNode *current = teamTable[teamIndex].teamHead;
        while (current != NULL) {
            PlayerNode *nextNode = current->nextInTeam;
            free(current);
            current = nextNode;
        }
        teamTable[teamIndex].teamHead = NULL;
        teamTable[teamIndex].teamTail = NULL;
    }
}

static void printMenu(void)
{
    printf("\n==============================================================================\n");
    printf("ICC ODI Player Performance Analyzer\n");
    printf("==============================================================================\n");
    printf("1. Add Player to Team\n");
    printf("2. Display Players of a Specific Team\n");
    printf("3. Display Teams by Average Batting Strike Rate\n");
    printf("4. Display Top K Players of a Specific Team by Role\n");
    printf("5. Display all Players of specific role Across All Teams by performance index\n");
    printf("6. Exit\n");
    printf("==============================================================================\n");
}

int main(void)
{
    initializeData();

    int menuChoice;
    do {
        printMenu();
        menuChoice = readIntegerInRange("Enter your choice: ", 1, 6);
        switch (menuChoice) {
            case 1: addPlayerToTeam();               break;
            case 2: displayPlayersOfTeam();          break;
            case 3: displayTeamsByAverageStrikeRate(); break;
            case 4: displayTopKPlayersOfTeam();      break;
            case 5: displayAllPlayersOfRole();       break;
            case 6: printf("Exiting. Goodbye!\n");   break;
        }
    } while (menuChoice != 6);

    freeAllData();
    return 0;
}