#include <gtest/gtest.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "interpreter.h"

// Include these definitions to test against solution:
// int soln_get_set(Cache *cache, address_type address);
// int soln_get_line(Cache *cache, address_type address);
// int soln_get_byte(Cache *cache, address_type address);

class ProjectTests : public ::testing::Test {
 protected:
  ProjectTests() {}           // constructor runs before each test
  virtual ~ProjectTests() {}  // destructor cleans up after tests
  virtual void SetUp() {}     // sets up before each test (after constructor)
  virtual void TearDown() {}  // clean up after each test, (before destructor)
};

TEST(ProjectTests, test_movl_register) {
  System sys;
  initialize_system(&sys);

  // Initialize some registers for testing
  sys.registers[EAX] = 5;
  sys.registers[EDX] = 3;
  sys.registers[ECX] = 2;

  // Execute instructions
  ExecResult result;
  result = execute_movl(&sys, "%EDX", "%EAX");
  ASSERT_EQ(result, SUCCESS) << "return value should be SUCCESS";
  ASSERT_EQ(sys.registers[EAX], 3)
      << "EAX should be 3 and yours is " << sys.registers[EAX] << ".";
  ASSERT_EQ(sys.registers[EDX], 3)
      << "EDX should be 3 and yours is " << sys.registers[EDX] << ".";
  ASSERT_EQ(sys.registers[ECX], 2)
      << "ECX should be 2 and yours is " << sys.registers[ECX] << ".";

  result = execute_movl(&sys, "%ECX", "%EDX");
  ASSERT_EQ(result, SUCCESS) << "return value should be SUCCESS";
  ASSERT_EQ(sys.registers[EAX], 3)
      << "EAX should be 3 and yours is " << sys.registers[EAX] << ".";
  ASSERT_EQ(sys.registers[EDX], 2)
      << "EDX should be 2 and yours is " << sys.registers[EDX] << ".";
  ASSERT_EQ(sys.registers[ECX], 2)
      << "ECX should be 2 and yours is " << sys.registers[ECX] << ".";

  result = execute_movl(&sys, "%EAX", "%ECX");
  ASSERT_EQ(result, SUCCESS) << "return value should be SUCCESS";
  ASSERT_EQ(sys.registers[EAX], 3)
      << "EAX should be 3 and yours is " << sys.registers[EAX] << ".";
  ASSERT_EQ(sys.registers[EDX], 2)
      << "EDX should be 2 and yours is " << sys.registers[EDX] << ".";
  ASSERT_EQ(sys.registers[ECX], 3)
      << "ECX should be 3 and yours is " << sys.registers[ECX] << ".";
}

TEST(ProjectTests, test_addl_register) {
  System sys;
  initialize_system(&sys);

  // Initialize some registers for testing
  sys.registers[EAX] = 5;
  sys.registers[EDX] = 3;
  sys.registers[ECX] = 2;

  // Execute instructions
  ExecResult result;
  result = execute_addl(&sys, "%EDX", "%EAX");
  ASSERT_EQ(result, SUCCESS) << "return value should be SUCCESS";
  ASSERT_EQ(sys.registers[EAX], 8)
      << "EAX should be 8 and yours is " << sys.registers[EAX] << ".";
  ASSERT_EQ(sys.registers[EDX], 3)
      << "EDX should be 3 and yours is " << sys.registers[EDX] << ".";
  ASSERT_EQ(sys.registers[ECX], 2)
      << "ECX should be 2 and yours is " << sys.registers[ECX] << ".";

  result = execute_addl(&sys, "%ECX", "%EDX");
  ASSERT_EQ(result, SUCCESS) << "return value should be SUCCESS";
  ASSERT_EQ(sys.registers[EAX], 8)
      << "EAX should be 8 and yours is " << sys.registers[EAX] << ".";
  ASSERT_EQ(sys.registers[EDX], 5)
      << "EDX should be 5 and yours is " << sys.registers[EDX] << ".";
  ASSERT_EQ(sys.registers[ECX], 2)
      << "ECX should be 2 and yours is " << sys.registers[ECX] << ".";

  result = execute_addl(&sys, "%EAX", "%ECX");
  ASSERT_EQ(result, SUCCESS) << "return value should be SUCCESS";
  ASSERT_EQ(sys.registers[EAX], 8)
      << "EAX should be 8 and yours is " << sys.registers[EAX] << ".";
  ASSERT_EQ(sys.registers[EDX], 5)
      << "EDX should be 5 and yours is " << sys.registers[EDX] << ".";
  ASSERT_EQ(sys.registers[ECX], 10)
      << "ECX should be 10 and yours is " << sys.registers[ECX] << ".";
}

