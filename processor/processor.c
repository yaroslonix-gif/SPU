#include "processor.h"

int main(const int argc, const char* argv[]) {
    START_LOGG;

    char code_name[CODE_NAME_SZ] = "";
    CodeNameAndPathFromArgs(argc, argv, code_name);

    struct SPU spu = {};
    SPUInit(&spu);
    SPUDownloadCode(&spu, code_name);

    while (true) {
        RunComand(&spu);
    }

    STOP_LOGG;
    return 0;
}

/////////////////////////////////////////////////////////// ОБРАБОТКА КОМАНД ///////////////////////////////////////////////////////////
void CodeNameAndPathFromArgs(const int argc, const char* argv[], char* code_name) {
    if (argc != 2)
        PROC_FAIL(WRONG_PROG_ARGS);
    
    strncat(code_name, argv[1], CODE_NAME_SZ - strlen(code_name));
}

void RunComand(struct SPU* spu) {
    HARD_ASSERT(spu != NULL, NULL_ERROR);

    stack_elem_t cmd = SPUGetCmd(spu);

    if ((cmd >> N_ARGS_STK_POS) % FIELD_SIZE == 2) {
        RunArithmeticCmd2Vals(spu, cmd);
        return;
    }

    if ((cmd >> N_ARGS_STK_POS) % FIELD_SIZE == 1) {
        RunArithmeticCmd1Val(spu, cmd);
        return;
    }

    if ((cmd >> N_JUMP_POS) % FIELD_SIZE == 1) {
        RunJumps(spu, cmd);
        return;
    }

    if ((cmd >> N_ARGS_STK_POS) % FIELD_SIZE == 0) {
        RunNotArithmetic(spu, cmd);
        return;
    }
}

void RunArithmeticCmd2Vals(struct SPU* spu, stack_elem_t cmd) {
    SPU_HARD_ASSERT(spu);

    stack_elem_t second_elem = SPUPopStkEl(spu);
    stack_elem_t first_elem = SPUPopStkEl(spu);
    stack_elem_t result = 0;

    switch (cmd)
    {
    case ADD:
        result = first_elem + second_elem;
        break;
    
    case SUB:
        result = first_elem - second_elem;
        break;

    case MUL:
        result = first_elem * second_elem;
        break;
    
    case DIV:
        if ( second_elem == 0)
            PROC_FAIL(DIVISION_BY_ZERO);
        
        result = first_elem / second_elem;
        break;
    
    case POW:
        result = pow(first_elem, second_elem);
        break;
    
    default:
        break;
    }

    SPUPushStkEl(spu, result);
}

void RunArithmeticCmd1Val(struct SPU* spu, stack_elem_t cmd) {
    SPU_HARD_ASSERT(spu);

    stack_elem_t elem = SPUPopStkEl(spu);
    stack_elem_t result = 0;

    switch (cmd)
    {
    case SQRT:
        if (elem < 0)
            PROC_FAIL(SQRT_FROM_NEG);
        
        result = sqrt(elem);
        break;
    
    default:
        break;
    }

    SPUPushStkEl(spu, result);
}


void RunNotArithmetic(struct SPU* spu, stack_elem_t cmd) {
    SPU_HARD_ASSERT(spu);

    stack_elem_t second_elem = 0;
    stack_elem_t first_elem = 0;
    stack_elem_t reg = 0;
    stack_elem_t value = 0;

    if ((cmd & 0x000fffff) == 5) {
        stack_elem_t second_value = 0;
        if ((cmd >> REG_ARGS_POS) % 16 == 1) {
            value = SPUGetReg(spu, SPUGetArg(spu));
        } else {
            SPUGetArg(spu);
        }

        if ((cmd >> N_ARGS_POS) % 16 == 1) {
            second_value = SPUGetArg(spu);
        } else {
            SPUGetArg(spu);
        }

        value += second_value;
        if ((cmd >> RAM_ARGS_POS) % 16 == 1) {
            SPUPushStkEl(spu, spu->ram[value]);
        } else {
            SPUPushStkEl(spu, value);
        }

        return;
    }

    if ((cmd & 0x000fffff) == 6) {
        stack_elem_t second_reg = 0;
        if ((cmd >> REG_ARGS_POS) % 16 == 1) {
            reg = SPUGetArg(spu);
        } else {
            SPUGetArg(spu);
        }

        if ((cmd >> N_ARGS_POS) % 16 == 1) {
            second_reg = SPUGetArg(spu);
        } else {
            SPUGetArg(spu);
        }

        stack_elem_t pop_value = SPUPopStkEl(spu);

        if ((cmd >> RAM_ARGS_POS) % 16 == 1) {
            if ((cmd >> REG_ARGS_POS) % 16 == 1) {
                reg = SPUGetReg(spu, reg);
            }
            reg += second_reg;
            spu->ram[reg] = pop_value;

        } else if ((cmd >> REG_ARGS_POS) % 16 == 1) {
            SPUPutReg(spu, reg, pop_value);
        }

        return;
    }

    switch (cmd)
    {
    case RET:
        reg = SPUPopStkRet(spu);
        SPUSetpc(spu, reg);
        break;

    case OUT:
        printf(STACK_ELEM_PRINT, SPUPopStkEl(spu));
        break;

    case OUTC:
        printf("%c", SPUPopStkEl(spu));
        break;
    
    case IN:
        printf("Enter number:\n");
        if (scanf(STACK_ELEM_SCAN, &value) <= 0)
            PROC_FAIL(SCAN_ERROR);
        
        SPUPushStkEl(spu, value);
        break;
    
    default:
        break;
    }
}

