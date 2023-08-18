#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>

#define MAX_INPUT_SIZE 1024
#define TRIE_SIZE 26 // Assuming only lowercase letters for simplicity

// Trie structure for auto-suggestions
typedef struct TrieNode {
    struct TrieNode *children[TRIE_SIZE];
    int isEndOfWord;
} TrieNode;

TrieNode* createNode() {
    TrieNode *newNode = malloc(sizeof(TrieNode));
    newNode->isEndOfWord = 0;
    for (int i = 0; i < TRIE_SIZE; i++) {
        newNode->children[i] = NULL;
    }
    return newNode;
}

void insert(TrieNode *root, const char *word) {
    int length = strlen(word);
    TrieNode *current = root;

    for (int i = 0; i < length; i++) {
        int index = word[i] - 'a';
        if (!current->children[index]) {
            current->children[index] = createNode();
        }
        current = current->children[index];
    }

    current->isEndOfWord = 1;
}

int search(TrieNode *root, const char *word) {
    int length = strlen(word);
    TrieNode *current = root;

    for (int i = 0; i < length; i++) {
        int index = word[i] - 'a';
        if (!current->children[index]) {
            return 0;
        }
        current = current->children[index];
    }

    return (current != NULL && current->isEndOfWord);
}

void signalHandler(int sig_num) {
    if (sig_num == SIGINT) {
        printf("\nInterrupted.\n");
    }
}

int main() {
    char input[MAX_INPUT_SIZE];
    TrieNode *commandsRoot = createNode();

    // Insert the commands into Trie
    insert(commandsRoot, "ls");
    insert(commandsRoot, "cd");
    insert(commandsRoot, "sleep");

    signal(SIGINT, signalHandler);

    while (1) {
        printf("CustomShell> ");
        fgets(input, sizeof(input), stdin);

        // Removing trailing newline character
        input[strcspn(input, "\n")] = 0;

        if (search(commandsRoot, input)) {
            pid_t pid = fork();

            if (pid == 0) {
                // Child process
                if (strcmp(input, "ls") == 0) {
                    execlp("/bin/ls", "ls", NULL);
                } else if (strcmp(input, "cd") == 0) {
                    // cd requires special handling, so we don't fork for it
                    printf("'cd' requires an argument, e.g., 'cd /path/to/directory'\n");
                    exit(0);
                } else if (strncmp(input, "sleep", 5) == 0) {
                    execlp("/bin/sleep", "sleep", input + 6, NULL);
                }
            } else {
                // Parent process
                wait(NULL); // Wait for child process to terminate
            }
        } else {
            printf("Command not found or unsupported: %s\n", input);
        }
    }

    return 0;
}
