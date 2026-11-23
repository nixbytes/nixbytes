#include "../src/summary.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

void test_skill_level() {
    printf("Testing skill levels...\n");
    assert(strcmp(get_skill_level_simple(2, 1), "Beginner") == 0);
    assert(strcmp(get_skill_level_simple(10, 3), "Intermediate") == 0);
    assert(strcmp(get_skill_level_simple(25, 6), "Advanced") == 0);
    printf("Skill level tests passed!\n");
}

void test_activity_chart() {
    printf("Testing activity chart...\n");
    char chart[MAX_BUFFER];
    get_activity_chart(chart, MAX_BUFFER);
    assert(chart[0] == '[');
    assert(chart[CHART_WIDTH + 1] == ']');
    assert(strlen(chart) == CHART_WIDTH + 2);
    printf("Activity chart tests passed!\n");
}

int main() {
    test_skill_level();
    test_activity_chart();
    printf("All tests passed successfully!\n");
    return 0;
}