TEST(ProjectTests, test_movl_errors) {
  System sys;
  initialize_system(&sys);

  // Initialize some registers for testing
  sys.registers[EAX] = 8;
  sys.registers[EDX] = 4;
  sys.registers[ECX] = 12;

  // Execute instructions
  ExecResult result;
  result = execute_movl(&sys, "RANDOM", "%EAX");
  ASSERT_EQ(result, INSTRUCTION_ERROR)
      << "return value should be INSTRUCTION_ERROR";

  result = execute_movl(&sys, "%EDX", "RANDOM");
  ASSERT_EQ(result, INSTRUCTION_ERROR)
      << "return value should be INSTRUCTION_ERROR";

  result = execute_movl(&sys, "%ECX", "$10");
  ASSERT_EQ(result, INSTRUCTION_ERROR)
      << "return value should be INSTRUCTION_ERROR";

  result = execute_movl(&sys, "(%EAX)", "(%EDX)");
  ASSERT_EQ(result, INSTRUCTION_ERROR)
      << "return value should be INSTRUCTION_ERROR";

  result = execute_movl(&sys, "-16(%EAX)", "%ECX");
  ASSERT_EQ(result, MEMORY_ERROR)
      << "return value of movl from an invalid memory should be MEMORY_ERROR";

  ASSERT_EQ(sys.registers[EAX], 8)
      << "Registers should not be changed after incorrect movl execution";
  ASSERT_EQ(sys.registers[EDX], 4)
      << "Registers should not be changed after incorrect movl execution";
  ASSERT_EQ(sys.registers[ECX], 12)
      << "Registers should not be changed after incorrect movl execution";
}

TEST(ProjectTests, test_push_register) {
  System sys;
  initialize_system(&sys);

  // Initialize some registers and stack pointer for testing
  sys.registers[EAX] = 5;
  sys.registers[EDX] = 3;
  sys.registers[ECX] = 2;
  int STACK_IDX = 500;
  sys.registers[ESP] =
      STACK_IDX * 4;  // Assume ESP starts at the top of the memory stack

  // Execute instructions
  ExecResult result;

  result = execute_push(&sys, "%EAX");
  ASSERT_EQ(result, SUCCESS) << "return value should be SUCCESS";
  ASSERT_EQ(sys.registers[ESP], STACK_IDX * 4 - 4)
      << "ESP should be " << STACK_IDX * 4 - 4 << " and yours is "
      << sys.registers[ESP] << ".";
  ASSERT_EQ(sys.memory.data[(STACK_IDX * 4 - 4) / 4], 5)
      << "Top of the stack should be 5 and yours is "
      << sys.memory.data[(STACK_IDX * 4 - 4) / 4] << ".";

  result = execute_push(&sys, "%EDX");
  ASSERT_EQ(result, SUCCESS) << "return value should be SUCCESS";
  ASSERT_EQ(sys.registers[ESP], STACK_IDX * 4 - 8)
      << "ESP should be " << STACK_IDX * 4 - 8 << " and yours is "
      << sys.registers[ESP] << ".";
  ASSERT_EQ(sys.memory.data[(STACK_IDX * 4 - 8) / 4], 3)
      << "Top of the stack should be 3 and yours is "
      << sys.memory.data[(STACK_IDX * 4 - 8) / 4] << ".";

  result = execute_push(&sys, "%ECX");
  ASSERT_EQ(result, SUCCESS) << "return value should be SUCCESS";
  ASSERT_EQ(sys.registers[ESP], STACK_IDX * 4 - 12)
      << "ESP should be " << STACK_IDX * 4 - 12 << " and yours is "
      << sys.registers[ESP] << ".";
  ASSERT_EQ(sys.memory.data[(STACK_IDX * 4 - 12) / 4], 2)
      << "Top of the stack should be 2 and yours is "
      << sys.memory.data[(STACK_IDX * 4 - 12) / 4] << ".";
}

