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
            if (dot_git) *dot_git = '\0';
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

void get_account_stats(char *output, size_t max_len) {
    char user[MAX_BUFFER];
    char remote_url[MAX_BUFFER];
    
    // Get the remote URL
    if (run_command("git remote get-url origin", remote_url, MAX_BUFFER) != 0) {
        strcpy(output, "Could not determine remote URL");
        return;
    }

    // Robust username extraction
    // Case 1: SSH (git@github.com:username/repo.git)
    char *colon = strchr(remote_url, ':');
    char *slash = strchr(remote_url, '/');
    if (colon && slash && colon < slash) {
        int len = slash - colon - 1;
        strncpy(user, colon + 1, len);
        user[len] = '\0';
    } 
    // Case 2: HTTPS (https://github.com/username/repo.git)
    else if (slash) {
        char *github_com = strstr(remote_url, "github.com/");
        if (github_com) {
            char *start = github_com + 11;
            char *end = strchr(start, '/');
            if (end) {
                int len = end - start;
                strncpy(user, start, len);
                user[len] = '\0';
            } else {
                strcpy(user, "unknown");
            }
        } else {
            strcpy(user, "unknown");
        }
    } else {
        strcpy(user, "unknown");
    }

    if (strcmp(user, "unknown") == 0 || strlen(user) == 0) {
        strcpy(output, "Account stats unavailable: username not found");
        return;
    }

    // Use curl and jq to get the top 5 languages
    char cmd_stats[MAX_BUFFER * 2];
    snprintf(cmd_stats, sizeof(cmd_stats), 
             "curl -s -H \"Authorization: token ${GITHUB_TOKEN}\" \"https://api.github.com/users/%s/repos?per_page=100\" | "
             "jq -r '.[].language' | grep -v 'null' | sort | uniq -c | sort -rn | head -n 5", 
             user);
    
    char final_cmd[MAX_BUFFER * 2 + 20];
    snprintf(final_cmd, sizeof(final_cmd), "sh -c '%s'", cmd_stats);

    char buffer[MAX_BUFFER] = {0};
    FILE *fp = popen(final_cmd, "r");
    if (!fp) {
        strcpy(output, "Error executing stats command");
        return;
    }

    char result[MAX_BUFFER] = {0};
    int max_count = -1;
    
    // First pass to find max_count for normalization
    // Since we can't easily read popen twice, we'll store the lines in a temporary buffer
    // For simplicity in "Beginner C", we'll store lines in a fixed array
    char lines[10][MAX_BUFFER];
    int line_count = 0;
    while (fgets(buffer, sizeof(buffer), fp) && line_count < 10) {
        strcpy(lines[line_count++], buffer);
    }
    pclose(fp);

    for (int i = 0; i < line_count; i++) {
        char lang[MAX_BUFFER];
        int count;
        if (sscanf(lines[i], "%d %s", &count, lang) == 2) {
            if (count > max_count) max_count = count;
        }
    }

    for (int i = 0; i < line_count; i++) {
        char lang[MAX_BUFFER];
        int count;
        if (sscanf(lines[i], "%d %s", &count, lang) == 2) {
            float percent = (max_count > 0) ? ((float)count / max_count * 100.0) : 0;
            int bar_len = (int)(percent / 100.0 * CHART_WIDTH);
            
            char line[MAX_BUFFER];
            int pos = 0;
            pos += sprintf(line, "%-12s [", lang);
            for (int j = 0; j < bar_len; j++) line[pos++] = '#';
            for (int j = bar_len; j < CHART_WIDTH; j++) line[pos++] = '-';
            pos += sprintf(line + pos, "] %.1f%%\n", percent);
            
            strncat(result, line, max_len - strlen(result) - 1);
        }
    }
    strcpy(output, result);
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

    char account_stats[MAX_BUFFER * 2];
    get_account_stats(account_stats, MAX_BUFFER * 2);

    FILE *fp = fopen("README.md", "r");
    if (!fp) {
        perror("Could not open README.md for reading");
        return;
    }

    FILE *tmp_fp = fopen("README.md.tmp", "w");
    if (!tmp_fp) {
        perror("Could not open README.md.tmp for writing");
        fclose(fp);
        return;
    }

    char line[MAX_BUFFER];
    int in_summary = 0;

    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, "<!-- REPO_SUMMARY_START -->")) {
            in_summary = 1;
            fputs(line, tmp_fp);
            fputs(summary, tmp_fp);
            fputs("\n### Account Language Distribution\n```\n", tmp_fp);
            fputs(account_stats, tmp_fp);
            fputs("```\n", tmp_fp);
            continue;
        }
        if (strstr(line, "<!-- REPO_SUMMARY_END -->")) {
            in_summary = 0;
            fputs(line, tmp_fp);
            continue;
        }
        if (!in_summary) {
            fputs(line, tmp_fp);
        }
    }

    fclose(fp);
    fclose(tmp_fp);

    if (rename("README.md.tmp", "README.md") != 0) {
        perror("Could not update README.md");
    } else {
        printf("README.md updated successfully.\n");
    }
}
