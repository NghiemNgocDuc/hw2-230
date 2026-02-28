#include "interpreter.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* reset the system to a defulat status */
void initialize_system(System *sys) {
  sys->registers[EAX] = 0;
  sys->registers[EDX] = 0;
  sys->registers[ECX] = 0;
  sys->registers[ESP] = MEMORY_SIZE - 256;
  sys->registers[EBP] = MEMORY_SIZE - 256;
  sys->registers[EIP] = 0;  // Program counter

  sys->memory.num_instructions = 0;
  for (int i = 0; i < MEMORY_SIZE; i++) {
    sys->memory.instruction[i] = NULL;
    sys->memory.data[i] = 0;
  }
  sys->comparison_flag = 0;
}

/* Remove leading and extra space, and \n from the input string and return the
 * length of updated string */
int reformat(char *line) {
  int idx, size = 0, flag = 0;
  int line_size = strlen(line);
  for (idx = 0; idx < line_size && line[idx] == ' '; idx++)
    ;
  for (; idx < line_size; idx++) {
    if (line[idx] == '\n') break;
    if (line[idx] == ' ') {
      if (!flag) {
        line[size++] = line[idx];
        flag = 1;
      }
    } else {
      line[size++] = line[idx];
      flag = 0;
    }
  }

  line[size] = '\0';
  return size;
}

/* Load all the instruction from the file into the instruction segment in the
 * system */
void load_instructions_from_file(System *sys, const char *filename) {
  FILE *file = fopen(filename, "r");
  if (!file) {
    perror("Error opening file");
    exit(EXIT_FAILURE);
  }

  char line[256];
  int address = 0;

  while (fgets(line, sizeof(line), file) != NULL && address < MEMORY_SIZE) {
    // Remove newline character
    line[strlen(line) - 1] = '\0';
    // Save instruction to the memory
    int size = reformat(line);
    if (size == 0) continue;
    sys->memory.instruction[address] = strdup(line);
    address++;
    // Reach out the end of the instruction
    if (strcmp(line, "END") == 0) break;
  }
  sys->memory.num_instructions = address;

  fclose(file);
}

/* Return value could be the name of one of the valid registers, or NOT_REG for
 * any other input */
RegisterName get_register_by_name(const char *name) {
  if (strcmp(name, "%EAX") == 0) return EAX;
  if (strcmp(name, "%EDX") == 0) return EDX;
  if (strcmp(name, "%ECX") == 0) return ECX;
  if (strcmp(name, "%ESP") == 0) return ESP;
  if (strcmp(name, "%EBP") == 0) return EBP;
  if (strcmp(name, "%EIP") == 0) return EIP;
  return NOT_REG;  // indicate this is not a register
}

/*
This function accepts an operand that can be represented in different formats:
registers, memory, or constant values. For example, a valid operand could be
%EAX, (%EAX), -20(%EAX), or $10.

The function returns one of the following: REG for a register operand, MEM for a
memory operand, or CONST for a constant operand. If the parsing succeeds, the
function will return the appropriate type; otherwise, it will return UNKNOWN in
the event of a parsing error.

For a register operand: reg will be one of the registers, and value will be -1.
For a memory operand: reg will be the register storing the memory address, and
value will be the offset of that memory address. For a constant operand: reg
will be NOT_REG, and value will be the constant value.
*/
MemoryType get_memory_type(const char *operand) {
  MemoryType result = {UNKNOWN, NOT_REG, -1};
  result.reg = get_register_by_name(operand);
  if (result.reg != NOT_REG) {
    result.type = REG;
  } else {
    if (operand[0] == '$') {
      result.type = CONST;
      result.value = atoi(&operand[1]);
    } else if (strstr(operand, "(") && strstr(operand, ")")) {
      char str[5];
      if (operand[0] == '(') {
        sscanf(operand, "(%s)", str);
        result.value = 0;
      } else {
        sscanf(operand, "%d(%s", &result.value, str);
      }
      str[strlen(str) - 1] = '\0';
      result.reg = get_register_by_name(str);
      if (result.reg != NOT_REG) {
        result.type = MEM;
      }
    }
  }
  return result;
}

/*
This function takes a string that represnts a label in the instruction.
It returns the memory address of the next instruction
corresponding to the label in the system. It returns -1 if the label is not
found or the label does not come with . as the first character
*/
int get_addr_from_label(System *sys, const char *label) {
  if (label[0] != '.') {
    return -1;
  }
  for (int i = 0; i < sys->memory.num_instructions; i++) {
    if (strcmp(sys->memory.instruction[i], label) == 0) {
      return (i + 1) * 4;
    }
  }
  return -1;
}


