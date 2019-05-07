#include "cpu.h"
#include <stdlib.h>
#include <stdio.h>

#define DATA_LEN 6

/**
 * Load the binary bytes from a .ls8 source file into a RAM array
 */
void cpu_load(struct cpu *cpu, char *file)
{
  FILE *fp = fopen(file, "r");
  char line[128];
  int index = 0;
  char c;
  int reading = 1;
  int address = 0;
  while ((c = fgetc(fp)) != EOF) {
    if (c == '#') {
      reading = 0;
    }
    if (c == '\n') {
      /* printf("adding %s to memory\n", line); */
      cpu->ram[address++] = strtoul(line, NULL, 2);
      line[0] = '\0';
      index = 0;
      reading = 1;
    } else if (reading) {
      line[index++] = c;
    } 
  }
  fclose(fp);

  char data[DATA_LEN] = {
    // From print8.ls8
    0b10000010, // LDI R0,8
    0b00000000,
    0b00001000,
    0b01000111, // PRN R0
    0b00000000,
    0b00000001  // HLT
  };


  /*
  for (int i = 0; i < DATA_LEN; i++) {
    cpu->ram[address++] = data[i];
  }
  */

  // TODO: Replace this with something less hard-coded
}

/**
 * ALU
 */
void alu(struct cpu *cpu, enum alu_op op, unsigned char regA, unsigned char regB)
{
  switch (op) {
    case ALU_MUL:
      // TODO
      break;

    // TODO: implement more ALU ops
  }
}

/** 
  * RAM
  */

int cpu_ram_read(struct cpu *cpu, int index) {
  // using the pc, returns the value of the next byte in ram
  return cpu->ram[cpu->pc + index];
}

int cpu_ram_write(struct cpu *cpu) {
  // how do we know which block of ram to write to?
  return 0;
}

/* branch table with opcode functions */
/*
typedef void (*Handler)(struct cpu *cpu, int op_a, int op_b);
Handler jump_table[255] = {0}; //(struct cpu *cpu, int op_a, int op_b);

branch_tbl[LDI] = handle_LDI;
*/
void handle_LDI(struct cpu *cpu, int op_a, int op_b)
{
    printf("LDI\n");
    printf("register is %d\n", op_a);
    printf("value is %d\n", op_b);
    cpu->reg[op_a] = op_b;
    printf("confirming: %d\n", cpu->reg[op_a]);
}

void handle_MUL(struct cpu *cpu, int op_a, int op_b)
{
    printf("MUL\n");
    cpu->reg[op_a] = cpu->reg[op_a] * cpu->reg[op_b];
}
/**
 * Run the CPU
 */
void cpu_run(struct cpu *cpu)
{
  printf("LDIIIIII: %d\n", LDI);
  int running = 1; // True until we get a HLT instruction
  int ir, instructions, op_a, op_b;

  while (running) {
    // 1. Get the value of the current instruction (in address PC).
    ir = cpu_ram_read(cpu, 0);
    // 2. Figure out how many operands this next instruction requires
    // 3. Get the appropriate value(s) of the operands following this instruction
    if (ir >= 128) {
      instructions = 2;
      op_a = cpu_ram_read(cpu, 1);
      op_b = cpu_ram_read(cpu, 2);
    } else if (ir >= 64) {
      instructions = 1;
      op_a = cpu_ram_read(cpu, 1);
    }
    // 4. switch() over it to decide on a course of action.
    // 5. Do whatever the instruction should do according to the spec.
    switch(ir) {
      case HLT:
        printf("HLT\n");
        running = 0;
        break;
      case LDI:
        handle_LDI(cpu, op_a, op_b);
        break;
      case MUL: // come back and call alu function later
        handle_MUL(cpu, op_a, op_b);
        break;
      case PRN:
        printf("PRN\n");
        printf("%d\n", cpu->reg[op_a]);
        break;
      default:
        printf("default instance\n");
        shutdown(cpu, 1);
    }
    // 6. Move the PC to the next instruction.
    cpu->pc += 1 + instructions;
  }
}

/**
 * Initialize a CPU struct
 */
void cpu_init(struct cpu *cpu)
{
  // TODO: Initialize the PC and other special registers
  cpu->pc = 0;
  cpu->reg = calloc(sizeof(cpu->reg), 8);
  cpu->reg[7] = 0xF4;
  cpu->ram = calloc(sizeof(cpu->ram), 256);
}

void shutdown(struct cpu *cpu, int exit_status) {
  free(cpu->reg);
  free(cpu->ram);
  free(cpu);
  exit(exit_status);
}
