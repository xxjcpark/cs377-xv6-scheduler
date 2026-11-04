#ifndef PROCSTAT_H
#define PROCSTAT_H


struct procstat {
    int pid;
    uint arrival;
    uint first_run;
    uint completion;
    uint wait_time;
    uint run_time;
};

#endif