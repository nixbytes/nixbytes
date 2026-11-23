#include "summary.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int run_command(const char *cmd, char *output, size_t max_len) {
    FILE *fp = popen(cmd, "r");
    if (fp == NULL) {
        perror("popen failed");
        return -1;
    }

    if (fgets(output, max_len, fp) != NULL) {
        output[strcspn(output, "\n")] = 0;
        pclose(fp);
        return 0;
    }

    pclose(fp);
    return -1;
}

void get_repo_name(char *output, size_t max_len) {
    char cmd[MAX_BUFFER];
    snprintf(cmd, sizeof(cmd), "git remote get-url origin");
    if (run_command(cmd, output, max_len) != 0) {
        strncpy(output, "Unknown Repo", max_len);
    } else {
        char *last_slash = strrchr(output, '/');
        if (last_slash) {
            char *name = last_slash + 1;
            char *dot_git = strstr(name, ".git");
            if (dot_git) {
                *dot_git = '\0';
            }
        }
    }
}

void get_commit_count(char *output, size_t max_len) {
    run_command("git rev-list --all --count", output, max_len);
    if (strlen(output) == 0) strcpy(output, "0");
}

void get_latest_commit(char *output, size_t max_len) {
    run_command("git log -1 --pretty=format:\"%s\"", output, max_len);
    if (strlen(output) == 0) strcpy(output, "No commits yet");
}

int get_file_count() {
    char output[MAX_BUFFER];
    run_command("git ls-files | wc -l", output, sizeof(output));
    return atoi(output);
}

int get_line_count() {
    char output[MAX_BUFFER];
    run_command("git ls-files | xargs wc -l | tail -n 1 | awk '{print $1}'", output, sizeof(output));
    return atoi(output);
}

const char* get_skill_level_simple(int files, int langs) {
    if (files < 5 && langs < 2) return "Beginner";
    if (files < 20 && langs < 5) return "Intermediate";
    return "Advanced";
}

void get_activity_chart(char *output, size_t max_len) {
    int files = get_file_count();
    int bar_len = (files * CHART_WIDTH) / 100;
    if (bar_len < 1) bar_len = 1;
    if (bar_len > CHART_WIDTH) bar_len = CHART_WIDTH;

    int pos = 0;
    output[pos++] = '[';
    for (int i = 0; i < bar_len; i++) output[pos++] = '#';
    for (int i = bar_len; i < CHART_WIDTH; i++) output[pos++] = '-';
    output[pos++] = ']';
    output[pos] = '\0';
}

void get_languages(char *output, size_t max_len) {
    char cmd[MAX_BUFFER];
    snprintf(cmd, sizeof(cmd), "git ls-files | sed 's/.*\\.//' | sort | uniq -c | sort -rn | head -n 4 | awk '{print $2}' | xargs");
    if (run_command(cmd, output, max_len) != 0) {
        strcpy(output, "Unknown");
    }
}

void update_readme() {
    char repo_name[MAX_BUFFER], commit_count[MAX_BUFFER], latest_commit[MAX_BUFFER], langs[MAX_BUFFER], chart[MAX_BUFFER];
    
    get_repo_name(repo_name, MAX_BUFFER);
    get_commit_count(commit_count, MAX_BUFFER);
    get_latest_commit(latest_commit, MAX_BUFFER);
    get_languages(langs, MAX_BUFFER);
    get_activity_chart(chart, MAX_BUFFER);
    
    int files = get_file_count();
    int lines = get_line_count();
    const char *skill = get_skill_level_simple(files, 2);

    char summary[MAX_BUFFER * 2];
    snprintf(summary, sizeof(summary), 
             "\n- **Repository:** `%s`\n- **Primary Languages:** %s\n- **Total Commits:** %s\n- **Latest Activity:** %s\n- **Skill Level:** %s\n- **Files:** %d\n- **Lines of Code:** %d\n- **Activity Chart:** %s\n",
             repo_name, langs, commit_count, latest_commit, skill, files, lines, chart);

    FILE *fp = fopen("README.md", "r");
    if (!fp) {
        perror("Could not open README.md");
        return;
    }

    char content[MAX_BUFFER * 10] = {0};
    char line[MAX_BUFFER];
    int in_summary = 0;
    char final_content[MAX_BUFFER * 20] = {0};

    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, "<!-- REPO_SUMMARY_START -->")) {
            in_summary = 1;
            strcat(final_content, line);
            strcat(final_content, summary);
            continue;
        }
        if (strstr(line, "<!-- REPO_SUMMARY_END -->")) {
            in_summary = 0;
        }
        if (!in_summary) {
            strcat(final_content, line);
        }
    }
    fclose(fp);

    fp = fopen("README.md", "w");
    if (!fp) {
        perror("Could not write README.md");
        return;
    }
    fputs(final_content, fp);
    fclose(fp);
    printf("README.md updated successfully.\n");
}