TEST(ProjectTests, test_pop_register) {
  System sys;
  initialize_system(&sys);

  // Initialize some registers and stack for testing
  int STACK_IDX = 500;
  sys.registers[ESP] =
      STACK_IDX * 4 - 12;  // Assume ESP starts with three elements on the stack

  // Set up the stack
  sys.memory.data[(STACK_IDX * 4 - 12) / 4] = 2;
  sys.memory.data[(STACK_IDX * 4 - 8) / 4] = 3;
  sys.memory.data[(STACK_IDX * 4 - 4) / 4] = 5;

  // Execute instructions
  ExecResult result;

  result = execute_pop(&sys, "%ECX");
  ASSERT_EQ(result, SUCCESS) << "return value should be SUCCESS";
  ASSERT_EQ(sys.registers[ESP], STACK_IDX * 4 - 8)
      << "ESP should be " << STACK_IDX * 4 - 8 << " and yours is "
      << sys.registers[ESP] << ".";
  ASSERT_EQ(sys.registers[ECX], 2)
      << "ECX should be 2 and yours is " << sys.registers[ECX] << ".";

  result = execute_pop(&sys, "%EDX");
  ASSERT_EQ(result, SUCCESS) << "return value should be SUCCESS";
  ASSERT_EQ(sys.registers[ESP], STACK_IDX * 4 - 4)
      << "ESP should be " << STACK_IDX * 4 - 4 << " and yours is "
      << sys.registers[ESP] << ".";
  ASSERT_EQ(sys.registers[EDX], 3)
      << "EDX should be 3 and yours is " << sys.registers[EDX] << ".";

  result = execute_pop(&sys, "%EAX");
  ASSERT_EQ(result, SUCCESS) << "return value should be SUCCESS";
  ASSERT_EQ(sys.registers[ESP], STACK_IDX * 4)
      << "ESP should be " << STACK_IDX * 4 << " and yours is "
      << sys.registers[ESP] << ".";
  ASSERT_EQ(sys.registers[EAX], 5)
      << "EAX should be 5 and yours is " << sys.registers[EAX] << ".";
}

TEST(ProjectTests, test_cmpl_register) {
  System sys;
  initialize_system(&sys);

  // Initialize some registers for testing
  sys.registers[EAX] = 5;
  sys.registers[EDX] = 7;

  // Execute instructions and verify comparison flag
  ExecResult result;

  // Case: EDX compared to EAX
  result = execute_cmpl(&sys, "%EDX", "%EAX");
  ASSERT_EQ(result, SUCCESS) << "return value should be SUCCESS";
  int comparison1 = sys.comparison_flag;

  // Case: EAX compared to EDX
  result = execute_cmpl(&sys, "%EAX", "%EDX");
  ASSERT_EQ(result, SUCCESS) << "return value should be SUCCESS";
  int comparison2 = sys.comparison_flag;

  // Case: EAX compared to 5
  result = execute_cmpl(&sys, "%EAX", "$5");
  ASSERT_EQ(result, SUCCESS) << "return value should be SUCCESS";
  int comparison3 = sys.comparison_flag;

  // Case: EAX compared to 3
  result = execute_cmpl(&sys, "%EAX", "$3");
  ASSERT_EQ(result, SUCCESS) << "return value should be SUCCESS";
  int comparison4 = sys.comparison_flag;

  ASSERT_NE(comparison3, comparison4)
      << "Comparison betweeen case of EQUAL and case of NOT EQUAL should be "
         "different.";

  ASSERT_NE(comparison1, comparison2)
      << "Comparison betweeen case of LARGER THAN and case of SMALLER THAN "
         "should be different.";

  ASSERT_NE(comparison2, comparison3)
      << "Comparison betweeen case of LARGER THAN and case of EQUAL should be "
         "different.";

  ASSERT_NE(comparison1, comparison3)
      << "Comparison betweeen case of SMALLER THAN  and case of EQUAL should "
         "be different.";
}