/*
The execute_movl function validates and executes a movl instruction, ensuring
source and destination operands are of known and appropriate types, and then
performs the move operation if valid.

It will return Success if there is no error.
It will return INSTRUCTION_ERROR if src or dst is a undifined memory space. In
this case, the type of a MemoryType data will be UNKNOWN. It will return
INSTRUCTION_ERROR if dst is a constant value instead of a register or memory
address. It will return INSTRUCTION_ERROR if both src and dst are memory
addresses. It will return MEMORY_ERROR if there is a memory address from src or
dst that is an invalid memory address (less than 0, or greater than (MEMORY_SIZE
- 1) * 4).

If there is any error, all the system registers, memory, and
system status should remain unchanged.
Do not change EIP in this function.
HINT: you may use get_memory_type in this function.
*/
ExecResult execute_movl(System *sys, char *src, char *dst) {
  MemoryType src_duc = get_memory_type(src);
  MemoryType dst_duc = get_memory_type(dst);

  if (src_duc.type == UNKNOWN || dst_duc.type == UNKNOWN) {
    return INSTRUCTION_ERROR;
  }

  if (dst_duc.type == CONST) {
    return INSTRUCTION_ERROR;
  }

  if (src_duc.type == MEM && dst_duc.type == MEM) {
    return INSTRUCTION_ERROR;
  }

  int src_value = 0;
  int address = 0;

  

  if (src_duc.type == REG) {
    src_value = sys->registers[src_duc.reg];
  }
  else if (src_duc.type == CONST) {
    src_value = src_duc.value;
  }
  else if (src_duc.type == MEM) {
    address = sys->registers[src_duc.reg] + src_duc.value;

    if (address < 0 || address > (MEMORY_SIZE - 1) * 4 || address % 4 != 0) {
      return MEMORY_ERROR;
    }

    src_value = sys->memory.data[address / 4];
  }

  

  if (dst_duc.type == REG) {
    sys->registers[dst_duc.reg] = src_value;
  }
  else if (dst_duc.type == MEM) {
    address = sys->registers[dst_duc.reg] + dst_duc.value;

    if (address < 0 || address > (MEMORY_SIZE - 1) * 4 || address % 4 != 0) {
      return MEMORY_ERROR;
    }

    sys->memory.data[address / 4] = src_value;
  }

  return SUCCESS;
}

/*
The execute_addl function validates and executes a addl instruction, ensuring
source and destination operands are of known and appropriate types, and then
performs the add operation if valid.

It will return Success if there is no error.
It will return INSTRUCTION_ERROR if src or dst is a undifined memory space. In
this case, the type of a MemoryType data will be UNKNOWN. It will return
INSTRUCTION_ERROR if dst is a constant value instead of a register or memory
address. It will return INSTRUCTION_ERROR if both src and dst are memory
addresses. It will return MEMORY_ERROR if there is a memory address from src or
dst that is an invalid memory address (less than 0, or
greater than (MEMORY_SIZE - 1) * 4).

If there is any error, all the system registers, memory, and
system status should remain unchanged.
Do not change EIP in this function.
HINT: you may use get_memory_type in this function.
*/
ExecResult execute_addl(System *sys, char *src, char *dst) {
  MemoryType src_duc = get_memory_type(src);
  MemoryType dst_duc = get_memory_type(dst);

  if (src_duc.type == UNKNOWN || dst_duc.type == UNKNOWN)
    return INSTRUCTION_ERROR;

  if (dst_duc.type == CONST)
    return INSTRUCTION_ERROR;

  if (src_duc.type == MEM && dst_duc.type == MEM)
    return INSTRUCTION_ERROR;

  int src_value = 0;
  int dst_value = 0;
  int src_address = 0;
  int dst_address = 0;

  if (src_duc.type == REG)
    src_value = sys->registers[src_duc.reg];
  else if (src_duc.type == CONST)
    src_value = src_duc.value;
  else {
    src_address = sys->registers[src_duc.reg] + src_duc.value;
    if (src_address < 0 || src_address > (MEMORY_SIZE - 1) * 4 || src_address % 4 != 0)
      return MEMORY_ERROR;
    src_value = sys->memory.data[src_address / 4];
  }

  if (dst_duc.type == REG)
    dst_value = sys->registers[dst_duc.reg];
  else {
    dst_address = sys->registers[dst_duc.reg] + dst_duc.value;
    if (dst_address < 0 || dst_address > (MEMORY_SIZE - 1) * 4 || dst_address % 4 != 0)
      return MEMORY_ERROR;
    dst_value = sys->memory.data[dst_address / 4];
  }

  int result = dst_value + src_value;

  if (dst_duc.type == REG)
    sys->registers[dst_duc.reg] = result;
  else
    sys->memory.data[dst_address / 4] = result;

  return SUCCESS;
}

