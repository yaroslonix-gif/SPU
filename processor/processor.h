#pragma once

#include "../base.h"
#include "../stack_program.h"
#include "../comands.h"

#define CODE_SIZE 5000
#define REGS_SIZE 16
#define RAM_SIZE 5000

//#define PROCESSOR_DEBUG_ON

#ifdef PROCESSOR_DEBUG_ON
    #define SPU_HARD_ASSERT(spu) { \
        SPUVerify(spu); \
    }
#else
    #define SPU_HARD_ASSERT(spu)
#endif

#define PROC_FAIL(err) { \
    LOGG(__STRINGIFY(err)); \
    abort(); \
}

#define FIELD_SIZE 16
#define N_ARGS_STK_POS 16
#define N_JUMP_POS 8

#define CODE_NAME_SZ 100

#define N_ARGS_POS 24
#define REG_ARGS_POS 28
#define RAM_ARGS_POS 20
/////////////////////////////////////////////////////////// ENUM ///////////////////////////////////////////////////////////
enum processor_status {
    PROCESSOR_OK = 0x0,

    PC_OUT_OF_CODE = 0x1,
    SPU_IS_NULL = 0x2,
    WRONG_PROG_ARGS = 0x3,
    
    DIVISION_BY_ZERO = 0x10,
    SCAN_ERROR = 0x11,
    SQRT_FROM_NEG = 0x12,
};

/////////////////////////////////////////////////////////// STRUCT ///////////////////////////////////////////////////////////
struct SPU {
    stack_elem_t code[CODE_SIZE];
    stack_elem_t registers[REGS_SIZE];
    stack_elem_t ram[RAM_SIZE];
    struct stack_t stack;
    struct stack_t ret_stack;
    uint64_t pc;
};


/////////////////////////////////////////////////////////// ПРОТОТИПЫ ФУНКЦИЙ ///////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////// ОБРАБОТКА КОМАНД ///////////////////////////////////////////////////////////
void CodeNameAndPathFromArgs(const int argc, const char* argv[], char* code_name);

void RunComand(struct SPU* spu);

void RunArithmeticCmd2Vals(struct SPU* spu, stack_elem_t cmd);

void RunArithmeticCmd1Val(struct SPU* spu, stack_elem_t cmd);

void RunNotArithmetic(struct SPU* spu, stack_elem_t cmd);

void RunJumps(struct SPU* spu, stack_elem_t cmd);

/////////////////////////////////////////////////////////// SPU ФУНКЦИИ ///////////////////////////////////////////////////////////
void SPUInit(struct SPU* spu);

void SPUDestroy(struct SPU* spu);

void SPUSetpc(struct SPU* spu, uint64_t pc);

stack_elem_t SPUGetCmd(struct SPU* spu);

stack_elem_t SPUGetArg(struct SPU* spu);

stack_elem_t SPUGetReg(struct SPU* spu, stack_elem_t reg);

void SPUPutReg(struct SPU* spu, stack_elem_t reg, stack_elem_t value);

void SPUPushRet(struct SPU* spu);

stack_elem_t SPUPopStkEl(struct SPU* spu);

stack_elem_t SPUPopStkRet(struct SPU* spu);

void SPUPushStkEl(struct SPU* spu, const stack_elem_t value);

void SPUDownloadCode(struct SPU* spu, const char* file_name);

enum processor_status SPUVerify(struct SPU* spu);
