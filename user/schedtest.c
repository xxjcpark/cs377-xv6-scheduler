#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/procstat.h"
#include "user/user.h"

#define MAXCHILDREN 15
#define WORK        100000000ULL

struct job {
  int arrival;
  uint64 workload;
};

int
main(int argc, char *argv[])
{
  if ((argc - 1) < 2 || (argc - 1) % 2 != 0) {
    printf("usage: schedtest arrival work [arrival work ...]\n");
    exit(1);
  }

  int numchildren = (argc - 1) / 2;

  if (numchildren > MAXCHILDREN) {
    printf("Max children is 15\n");
    exit(1);
  }

  struct procstat stats[MAXCHILDREN];
  struct job jobs[MAXCHILDREN];

  for (int i = 0; i < numchildren; i++) {
    struct job j;
    j.arrival = atoi(argv[1 + 2 * i]);
    j.workload = atoi(argv[2 + 2 * i]) * WORK;
    jobs[i] = j;
  }

  for (int i = 1; i < numchildren; i++) {
    struct job key = jobs[i];
    int j = i - 1;
    while (j >= 0 && jobs[j].arrival > key.arrival) {
      jobs[j + 1] = jobs[j];
      j--;
    }
    jobs[j + 1] = key;
  }

  uint start = uptime();

  for (int i = 0; i < numchildren; i++) {
    while (uptime() < start + jobs[i].arrival) {
      pause(1);
    }
    int pid = fork();
    if (pid < 0) {
      printf("schedtest: fork failed\n");
      exit(1);
    }
    if (pid == 0) {
      volatile uint64 x = 0;
      for (uint64 j = 0; j < jobs[i].workload; j++)
        x++;
      (void)x;
      exit(0);
    }
  }

  for (int i = 0; i < numchildren; i++) {
    int status;
    if (waitstat(&status, &stats[i]) < 0) {
      printf("schedtest: waitstat failed\n");
      exit(1);
    }
  }

  uint end = uptime();

  uint total_turnaround = 0, total_response = 0;
  for (int i = 0; i < numchildren; i++) {
    struct procstat *s = &stats[i];
    uint turnaround = s->completion - s->arrival;
    uint response = s->first_run - s->arrival;
    total_turnaround += turnaround;
    total_response += response;
    printf("STATS pid=%d arrival=%d first_run=%d completion=%d "
           "wait_time=%d run_time=%d turnaround=%d response=%d\n",
           s->pid, s->arrival, s->first_run, s->completion, s->wait_time,
           s->run_time, turnaround, response);
  }

  printf("avg turnaround=%d avg response=%d total uptime=%d\n",
         total_turnaround / numchildren, total_response / numchildren,
         end - start);
  printf("schedtest done\n");
  exit(0);
}