/*
The execute_push function validates and executes a pushl instruction, ensuring
source operands is of known and appropriate type, and then performs the push
operation if valid.

It will return Success if there is no error.
It will return INSTRUCTION_ERROR if src is a undifined memory space. In this
case, the type of a MemoryType data will be UNKNOWN. It will return MEMORY_ERROR
if the address stored in src is an invalid memory address (less than 0, or
greater than (MEMORY_SIZE - 1) * 4).
It will return MEMORY_ERROR if esp is
an invalid memory address: less than 4, greater than or equal to MEMORY_SIZE *
4).

If there is any error, all the system registers, memory, and
system status should remain unchanged.
Do not change EIP in this function.
HINT: you may use get_memory_type in this function.
*/
ExecResult execute_push(System *sys, char *src) {
  MemoryType src_duc = get_memory_type(src);

  
  if (src_duc.type == UNKNOWN) {
    return INSTRUCTION_ERROR;
  }

  int src_value = 0;
  int src_address = 0;

  

  if (src_duc.type == REG) {
    src_value = sys->registers[src_duc.reg];
  }
  else if (src_duc.type == CONST) {
    src_value = src_duc.value;
  }
  else {  // MEM
    src_address = sys->registers[src_duc.reg] + src_duc.value;

    if (src_address < 0 || 
        src_address > (MEMORY_SIZE - 1) * 4 || 
        src_address % 4 != 0) {
      return MEMORY_ERROR;
    }

    src_value = sys->memory.data[src_address / 4];
  }

  

  int old_esp = sys->registers[ESP];
  int new_esp = old_esp - 4;

 
  if (new_esp < 4 || new_esp >= MEMORY_SIZE * 4 || new_esp % 4 != 0) {
    return MEMORY_ERROR;
  }

  
  sys->memory.data[new_esp / 4] = src_value;

 

  sys->registers[ESP] = new_esp;

  return SUCCESS;
}

/*
The execute_pop function validates and executes a popl instruction, ensuring the
destination operand is of known and appropriate type, and then performs the pop
operation if valid.

It will return SUCCESS if there is no error.
It will return INSTRUCTION_ERROR if dst is not a register or memory address.
It will return MEMORY_ERROR if dst is an invalid memory address: less than 0, or
greater than (MEMORY_SIZE - 1) * 4).
It will return MEMORY_ERROR if the
address stored in esp is an invalid memory address: less than 0, or
greater than (MEMORY_SIZE - 1) * 4).

If there is any error, all the system registers, memory, and
system status should remain unchanged.
Do not change EIP in this function.
HINT: you may use get_memory_type in this function.
*/
ExecResult execute_pop(System *sys, char *dst) {
    MemoryType dst_duc = get_memory_type(dst);

    if (dst_duc.type == UNKNOWN || dst_duc.type == CONST) {
        return INSTRUCTION_ERROR;
    }

    int old_esp = sys->registers[ESP];

    if (old_esp < 0 || old_esp > (MEMORY_SIZE - 1) * 4 || old_esp % 4 != 0) {
        return MEMORY_ERROR;
    }

    int value = sys->memory.data[old_esp / 4];
    int incremented_esp = old_esp + 4;

    
    if (incremented_esp < 0 || incremented_esp > MEMORY_SIZE * 4) {
        return MEMORY_ERROR;
    }

    if (dst_duc.type == REG) {
        
        sys->registers[dst_duc.reg] = value;
        
       
        sys->registers[ESP] = sys->registers[ESP] + 4;
        
    } else { // MEM Case
        int dst_address = sys->registers[dst_duc.reg] + dst_duc.value;

        if (dst_address < 0 || dst_address > (MEMORY_SIZE - 1) * 4 || dst_address % 4 != 0) {
            return MEMORY_ERROR;
        }

        sys->memory.data[dst_address / 4] = value;
        sys->registers[ESP] = incremented_esp;
    }

    return SUCCESS;
}
/*
The execute_cmpl function validates and executes a cmpl instruction, ensuring
the source and destination operands are of known and appropriate types, and then
performs the compare operation and update comparison_flag in the system if
valid.

It will return SUCCESS if there is no error.
It will return INSTRUCTION_ERROR if src or dst is an undefined memory space.
It will return INSTRUCTION_ERROR if both src and dst are memory addresses.
It will return MEMORY_ERROR if there is a memory address from src or dst that is
an invalid memory address (less than 0, or
greater than (MEMORY_SIZE - 1) * 4).

If there is any error, all the system registers, memory, and
system status should remain unchanged. You can decide the value of
comparison_flag for each comparison result.
Do not change EIP in this function.
HINT: you may use get_memory_type in this function.
*/
ExecResult execute_cmpl(System *sys, char *src, char *dst) {
  MemoryType src_duc = get_memory_type(src);
  MemoryType dst_duc = get_memory_type(dst);

  
  if (src_duc.type == UNKNOWN || dst_duc.type == UNKNOWN) {
    return INSTRUCTION_ERROR;
  }

  if (src_duc.type == MEM && dst_duc.type == MEM) {
    return INSTRUCTION_ERROR;
  }
    
  
  if (dst_duc.type == CONST) {
    return INSTRUCTION_ERROR;
  }

  int src_value = 0;
  int dst_value = 0;
  int address = 0;

  if (src_duc.type == REG) {
    src_value = sys->registers[src_duc.reg];
  } else if (src_duc.type == CONST) {
    src_value = src_duc.value;
  } else if (src_duc.type == MEM) {
    address = sys->registers[src_duc.reg] + src_duc.value;
    if (address < 0 || address > (MEMORY_SIZE - 1) * 4 || address % 4 != 0) {
      return MEMORY_ERROR;
    }
    src_value = sys->memory.data[address / 4];
  }

  if (dst_duc.type == REG) {
    dst_value = sys->registers[dst_duc.reg];
  } else if (dst_duc.type == MEM) {
    address = sys->registers[dst_duc.reg] + dst_duc.value;
    if (address < 0 || address > (MEMORY_SIZE - 1) * 4 || address % 4 != 0) {
      return MEMORY_ERROR;
    }
    dst_value = sys->memory.data[address / 4];
  }

  if (dst_value == src_value) {
    sys->comparison_flag = 0;
  } else if (dst_value > src_value) {
    sys->comparison_flag = 1;
  } else {
    sys->comparison_flag = -1;
  }
  return SUCCESS;
}

