//TODO handle some shit that can happen if something

#include <stdio.h>

#include "stack.h"

int main() {
    stack stk = {};
    StackCtor(&stk, 10, DEBUG_INFO(stk)); // воттаквот дальше сам
    return 0;
}