void RunJumps(struct SPU* spu, stack_elem_t cmd) {
    SPU_HARD_ASSERT(spu);

    stack_elem_t addr = SPUGetArg(spu);
    if (cmd == JMP) {
        SPUSetpc(spu, addr);
        return;
    }

    if (cmd == CALL) {
        SPUPushRet(spu);
        SPUSetpc(spu, addr);
        return;
    }

    stack_elem_t second_elem = SPUPopStkEl(spu);
    stack_elem_t first_elem = SPUPopStkEl(spu);
    
    switch (cmd)
    {    
    case JA:
        if (first_elem > second_elem)
            SPUSetpc(spu, addr);
        break;

    case JB:
        if (first_elem < second_elem)
            SPUSetpc(spu, addr);
        break;
    
    case JAE:
        if (first_elem >= second_elem)
            SPUSetpc(spu, addr);
        break;

    case JBE:
        if (first_elem <= second_elem)
            SPUSetpc(spu, addr);
        break;
    
    case JE:
        if (first_elem == second_elem)
            SPUSetpc(spu, addr);
        break;

    case JNE:
        if (first_elem != second_elem)
            SPUSetpc(spu, addr);
        break;
    
    default:
        break;
    }
}


/////////////////////////////////////////////////////////// SPU ФУНКЦИИ ///////////////////////////////////////////////////////////
void SPUInit(struct SPU* spu) {
    StackInit(&spu->stack, 10);
    StackInit(&spu->ret_stack, 10);
}

void SPUDestroy(struct SPU* spu) {
    SPU_HARD_ASSERT(spu);

    StackDestroy(&spu->stack);
    StackDestroy(&spu->ret_stack);
}

void SPUSetpc(struct SPU* spu, uint64_t pc) {
    SPU_HARD_ASSERT(spu);
    HARD_VERIFY(pc < CODE_SIZE, PC_OUT_OF_CODE);

    spu->pc = pc;
}

stack_elem_t SPUGetCmd(struct SPU* spu) {
    SPU_HARD_ASSERT(spu);

    stack_elem_t cmd = spu->code[spu->pc];
    if (cmd == STOP) {
        SPUDestroy(spu);
        PROC_FAIL("PROGRAM END");
    }

    spu->pc++;
    return cmd;
}

stack_elem_t SPUGetArg(struct SPU* spu) {
    SPU_HARD_ASSERT(spu);

    stack_elem_t arg = spu->code[spu->pc];
    spu->pc++;
    return arg;
}

stack_elem_t SPUGetReg(struct SPU* spu, stack_elem_t reg) {
    SPU_HARD_ASSERT(spu);

    return spu->registers[reg];
}

void SPUPutReg(struct SPU* spu, stack_elem_t reg, stack_elem_t value) {
    SPU_HARD_ASSERT(spu);

    spu->registers[reg] = value;
}

void SPUPushRet(struct SPU* spu) {
    SPU_HARD_ASSERT(spu);

    stack_elem_t ret = spu->pc;
    StackPush(&spu->ret_stack, ret);
}

stack_elem_t SPUPopStkEl(struct SPU* spu) {
    SPU_HARD_ASSERT(spu);

    return StackPop(&spu->stack);
}

stack_elem_t SPUPopStkRet(struct SPU* spu) {
    SPU_HARD_ASSERT(spu);

    return StackPop(&spu->ret_stack);
}

void SPUPushStkEl(struct SPU* spu, const stack_elem_t value) {
    SPU_HARD_ASSERT(spu);

    StackPush(&spu->stack, value);
}

void SPUDownloadCode(struct SPU* spu, const char* file_name) {
    SPU_HARD_ASSERT(spu);
    HARD_ASSERT(file_name != NULL, NULL_ERROR);

    FILE* bin_file = fopen(file_name, "rb");
    HARD_VERIFY(bin_file != NULL, CANT_OPEN_FILE);

    size_t code_sz = get_file_size_by_file_name(file_name) / sizeof(spu->code[0]);

    fread(&spu->code, sizeof(spu->code[0]), code_sz, bin_file);

    fclose(bin_file);
}

enum processor_status SPUVerify(struct SPU* spu) {
    HARD_VERIFY(spu != NULL, SPU_IS_NULL);

    struct debug_params stk_debug_params = STACK_DEBUG_VALUES_FOR_DUMP;
    
    STACK_HARD_ASSERT((&spu->stack));
    STACK_HARD_ASSERT((&spu->ret_stack));
    HARD_VERIFY(spu->pc < CODE_SIZE, PC_OUT_OF_CODE);

    return PROCESSOR_OK;
}