TEST(ProjectTests, test_cond_jmp_true) {
  System sys;
  initialize_system(&sys);

  // Initialize system for testing
  sys.memory.num_instructions = 4;
  sys.memory.instruction[0] = strdup(".L1");           // address 0
  sys.memory.instruction[1] = strdup("CMPL %EAX $7");  // address 4
  sys.memory.instruction[2] = strdup("JG .L1");        // address 8
  sys.memory.instruction[3] = strdup("JMP .L1");       // address 12

  sys.registers[EAX] = 5;
  sys.registers[EIP] = 8;

  ExecResult result;
  result = execute_cmpl(&sys, "%EAX", "$7");
  ASSERT_EQ(result, SUCCESS) << "return value should be SUCCESS";

  result = execute_jmp(&sys, "JG", ".L1");
  ASSERT_EQ(result, SUCCESS) << "return value should be SUCCESS";

  ASSERT_EQ(sys.registers[EIP], 4)
      << "Since 7 is larger than 5 so the condition is met and EIP should be "
         "updated to the next line of lable .L1 with address 4 and yours is "
      << sys.registers[EIP] << ".";
}

TEST(ProjectTests, test_cond_jmp_false) {
  System sys;
  initialize_system(&sys);

  // Initialize system for testing
  sys.memory.num_instructions = 4;
  sys.memory.instruction[0] = strdup(".L1");           // address 0
  sys.memory.instruction[1] = strdup("CMPL %EAX $7");  // address 4
  sys.memory.instruction[2] = strdup("JE .L1");        // address 8
  sys.memory.instruction[3] = strdup("JMP .L1");       // address 12

  sys.registers[EAX] = 5;
  sys.registers[EIP] = 8;

  ExecResult result;
  result = execute_cmpl(&sys, "%EAX", "$7");
  ASSERT_EQ(result, SUCCESS) << "return value should be SUCCESS";

  result = execute_jmp(&sys, "JE", ".L1");
  ASSERT_EQ(result, SUCCESS) << "return value should be SUCCESS";

  ASSERT_EQ(sys.registers[EIP], 12)
      << "Since 7 is not equal to 5 so the condition is not met and EIP should "
         "be updated to the next line with address 12 and yours is "
      << sys.registers[EIP] << ".";
}

TEST(ProjectTests, test_direct_jmp) {
  System sys;
  initialize_system(&sys);

  // Initialize system for testing
  sys.memory.num_instructions = 5;
  sys.memory.instruction[0] = strdup(".L1");           // address 0
  sys.memory.instruction[1] = strdup("CMPL %EAX $7");  // address 4
  sys.memory.instruction[2] = strdup(".L2");           // address 8
  sys.memory.instruction[3] = strdup("POPL %EAX");     // address 12
  sys.memory.instruction[4] = strdup("JMP .L2");       // address 16

  sys.registers[EIP] = 16;

  ExecResult result;
  result = execute_jmp(&sys, "JMP", ".L2");
  ASSERT_EQ(result, SUCCESS) << "return value should be SUCCESS";
  ASSERT_EQ(sys.registers[EIP], 12)
      << "After jump EIP should be updated to the next line of lable .L2 with "
         "address 12 and yours is "
      << sys.registers[EIP] << ".";
}

