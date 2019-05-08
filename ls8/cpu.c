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
  if (fp == NULL) {
    fprintf(stderr, "file not found\n");
    exit(1);
  }
  char line[128];
  int index = 0;
  char c;
  int reading = 1;
  int address = 0;
  while ((c = fgetc(fp)) != EOF) {
    // read one char at a time until we get to EOF
    if (c == '#') {
      // if we've reached a comment, stop reading into line
      reading = 0;
    }
    if (c == '\n') {
      // if we've reached a new line, add line to memory and reset
      cpu_ram_write(cpu, address++, strtoul(line, NULL, 2));
      line[0] = '\0';
      index = 0;
      reading = 1;
    } else if (reading) {
      // otherwise, add the character to line
      line[index++] = c;
    } 
  }
  fclose(fp);
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

int cpu_ram_write(struct cpu *cpu, int index, char *input) {
  // how do we know which block of ram to write to?
  cpu->ram[index] = input;
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
    /* printf("LDI\n"); */
    /* printf("register is %d\n", op_a); */
    /* printf("value is %d\n", op_b); */
    cpu->reg[op_a] = op_b;
    /* printf("confirming: %d\n", cpu->reg[op_a]); */
}

void handle_MUL(struct cpu *cpu, int op_a, int op_b)
{
    /* printf("MUL\n"); */
    cpu->reg[op_a] = cpu->reg[op_a] * cpu->reg[op_b];
}
/**
 * Run the CPU
 */
void cpu_run(struct cpu *cpu)
{
  int running = 1; // True until we get a HLT instruction
  int ir, ops_count, op_a, op_b;

  while (running) {
    // 1. Get the value of the current instruction (in address PC).
    ir = cpu_ram_read(cpu, 0);
    // 2. Figure out how many operands this next instruction requires
    ops_count = (ir >> 6) & 0b11; // check the first two bits for ops count
    /* printf("ic is %d\n", ic); */
    // 3. Get the appropriate value(s) of the operands following this instruction
    if (ops_count == 2) {
      /* instructions = 2; */
      op_a = cpu_ram_read(cpu, 1);
      op_b = cpu_ram_read(cpu, 2);
    } else if (ops_count == 1) {
      /* instructions = 1; */
      op_a = cpu_ram_read(cpu, 1);
    }
    // 4. switch() over it to decide on a course of action.
    // 5. Do whatever the instruction should do according to the spec.
    switch(ir) {
      case HLT:
        /* printf("HLT\n"); */
        running = 0;
        break;
      case LDI:
        handle_LDI(cpu, op_a, op_b);
        break;
      case MUL: // come back and call alu function later
        handle_MUL(cpu, op_a, op_b);
        break;
      case PRN:
        /* printf("PRN\n"); */
        printf("%d\n", cpu->reg[op_a]);
        break;
      case PUSH:
        /* printf("PUSH\n"); */
        cpu->sp--;
        *cpu->sp = cpu->reg[op_a];
        break;
      case POP:
        /* printf("POP\n"); */
        cpu->reg[op_a] = *cpu->sp;
        cpu->sp++;
        break;
      default:
        /* printf("default instance\n"); */
        shutdown(cpu, 1);
    }
    // 6. Move the PC to the next instruction.
    cpu->pc += 1 + ops_count;
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
  cpu->reg[7] = 0xf4;
  cpu->ram = calloc(sizeof(cpu->ram), 256);
  cpu->sp = &cpu->ram[0xf4];
}

void shutdown(struct cpu *cpu, int exit_status) {
  free(cpu->reg);
  free(cpu->ram);
  free(cpu);
  exit(exit_status);
}
