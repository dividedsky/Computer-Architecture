#include <stdio.h>
#include <stdlib.h>
#include "cpu.h"

/**
 * Main
 */
int main(int argc, char *argv[])
{
  struct cpu cpu;

  if (argc != 2) {
    printf("usage: ./ls8 [file]\n");
    return 1;
  }
  cpu_init(&cpu);
  cpu_load(&cpu, argv[1]);
  cpu_run(&cpu);

  free(cpu.reg);
  free(cpu.ram);
  return 0;
}
