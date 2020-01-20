
#include <stdlib.h>
#include <stdio.h>

#include "RTT/SEGGER_RTT.h"

void rtt_assert_0(char* filename, int line, char* condition) {
    SEGGER_RTT_WriteString(0, "ASSERT ");
    SEGGER_RTT_WriteString(0, filename);
    SEGGER_RTT_WriteString(0, ":");

    char num_memory[16];
    snprintf(num_memory, sizeof(num_memory), "%d", line);

    SEGGER_RTT_WriteString(0, num_memory);
    SEGGER_RTT_WriteString(0, " ");
    SEGGER_RTT_WriteString(0, condition);
    SEGGER_RTT_WriteString(0, "\n");

    panic();
}

void rtt_assert_1(char* filename, int line, char* condition, uint32_t arg_1) {
    SEGGER_RTT_WriteString(0, "ASSERT ");
    SEGGER_RTT_WriteString(0, filename);
    SEGGER_RTT_WriteString(0, ":");

    char num_memory[16];
    snprintf(num_memory, sizeof(num_memory), "%d", line);

    SEGGER_RTT_WriteString(0, num_memory);
    SEGGER_RTT_WriteString(0, " ");
    SEGGER_RTT_WriteString(0, condition);

    SEGGER_RTT_WriteString(0, ", ");

    snprintf(num_memory, sizeof(num_memory), "%lu", arg_1);

    SEGGER_RTT_WriteString(0, num_memory);
    SEGGER_RTT_WriteString(0, "\r\n");

    panic();
}
void rtt_assert_2(char* filename, int line, char* condition, uint32_t arg_1, uint32_t arg_2) {
    SEGGER_RTT_WriteString(0, "ASSERT ");
    SEGGER_RTT_WriteString(0, filename);
    SEGGER_RTT_WriteString(0, ":");

    char num_memory[16];
    snprintf(num_memory, sizeof(num_memory), "%d", line);

    SEGGER_RTT_WriteString(0, num_memory);
    SEGGER_RTT_WriteString(0, " ");
    SEGGER_RTT_WriteString(0, condition);

    SEGGER_RTT_WriteString(0, ", ");

    snprintf(num_memory, sizeof(num_memory), "%lu", arg_1);

    SEGGER_RTT_WriteString(0, num_memory);
    SEGGER_RTT_WriteString(0, ", ");

    snprintf(num_memory, sizeof(num_memory), "%lu", arg_2);

    SEGGER_RTT_WriteString(0, num_memory);
    SEGGER_RTT_WriteString(0, "\r\n");
    panic();
}