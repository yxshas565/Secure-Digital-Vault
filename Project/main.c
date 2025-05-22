#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>

#define CONFIG_FILE "vault_config.config"
#define VAULT_DIR "vault_files"

#define MAX_INPUT 128

int list_vault_files_and_select(char *selected_file, size_t maxlen) {
    DIR *dir = opendir(VAULT_DIR);
    if (!dir) {
        perror("Failed to open vault_files directory");
        return 0;
    }
    struct dirent *entry;
    int index = 1;
    char file_list[100][256];

    printf("\nVault Files:\n");
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        printf("%d) %s\n", index, entry->d_name);
        strncpy(file_list[index - 1], entry->d_name, 255);
        file_list[index - 1][255] = '\0';
        index++;
        if (index > 100) break;
    }
    closedir(dir);

    if (index == 1) {
        printf("No files found in vault.\n");
        return 0;
    }

    char input[16];
    int choice = 0;
    while (1) {
        printf("Choose a file from the list by entering it's number : ");
        if (!fgets(input, sizeof(input), stdin)) return 0;
        input[strcspn(input, "\n")] = 0;

        int valid = 1;
        for (size_t i = 0; i < strlen(input); i++) {
            if (!isdigit((unsigned char)input[i])) valid = 0;
        }
        if (!valid) {
            printf("Invalid input. Please enter a number.\n");
            continue;
        }
        choice = atoi(input);
        if (choice < 1 || choice >= index) {
            printf("Invalid choice. Try again.\n");
            continue;
        }
        break;
    }

    strncpy(selected_file, file_list[choice - 1], maxlen - 1);
    selected_file[maxlen - 1] = '\0';
    return 1;
}

void display_file_contents(const char *filename) {
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s/%s", VAULT_DIR, filename);

    FILE *f = fopen(filepath, "r");
    if (!f) {
        printf("Could not open file '%s'.\n", filename);
        return;
    }

    printf("\nContents of %s:\n\n", filename);
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        printf("%s", line);
    }
    fclose(f);
}

void file_menu_loop() {
    while (1) {
        printf("\n");
        printf("\n--- Vault File Menu ---\n");
        printf("1) List and open a file\n");
        printf("2) Exit\n");
        printf("Enter choice: ");

        char input[16];
        if (!fgets(input, sizeof(input), stdin)) {
            printf("Error reading input.\n");
            continue;
        }
        input[strcspn(input, "\n")] = 0;

        if (strcmp(input, "1") == 0) {
            char selected_file[256];
            if (list_vault_files_and_select(selected_file, sizeof(selected_file))) {
                display_file_contents(selected_file);
            }
        } else if (strcmp(input, "2") == 0) {
            printf("Exiting vault. Goodbye!\n");
            break;
        } else {
            printf("Invalid choice. Try again.\n");
        }
    }
}

void get_input(const char *prompt, char *buffer, size_t size) {
    printf("%s", prompt);
    if (fgets(buffer, (int)size, stdin) == NULL) {
        fprintf(stderr, "Input error.\n");
        exit(1);
    }
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n')
        buffer[len - 1] = '\0';
}

void write_config(const char *p1, const char *p2, const char *master,
                  const char *q1, const char *a1,
                  const char *q2, const char *a2) {
    FILE *f = fopen(CONFIG_FILE, "w");
    if (!f) {
        perror("Error opening config file for writing");
        exit(1);
    }
    fprintf(f, "password1=%s\n", p1);
    fprintf(f, "password2=%s\n", p2);
    fprintf(f, "master=%s\n", master);
    fprintf(f, "question1=%s\n", q1);
    fprintf(f, "answer1=%s\n", a1);
    fprintf(f, "question2=%s\n", q2);
    fprintf(f, "answer2=%s\n", a2);
    fclose(f);
}

int read_config_value(const char *key, char *out, size_t maxlen) {
    FILE *f = fopen(CONFIG_FILE, "r");
    if (!f) return 0;
    char line[256];
    size_t keylen = strlen(key);
    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, key, keylen) == 0 && line[keylen] == '=') {
            strncpy(out, line + keylen + 1, maxlen);
            size_t len = strlen(out);
            if (len > 0 && (out[len - 1] == '\n' || out[len - 1] == '\r'))
                out[len - 1] = '\0';
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}

typedef struct {
    const char *question;
    const char *options;
    char correct_option;
} Puzzle;

