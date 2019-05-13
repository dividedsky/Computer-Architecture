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
      if (line[0] != '\0')
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
      cpu->reg[regA] = cpu->reg[regA] * cpu->reg[regB];
      break;
    case ALU_ADD:
      cpu->reg[regA] = cpu->reg[regA] + cpu->reg[regB];
      break;
    case ALU_CMP:
      // and mask to reset flag bits
      *cpu->fl = *cpu->fl & 0b00000000;
      if (cpu->reg[regA] == cpu->reg[regB]) {
        // or maks to set selected bit on
        *cpu->fl = *cpu->fl | 0b00000001;
      } else if (cpu->reg[regA] > cpu->reg[regB]) {
        *cpu->fl = *cpu->fl | 0b00000100;
      } else {
        *cpu->fl = *cpu->fl | 0b00000010;
      }
      break;
    case ALU_AND:
      cpu->reg[regA] = cpu->reg[regA] & cpu->reg[regB];
      break;
    case ALU_OR:
      cpu->reg[regA] = cpu->reg[regA] | cpu->reg[regB];
      break;
    case ALU_XOR:
      cpu->reg[regA] = cpu->reg[regA] ^ cpu->reg[regB];
      break;
    case ALU_NOT:
      cpu->reg[regA] = ~cpu->reg[regA];
      break;
    case ALU_SHL:
      cpu->reg[regA] = cpu->reg[regA] << cpu->reg[regB];
      break;
    case ALU_SHR:
      cpu->reg[regA] = cpu->reg[regA] >> cpu->reg[regB];
      break;
    case ALU_MOD:
      cpu->reg[regA] = cpu->reg[regA] % cpu->reg[regB];
      break;
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
    cpu->reg[op_a] = op_b;
}

void handle_MUL(struct cpu *cpu, int op_a, int op_b)
{
    cpu->reg[op_a] = cpu->reg[op_a] * cpu->reg[op_b];
}

/**
 * Run the CPU
 */
void cpu_run(struct cpu *cpu)
{
  int running = 1; // True until we get a HLT instruction
  int ir, ops_count, op_a, op_b, jmp_flag;

  while (running) {
    // 1. Get the value of the current instruction (in address PC).
    ir = cpu_ram_read(cpu, 0);
    // 2. Figure out how many operands this next instruction requires
    ops_count = (ir >> 6) & 0b11; // check the first two bits for ops count
    // 3. Get the appropriate value(s) of the operands following this instruction
    if (ops_count == 2) {
      op_a = cpu_ram_read(cpu, 1);
      op_b = cpu_ram_read(cpu, 2);
    } else if (ops_count == 1) {
      op_a = cpu_ram_read(cpu, 1);
    }
    // 4. switch() over it to decide on a course of action.
    // 5. Do whatever the instruction should do according to the spec.
    jmp_flag = 0;
    switch(ir) {
      case CALL:
        jmp_flag = 1;
        cpu->sp--;
        *cpu->sp = cpu->pc + 2;
        cpu->pc = cpu->reg[op_a];
        break;
      case RET:
        jmp_flag = 1;
        cpu->pc = *cpu->sp;
        break;
      case HLT:
        running = 0;
        break;
      case LDI:
        handle_LDI(cpu, op_a, op_b);
        break;
      case MUL:
        alu(cpu, ALU_MUL, op_a, op_b);
        break;
      case ADD:
        alu(cpu, ALU_ADD, op_a, op_b);
        break;
      case ADDI:
        cpu->reg[op_a] += op_b;
        break;
      case CMP:
        alu(cpu, ALU_CMP, op_a, op_b);
        break;
      case JMP:
        jmp_flag = 1;
        cpu->pc = cpu->reg[op_a];
        break;
      case JEQ:
        if (*cpu->fl & 0b00000001) {
          jmp_flag = 1;
          cpu->pc = cpu->reg[op_a];
        }
        break;
      case JNE:
        if (!(*cpu->fl & 0b00000001)) {
          jmp_flag = 1;
          cpu->pc = cpu->reg[op_a];
        }
        break;
      case AND:
        alu(cpu, ALU_AND, op_a, op_b);
        break;
      case OR:
        alu(cpu, ALU_OR, op_a, op_b);
        break;
      case XOR:
        alu(cpu, ALU_XOR, op_a, op_b);
        break;
      case NOT:
        alu(cpu, ALU_NOT, op_a, op_b);
        break;
      case SHL:
        alu(cpu, ALU_SHL, op_a, op_b);
        break;
      case SHR:
        alu(cpu, ALU_SHR, op_a, op_b);
        break;
      case PRN:
        printf("%d\n", cpu->reg[op_a]);
        break;
      case PUSH:
        cpu->sp--;
        *cpu->sp = cpu->reg[op_a];
        break;
      case POP:
        cpu->reg[op_a] = *cpu->sp;
        cpu->sp++;
        break;
      default:
        printf("default instance\n");
        shutdown(cpu, 1);
    }
    // 6. Move the PC to the next instruction.
    if (!jmp_flag) {
      cpu->pc += 1 + ops_count;
    }
  }
}

/**
 * Initialize a CPU struct
 */
void cpu_init(struct cpu *cpu)
{
  cpu->pc = 0;
  cpu->reg = calloc(sizeof(cpu->reg), 8);
  cpu->ram = calloc(sizeof(cpu->ram), 256);
  cpu->sp = &cpu->ram[0xf4];
  cpu->fl = calloc(1, sizeof(char));
  /* i know the above is wrong, and the pointer should be in reg[7], but
     is there anyway to get around that? cpu->mem[cpu->reg[SP]] just feels so messy.
     */
}

void shutdown(struct cpu *cpu, int exit_status) {
  free(cpu->reg);
  free(cpu->ram);
  free(cpu);
  exit(exit_status);
}