TEST(ProjectTests, test_execute_call) {
  System sys;
  initialize_system(&sys);

  // Initialize system for testing
  sys.memory.num_instructions = 6;
  sys.memory.instruction[0] = strdup(".L1");           // address 0
  sys.memory.instruction[1] = strdup("CALL .L3");      // address 4
  sys.memory.instruction[2] = strdup("CMPL %EAX $7");  // address 8
  sys.memory.instruction[3] = strdup(".L3");           // address 12
  sys.memory.instruction[4] = strdup("POPL %EAX");     // address 16
  sys.memory.instruction[5] = strdup("RET");           // address 20

  sys.registers[EIP] = 4;  // Pointing to CALL .L3
  int STACK_IDX = 500;
  sys.registers[ESP] =
      STACK_IDX * 4;  // Assume ESP starts at the top of the memory stack

  ExecResult result;
  result = execute_call(&sys, ".L3");

  ASSERT_EQ(result, SUCCESS) << "return value should be SUCCESS";
  ASSERT_EQ(sys.registers[EIP], 16)
      << "After CALL, EIP should be updated to the next line of label .L3 with "
         "address 16 and yours is "
      << sys.registers[EIP] << ".";
}

TEST(ProjectTests, test_execute_ret) {
  System sys;
  initialize_system(&sys);

  // Initialize system for testing
  sys.memory.num_instructions = 5;
  sys.memory.instruction[0] = strdup("CALL .L1");      // address 0
  sys.memory.instruction[1] = strdup("MOVL $1 %EAX");  // address 4
  sys.memory.instruction[2] = strdup(".L1");           // address 8
  sys.memory.instruction[3] = strdup("RET");           // address 12
  sys.memory.instruction[4] = strdup("MOVL $2 %EAX");  // address 16

  sys.registers[EIP] = 12;  // Pointing to RET
  int STACK_IDX = 500;
  sys.registers[ESP] = STACK_IDX * 4;
  sys.memory.data[STACK_IDX] = 4;

  ExecResult result;
  result = execute_ret(&sys);

  ASSERT_EQ(result, SUCCESS) << "return value should be SUCCESS";
  ASSERT_EQ(sys.registers[EIP], 4)
      << "After RET, EIP should be updated to the address 4 and yours is "
      << sys.registers[EIP] << ".";
}

TEST(ProjectTests, test_execute_instructions) {
  System sys;
  initialize_system(&sys);

  // Load instructions from the file specified in the program argument
  sys.memory.num_instructions = 5;
  sys.memory.instruction[0] = strdup("MOVL $1 %ECX");    // address 0
  sys.memory.instruction[1] = strdup("PUSHL %EDX");      // address 4
  sys.memory.instruction[2] = strdup("POPL %EAX");       // address 8
  sys.memory.instruction[3] = strdup("CMPL %EDX %EAX");  // address 12
  sys.memory.instruction[4] = strdup("END");             // address 16

  // Initialize some registers for testing
  sys.registers[EAX] = 5;
  sys.registers[EDX] = 3;
  sys.registers[ECX] = 2;

  // Execute instructions
  execute_instructions(&sys);

  ASSERT_EQ(sys.registers[EIP], 16)
      << "EIP should be 16 when reaching END instruction and yours is "
      << sys.registers[EIP] << ".";
}

TEST(ProjectTests, test_movl_register_from_file) {
  System sys;
  initialize_system(&sys);

  // Load instructions from the file specified in the program argument
  load_instructions_from_file(&sys, "test/movl_register.txt");

  // Initialize some registers for testing
  sys.registers[EAX] = 5;
  sys.registers[EDX] = 3;
  sys.registers[ECX] = 2;

  // Execute instructions
  execute_instructions(&sys);

  ASSERT_EQ(sys.registers[EAX], 3)
      << "EAX should be 3 and yours is " << sys.registers[EAX] << ".";
  ASSERT_EQ(sys.registers[EDX], 2)
      << "EDX should be 2 and yours is " << sys.registers[EDX] << ".";
  ASSERT_EQ(sys.registers[ECX], 3)
      << "ECX should be 3 and yours is " << sys.registers[ECX] << ".";
}