/*
The execute_jmp function validates and executes a condition or direct jump
instruction, ensuring the destination operands is of known label, and then
performs the direct jump operation, or condition jump if condition is met.
A valid condition argument should be one of the following strings: "JE", "JNE",
"JL", "JG", or "JMP"

It will return SUCCESS if the jump is executed successfully no matter whether
condition is met. It will return PC_ERROR if the destination label cannot be
found in the instruction segment in the system.

If there is any error, all the system registers (except for EIP), memory, and
system status should remain unchanged.
Please update program counter (EIP) in this function.
HINT: you may use get_addr_from_label in this function.
*/
ExecResult execute_jmp(System *sys, char *condition, char *dst) {
  int target_address = get_addr_from_label(sys, dst);

 
  if (target_address == -1) {
    return PC_ERROR;
  }

  int should_jump = 0;

  if (strcmp(condition, "JMP") == 0) {
    should_jump = 1;
  } else if (strcmp(condition, "JE") == 0) {
    if (sys->comparison_flag == 0) should_jump = 1;
  } else if (strcmp(condition, "JNE") == 0) {
    if (sys->comparison_flag != 0) should_jump = 1;
  } else if (strcmp(condition, "JL") == 0) {
    if (sys->comparison_flag == -1) should_jump = 1;
  } else if (strcmp(condition, "JG") == 0) {
    if (sys->comparison_flag == 1) should_jump = 1;
  }
  if (should_jump) {
    sys->registers[EIP] = target_address;
  } else {
    
  }
  
  return SUCCESS;
}

/*
The execute_call function validates and executes a call instruction, ensuring
the destination operand is a known label, and then performs the call operation.

It will return SUCCESS if the call is executed successfully.
It will return PC_ERROR if the destination label cannot be found in the
instruction segment in the system.

If there is any error, all the system registers (except for EIP), memory, and
system status should remain unchanged.
Please update program counter (EIP) in this function.
HINT: you may use get_addr_from_label in this function.
*/
ExecResult execute_call(System *sys, char *dst) {
  int target_address = get_addr_from_label(sys, dst);
  if (target_address == -1) {
    return PC_ERROR;
  }

 
  int return_address = sys->registers[EIP] + 4;
  int old_esp = sys->registers[ESP];
  int new_esp = old_esp - 4;

  
  if (new_esp < 4 || new_esp >= MEMORY_SIZE * 4 || new_esp % 4 != 0) {
    return MEMORY_ERROR;
  }

 
  sys->memory.data[new_esp / 4] = return_address;

  
  sys->registers[ESP] = new_esp;
  sys->registers[EIP] = target_address;
  return SUCCESS;
}

