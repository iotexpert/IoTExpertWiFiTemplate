#pragma once
#include <stdbool.h>

void wifi_task(void *arg);
bool wifi_cmd_enable(char *interface);