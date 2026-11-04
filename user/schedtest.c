#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// Small workload to check that the per-process STATS lines
// printed by the kernel in kexit() look reasonable.
// Forks NCHILD CPU-bound children; child i does (i+1) units of work,
// so later children should show larger run_time.

#define NCHILD 3
#define WORK 100000000ULL // loop iterations per unit of work

int
main(void)
{
  for (int i = 0; i < NCHILD; i++) {
    int pid = fork();
    if (pid < 0) {
      printf("schedtest: fork failed\n");
      exit(1);
    }
    if (pid == 0) {
      // volatile so the compiler can't optimize the loop away
      volatile uint64 x = 0;
      for (uint64 j = 0; j < (i + 1) * WORK; j++)
        x++;
      exit(0);
    }
  }

  for (int i = 0; i < NCHILD; i++)
    wait(0);

  printf("schedtest done\n");
  exit(0);
}
