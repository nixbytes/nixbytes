#ifndef SUMMARY_H
#define SUMMARY_H

#include <stddef.h>

#define MAX_BUFFER 2048
#define CHART_WIDTH 20

int run_command(const char *cmd, char *output, size_t max_len);
void get_repo_name(char *output, size_t max_len);
void get_commit_count(char *output, size_t max_len);
void get_latest_commit(char *output, size_t max_len);
int get_file_count();
int get_line_count();
const char* get_skill_level_simple(int files, int langs);
void get_activity_chart(char *output, size_t max_len);
void get_languages(char *output, size_t max_len);
void get_account_stats(char *output, size_t max_len);
void update_readme();

#endif