/*
The execute_ret function validates and executes a return instruction, which pops
the return address from the stack and update EIP (program counter).

It will return SUCCESS if the return is executed successfully.
It will return PC_ERROR if the return address is invalid (less than 0 or greater
than or equal to the number of instructions).

If there is any error, all the system registers (except for EIP and ESP),
memory, and system status should remain unchanged.
Please update program counter (EIP) in this function.
*/
ExecResult execute_ret(System *sys) {
  int current_esp = sys->registers[ESP];

  
  if (current_esp < 0 || current_esp > (MEMORY_SIZE - 1) * 4 || current_esp % 4 != 0) {
    return MEMORY_ERROR;
  }
  int ret_addr = sys->memory.data[current_esp / 4];

 
  if (ret_addr < 0 || ret_addr >= sys->memory.num_instructions * 4 || ret_addr % 4 != 0) {
    return PC_ERROR;
  }
  sys->registers[EIP] = ret_addr;
  sys->registers[ESP] = current_esp + 4;
  return SUCCESS;
}

/*
Utilizing the EIP register's value (also known as the program counter), the
function fetches instructions from the instruction segment in system memory. It
then executes each instruction, which can be one of MOVL, ADDL PUSHL, POPL,
CMPL, CALL, RET, JMP, JNE, JE, JL, or JG, by employing the corresponding execute
functions. This process continues until the program encounters any Error status
or the END instruction. During the execution, it will ignore all the
instructions that are not listed above and continue to the next one.
Please update program counter (EIP) for MOVL, ADDL, PUSHL, POPL, and CMPL in
this function.
*/
void execute_instructions(System *sys) {
  char inst[256];
  ExecResult result = SUCCESS;

  while (result == SUCCESS) {
    
    int current_pc = sys->registers[EIP];
    int instruction_idx = current_pc / 4;

   
    if (instruction_idx < 0 || instruction_idx >= sys->memory.num_instructions) {
      break; 
    }

    char *raw_line = sys->memory.instruction[instruction_idx];
    if (raw_line == NULL) break;

    
    strcpy(inst, raw_line);

   
    char *opcode = strtok(inst, " ");
    if (opcode == NULL) {
      sys->registers[EIP] += 4;
      continue;
    }

    
    if (strcmp(opcode, "END") == 0) {
      break;
    }

    
    if (strcmp(opcode, "MOVL") == 0) {
      char *src = strtok(NULL, ", ");
      char *dst = strtok(NULL, ", ");
      result = execute_movl(sys, src, dst);
      if (result == SUCCESS) sys->registers[EIP] += 4;

    } else if (strcmp(opcode, "ADDL") == 0) {
      char *src = strtok(NULL, ", ");
      char *dst = strtok(NULL, ", ");
      result = execute_addl(sys, src, dst);
      if (result == SUCCESS) sys->registers[EIP] += 4;

    } else if (strcmp(opcode, "PUSHL") == 0) {
      char *src = strtok(NULL, " ");
      result = execute_push(sys, src);
      if (result == SUCCESS) sys->registers[EIP] += 4;

    } else if (strcmp(opcode, "POPL") == 0) {
      char *dst = strtok(NULL, " ");
      result = execute_pop(sys, dst);
      if (result == SUCCESS) sys->registers[EIP] += 4;

    } else if (strcmp(opcode, "CMPL") == 0) {
      char *src = strtok(NULL, ", ");
      char *dst = strtok(NULL, ", ");
      result = execute_cmpl(sys, src, dst);
      if (result == SUCCESS) sys->registers[EIP] += 4;

    } else if (strcmp(opcode, "CALL") == 0) {
      char *label = strtok(NULL, " ");
      result = execute_call(sys, label);
      

    } else if (strcmp(opcode, "RET") == 0) {
      result = execute_ret(sys);
      

    } else if (opcode[0] == 'J') { 
      char *label = strtok(NULL, " ");
      
      int old_eip = sys->registers[EIP];
      result = execute_jmp(sys, opcode, label);
      
      
      if (result == SUCCESS && sys->registers[EIP] == old_eip) {
        sys->registers[EIP] += 4;
      }

    } else {
      
      sys->registers[EIP] += 4;
    }
  }
}
