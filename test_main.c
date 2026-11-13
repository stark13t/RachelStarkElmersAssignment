/*
 * One test built from the worked example in the brief. Drop this next to main.c and run:
 *
 *     gcc -Wall -Wextra -o test_main test_main.c && ./test_main
 *
 * It includes main.c directly (renaming its main) so it can call your functions without a header.
 * Add your own tests the same way: a function that checks one thing, called from main below.
 */
#include <stdio.h>

#define main main_under_test
#include "main.c"
#undef main

static int failures = 0;

static void expect_equal(const char *what, long actual, long expected) {
    if (actual != expected) {
        printf("FAIL  %-28s got %ld, expected %ld\n", what, actual, expected);
        failures++;
    } else {
        printf("ok    %-28s %ld\n", what, actual);
    }
}

/* First weight frame of the worked example:
 * A5 5A 0C 01 07 10 27 D0 04 C8 03 86 05 3D 04 98 */
static void test_worked_example_frame(void) {
    expect_equal("timestamp (bytes 10 27)", combineBytes(0x10, 0x27), 10000);
    expect_equal("cell 1 raw (bytes D0 04)", combineBytes(0xD0, 0x04), 1232);
    expect_equal("cell 2 raw (bytes C8 03)", combineBytes(0xC8, 0x03), 968);
    expect_equal("cell 3 raw (bytes 86 05)", combineBytes(0x86, 0x05), 1414);
    expect_equal("cell 4 raw (bytes 3D 04)", combineBytes(0x3D, 0x04), 1085);
}

/*Checks to ensure a faulted cell does not update and a non faulted cell updates correctly*/
static void test_faults(void) {
    uint8_t faults = 0b00000001;
    struct weight w1 = {0x01, 100, 0};
    struct weight w2 = {0x02, 200, 1000};
    updateWeight(faults, 1000, &w1);
    updateWeight(faults, 2000, &w2);
    expect_equal("Weight 1", w1.currWeight, 100);
    expect_equal("Weight 2", w2.currWeight, 500);
}

static void test_checksum(void) {
    uint8_t frame[12] = {0x01, 0x07, 0x10, 0x27, 0xD0, 0x04, 0xC8, 0x03, 0x86, 0x05, 0x3D, 0x04, 0x98};
    expect_equal("checksum (92)", checkSum(13, frame), 0x98);
}

int main(void) {
    test_worked_example_frame();
    test_checksum();
    test_faults();
    printf("%s\n", failures == 0 ? "all passed" : "some checks failed");
    return failures == 0 ? 0 : 1;
}
