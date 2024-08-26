#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <curl/curl.h>

#define LINE_BUFFER_SIZE 256
#define NUM_LINES 5
#define SLEEP_INTERVAL 5  

void send_telegram_message(const char *token, const char *chat_id, const char *message) {
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if (curl) {
        char url[512];
        snprintf(url, sizeof(url), "https://api.telegram.org/bot%s/sendMessage", token);

        char post_fields[1024];
        snprintf(post_fields, sizeof(post_fields), "chat_id=%s&text=%s", chat_id, message);

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_fields);
        res = curl_easy_perform(curl);

        if (res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

        curl_easy_cleanup(curl);
    }
}

int read_last_lines(const char *filename, char lines[][LINE_BUFFER_SIZE], int num_lines) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("fopen");
        return -1;
    }

    int total_lines = 0;
    char buffer[LINE_BUFFER_SIZE];

    while (fgets(buffer, LINE_BUFFER_SIZE, file) != NULL) {
        total_lines++;
    }

    fseek(file, 0, SEEK_SET);

    int start_line = total_lines > num_lines ? total_lines - num_lines : 0;
    int current_line = 0;

    while (fgets(buffer, LINE_BUFFER_SIZE, file) != NULL) {
        if (current_line >= start_line) {
            strncpy(lines[current_line - start_line], buffer, LINE_BUFFER_SIZE);
        }
        current_line++;
    }

    fclose(file);
    return 0;
}

int find_and_send_opened_line(char lines[][LINE_BUFFER_SIZE], int num_lines, const char *token, const char *chat_id, char *last_message) {
    for (int i = 0; i < num_lines; i++) {
        if (strstr(lines[i], "opened") != NULL && strstr(lines[i], "sshd:session") != NULL ) {
            if (strcmp(lines[i], last_message) != 0) {
                send_telegram_message(token, chat_id, lines[i]);
                strncpy(last_message, lines[i], LINE_BUFFER_SIZE);
            }
            return 1;
        }
    }
    return 0;
}

int main() {
    const char *filename = "/var/log/auth.log";
    char lines[NUM_LINES][LINE_BUFFER_SIZE] = {0};
    char last_message[LINE_BUFFER_SIZE] = {0}; 

    const char *token = getenv("TELEGRAM_BOT_TOKEN");
    const char *chat_id = getenv("TELEGRAM_CHAT_ID");

    if (!token) {
        fprintf(stderr, "TELEGRAM_BOT_TOKEN environment variable is not set\n");
        return 1;
    }

    if (!chat_id) {
        fprintf(stderr, "TELEGRAM_CHAT_ID environment variable is not set\n");
        return 1;
    }

    while (1) {
        if (read_last_lines(filename, lines, NUM_LINES) == 0) {
            find_and_send_opened_line(lines, NUM_LINES, token, chat_id, last_message);
        }

        sleep(SLEEP_INTERVAL); 
    }

    return 0;
}