int puzzle() {
    static int seeded = 0;
    if (!seeded) {
        srand((unsigned int)time(NULL));
        seeded = 1;
    }

    Puzzle puzzles[] = {
        {
            "What is 7 * 6?",
            "a) 42\nb) 36\nc) 48\nd) 40\n",
            'a'
        },
        {
            "What is the capital of France?",
            "a) Berlin\nb) London\nc) Paris\nd) Rome\n",
            'c'
        },
        {
            "Which planet is known as the Red Planet?",
            "a) Venus\nb) Mars\nc) Jupiter\nd) Saturn\n",
            'b'
        },
        {
            "What is the square root of 81?",
            "a) 7\nb) 8\nc) 9\nd) 10\n",
            'c'
        },
        {
            "What is 15 + 26?",
            "a) 40\nb) 41\nc) 42\nd) 43\n",
            'b'
        }
    };

    int n = sizeof(puzzles) / sizeof(puzzles[0]);
    int choice = rand() % n;
    Puzzle p = puzzles[choice];

    printf("\nPuzzle Time! Solve this:\n");
    printf("%s\n", p.question);
    printf("%s", p.options);

    char ans[MAX_INPUT];
    get_input("Your answer (a/b/c/d): ", ans, sizeof(ans));

    if (tolower(ans[0]) == p.correct_option) {
        printf("Correct!\n");
        return 1;
    }
    printf("Wrong answer.\n");
    return 0;
}

int verify(const char *prompt, const char *stored) {
    char input[MAX_INPUT];
    get_input(prompt, input, sizeof(input));
    return (strcmp(input, stored) == 0);
}

void setup() {
    printf("=== Vault Setup ===\n");
    char pass1[MAX_INPUT], pass2[MAX_INPUT], master[MAX_INPUT];
    char q1[MAX_INPUT], a1[MAX_INPUT];
    char q2[MAX_INPUT], a2[MAX_INPUT];

    get_input("Set Password 1: ", pass1, sizeof(pass1));
    get_input("Set Password 2: ", pass2, sizeof(pass2));
    get_input("Set Master Password: ", master, sizeof(master));

    get_input("Security Question 1: ", q1, sizeof(q1));
    get_input("Answer 1: ", a1, sizeof(a1));

    get_input("Security Question 2: ", q2, sizeof(q2));
    get_input("Answer 2: ", a2, sizeof(a2));

    write_config(pass1, pass2, master, q1, a1, q2, a2);
    printf("Setup complete! Run the program again to unlock the vault.\n");
}

void reset() {
    printf("\n=== Vault Reset ===\n");
    char stored_master[MAX_INPUT];
    if (!read_config_value("master", stored_master, sizeof(stored_master))) {
        printf("No vault config found. Run setup first.\n");
        return;
    }
    if (!verify("Enter Master Password to reset: ", stored_master)) {
        printf("Master password incorrect. Abort reset.\n");
        return;
    }
    setup();
}

void unlock() {
    char stored_p1[MAX_INPUT], stored_p2[MAX_INPUT], stored_master[MAX_INPUT];
    char q1[MAX_INPUT], q2[MAX_INPUT];
    char stored_a1[MAX_INPUT], stored_a2[MAX_INPUT];

    if (!read_config_value("password1", stored_p1, sizeof(stored_p1)) ||
        !read_config_value("password2", stored_p2, sizeof(stored_p2)) ||
        !read_config_value("master", stored_master, sizeof(stored_master)) ||
        !read_config_value("question1", q1, sizeof(q1)) ||
        !read_config_value("answer1", stored_a1, sizeof(stored_a1)) ||
        !read_config_value("question2", q2, sizeof(q2)) ||
        !read_config_value("answer2", stored_a2, sizeof(stored_a2))) {
        printf("Vault config missing or corrupted. Please run setup.\n");
        return;
    }

    printf("=== Unlock Vault ===\n");
    if (!verify("Enter Password 1: ", stored_p1)) {
        printf("Password 1 incorrect.\n");
        return;
    }

    if (!puzzle()) {
        return;
    }

    if (!verify("Enter Password 2: ", stored_p2)) {
        printf("Password 2 incorrect.\n");
        return;
    }

    printf("Answer Security Question 1: %s\n", q1);
    if (!verify("Answer: ", stored_a1)) {
        printf("Security answer 1 incorrect.\n");
        return;
    }

    printf("Answer Security Question 2: %s\n", q2);
    if (!verify("Answer: ", stored_a2)) {
        printf("Security answer 2 incorrect.\n");
        return;
    }

    printf("Vault unlocked!\n");
    file_menu_loop();
}

int main() {
    if (access(CONFIG_FILE, F_OK) == -1) {
        setup();
        return 0;
    }

    printf("Welcome to the Secure Vault!\n");
    while (1) {
        printf("\n1) Unlock Vault\n2) Reset Vault\n3) Exit\nEnter choice: ");
        char input[16];
        if (!fgets(input, sizeof(input), stdin)) {
            printf("Input error.\n");
            continue;
        }
        input[strcspn(input, "\n")] = 0;
        if (strcmp(input, "1") == 0) {
            unlock();
        } else if (strcmp(input, "2") == 0) {
            reset();
        } else if (strcmp(input, "3") == 0) {
            printf("Goodbye!\n");
            break;
        } else {
            printf("Invalid choice.\n");
        }
    }
    return 0;
}
