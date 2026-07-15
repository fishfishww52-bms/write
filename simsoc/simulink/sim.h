#ifndef SIM_H
#define SIM_H

#include "soc.h"

#define INPUT_COUNT 6

typedef struct
{
    int voltage;
    int current;
    int remcap;
    int remsoc;
} out_type;

extern int in[INPUT_COUNT];
extern out_type out;

void RunAlgorithm(void);
void maintask(int voltage, int current, int cap, int cellnum, int temp, int init_voltage);

#endif
