#pragma once

#include "../base.h"
#include "../stack_program.h"
#include "../comands.h"

#define FUNC_POS 8  // 1 - JMP - ы

#define N_ARGS_POS 24
#define REG_ARGS_POS 28
#define RAM_ARGS_POS 20

#define MAX_LABEL_NAME 30
#define MAX_LABEL_NAME_SCAN "29"
#define MAX_CMD_LEN 100
#define MAX_CMD_LEN_SCAN "%99s"
#define BASE_LABELS_CAPACITY 20
#define FIELD_SIZE 16

#define CODE_NAME_SZ_READ (CODE_NAME_SZ - 20)
#define CODE_NAME_SZ 100

#define COMPILER_FAIL(err) { \
    LOGG(__STRINGIFY(err)); \
    abort(); \
}

/////////////////////////////////////////////////////////// ENUM ///////////////////////////////////////////////////////////
enum compiler_status {
    COMPILER_OK = 0x0,

    NOT_ENOUGH_ARGS_FOR_RUN_COMAND = 0x1,
    DIVISION_BY_ZERO = 0x2,
    WRONG_COMP_PROG_ARGS = 0x3,
};

enum regs {
    OX = 0x0,

    AX = 0x1,
    BX = 0x2,
    CX = 0x3,
    DX = 0x4,
    EX = 0x5,
    FX = 0x6,
    GX = 0x7,
};

/////////////////////////////////////////////////////////// STRUCT ///////////////////////////////////////////////////////////
struct comand{
    struct String name;
    uint64_t hash;
    enum cmds cmd_id;
};

struct proc_register{
    struct String name;
    uint64_t hash;
    enum regs reg_id;
};

struct Label {
    int64_t pos;
    uint64_t hash;
    char name[MAX_LABEL_NAME];
};

struct Compiler {
    struct Text code;
    struct Label* labels;
    
    char name_program_in[CODE_NAME_SZ];
    char name_program_out[CODE_NAME_SZ];
    char name_bin_program_out[CODE_NAME_SZ];

    size_t num_of_labels;
    size_t labels_capacity;
    size_t now_line;
};


/////////////////////////////////////////////////////////// GLOBAL ///////////////////////////////////////////////////////////
extern struct comand comands[];

extern struct proc_register proc_registers[];

extern size_t comands_sz;

extern size_t proc_registers_sz;

/////////////////////////////////////////////////////////// ПРОТОТИПЫ ФУНКЦИЙ ///////////////////////////////////////////////////////////
void CodeNameAndPathFromArgs(const int argc, const char* argv[], char* code_name);

void InitCompiler(struct Compiler* compiler, const char* name_program);

int GetLabelPos(struct Compiler* compiler, const char* label_name);

void AddLabel(struct Compiler* compiler, const char* label_name, size_t pos);

void DestroyCompiler(struct Compiler* compiler);

void FindAllLabelsInCode(struct Compiler* compiler);

void TranslateCode(struct Compiler* compiler);

uint64_t GetCmdIdByName(const char* cmd_name);

uint64_t GetRegisterIdByName(const char* reg_name);

void GetCmdArgs(struct Compiler* compiler, size_t* cmd_id, char *args, struct stack_t* arguments);

int CheckStringForEmpty(const char* source);

void PrepareHash();

void PrepareSz();

uint64_t CalcHashToEOS(const char* data);

void GetPushArgs(size_t* cmd_id, char *args, struct stack_t* arguments);

void GetPopArgs(size_t* cmd_id, char *args, struct stack_t* arguments);