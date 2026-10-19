#include "compiler.h"

int main(const int argc, const char* argv[]) {
    START_LOGG;

    char code_name[CODE_NAME_SZ_READ] = "";
    CodeNameAndPathFromArgs(argc, argv, code_name);

    struct Compiler compiler = {};  
    InitCompiler(&compiler, code_name);

    FindAllLabelsInCode(&compiler);
    TranslateCode(&compiler);

    DestroyCompiler(&compiler);

    printf("ok");

    STOP_LOGG;
    return 0;
}

void InitCompiler(struct Compiler* compiler, const char* name_program) {
    HARD_ASSERT(compiler != NULL, NULL_ERROR);
    HARD_ASSERT(name_program != NULL, NULL_ERROR);

    snprintf(compiler->name_program_in, CODE_NAME_SZ, "code/%s", name_program);
    snprintf(compiler->name_program_out, CODE_NAME_SZ, "code/byte_code/byte_%s", name_program);
    snprintf(compiler->name_bin_program_out, CODE_NAME_SZ, "code/bin_%s.bin", name_program);

    read_file_by_lines_one_buf(&compiler->code, compiler->name_program_in);

    compiler->labels = (struct Label*)calloc(BASE_LABELS_CAPACITY, sizeof(compiler->labels[0]));
    compiler->num_of_labels = 0;
    compiler->labels_capacity = BASE_LABELS_CAPACITY;

    PrepareSz();
    PrepareHash();
}

int GetLabelPos(struct Compiler* compiler, const char* label_name) {
    HARD_ASSERT(compiler != NULL, NULL_ERROR);
    HARD_ASSERT(label_name != NULL, NULL_ERROR);

    uint64_t label_hash = CalcHashToEOS(label_name);
    for (size_t i = 0; i < compiler->num_of_labels; i++) {
        if (label_hash == compiler->labels[i].hash) {
            return compiler->labels[i].pos;
        }
    }

    return -1;
}

void AddLabel(struct Compiler* compiler, const char* label_name, size_t pos) {
    HARD_ASSERT(compiler != NULL, NULL_ERROR);
    HARD_ASSERT(label_name != NULL, NULL_ERROR);

    if (GetLabelPos(compiler, label_name) == -1) {
        if (compiler->num_of_labels >= compiler->labels_capacity) {
            compiler->labels = (struct Label*)_recalloc(compiler->labels, compiler->labels_capacity * 2, sizeof(compiler->labels[0]));
            if (compiler->labels == NULL) {
                COMPILER_FAIL();
            }
            compiler->labels_capacity *= 2;
        }
        strcpy(compiler->labels[compiler->num_of_labels].name, label_name);
        compiler->labels[compiler->num_of_labels].pos = pos;
        compiler->labels[compiler->num_of_labels].hash = CalcHashToEOS(label_name);
        compiler->num_of_labels++;
        return;
    }
    //COMPILER_FAIL();
}

void DestroyCompiler(struct Compiler* compiler) {
    HARD_ASSERT(compiler != NULL, NULL_ERROR);

    free_text(&compiler->code);
    free(compiler->labels);

    compiler->labels = NULL;
}

void FindAllLabelsInCode(struct Compiler* compiler) {
    HARD_ASSERT(compiler != NULL, NULL_ERROR);
    size_t pos = 0;
    
    for (size_t i = 0; i < compiler->code.n_lines; i++) {
        char cmd_name[MAX_CMD_LEN] = {};
        sscanf(compiler->code.data_ind[i].str, MAX_CMD_LEN_SCAN, cmd_name);
        size_t cmd_id = GetCmdIdByName(cmd_name);

        if (cmd_id == 0) {
            if (cmd_name[0] == ':') {
                AddLabel(compiler, cmd_name + 1, pos);
                char* args = compiler->code.data_ind[i].str + strlen(cmd_name);
                
                CheckStringForEmpty(args);
                continue;
            } else {
                if (cmd_name[0] == ';' || strlen(cmd_name) == 0)
                    continue;
                
                printf("%s\n", cmd_name);
                printf("%zu", i);
                LOGG("ERROR\n");
                COMPILER_FAIL();
            }
        }
        size_t n_args = (cmd_id >> N_ARGS_POS) % FIELD_SIZE;
        size_t reg_args = (cmd_id >> REG_ARGS_POS) % FIELD_SIZE;

        if (cmd_id == PUSH  || cmd_id == POP)
            n_args = 2;
        
        pos += (n_args + reg_args + 1);
    }
}

void TranslateCode(struct Compiler* compiler) {
    HARD_ASSERT(compiler != NULL, NULL_ERROR);

    FILE* file_out = fopen(compiler->name_program_out, "w");
    HARD_VERIFY(file_out != NULL, CANT_OPEN_FILE);

    FILE* bin_out = fopen(compiler->name_bin_program_out, "wb");
    HARD_VERIFY(bin_out != NULL, CANT_OPEN_FILE);
    
    for (size_t i = 0; i < compiler->code.n_lines; i++) {
        char cmd_name[MAX_CMD_LEN] = {};
        sscanf(compiler->code.data_ind[i].str, MAX_CMD_LEN_SCAN, cmd_name);
        size_t cmd_id = GetCmdIdByName(cmd_name);

        if (cmd_id == 0) {
            if (cmd_name[0] == ':')
                continue;
            
            if (cmd_name[0] == ';' || strlen(cmd_name) == 0)
                continue;
        }
        // mov
        char* args = compiler->code.data_ind[i].str + strlen(cmd_name);
        struct stack_t arguments = {};
        size_t n_args = (cmd_id >> N_ARGS_POS) % FIELD_SIZE;
        size_t reg_args = (cmd_id >> REG_ARGS_POS) % FIELD_SIZE;

        if (cmd_id == PUSH || cmd_id == POP)
            n_args = 2;

        StackInit(&arguments, MAX(1, n_args + reg_args));

        GetCmdArgs(compiler, &cmd_id, args, &arguments);
        
        fprintf(file_out, "%x ", cmd_id);
        for (size_t i = 0; i < n_args + reg_args; i++) {
            fprintf(file_out, STACK_ELEM_PRINT " ", arguments.data[i]);
        }
        fprintf(file_out, "\n");
        
        fwrite(&cmd_id, sizeof(cmd_id), 1, bin_out);
        fwrite(arguments.data, sizeof(arguments.data[0]), n_args + reg_args, bin_out);
    }
    fclose(file_out);
    fclose(bin_out);
}

void GetCmdArgs(struct Compiler* compiler, size_t* cmd_id, char *args, struct stack_t* arguments) { 
    ALOGG;
    HARD_ASSERT(compiler != NULL, NULL_ERROR);
    HARD_ASSERT(cmd_id != NULL, NULL_ERROR);
    HARD_ASSERT(args != NULL, NULL_ERROR);
    HARD_ASSERT(arguments != NULL, NULL_ERROR);

    if ((*cmd_id >> FUNC_POS) % 16 == 1) {
        char label_name[MAX_LABEL_NAME] = {};
        sscanf(args, "%" MAX_LABEL_NAME_SCAN "s", label_name);

        int pos = GetLabelPos(compiler, label_name);
        //printf("%d\n", pos);

        if (pos == -1)
            COMPILER_FAIL();
        
        StackPush(arguments, pos);
        return;
    }

    if (*cmd_id == PUSH) {
        GetPushArgs(cmd_id, args, arguments);
        return;
    }

    if (*cmd_id == POP) {
        GetPopArgs(cmd_id, args, arguments);
        return;
    }

    size_t n_args = (*cmd_id >> N_ARGS_POS) % FIELD_SIZE;
    size_t reg_args = (*cmd_id >> REG_ARGS_POS) % FIELD_SIZE;

    for (size_t i = 0; i < reg_args; i++) {
        int count = 0;
        char reg_name[10] = {};
        sscanf(args, "%s%n", reg_name, &count);
        uint64_t reg_id = GetRegisterIdByName(reg_name);

        if (reg_id > proc_registers_sz) {
            int64_t value = 0;
            sscanf(args, "%lld%n", &value, &count);
            StackPush(arguments, value);
        } else {
            StackPush(arguments, reg_id);
        }

        args += count;
    }

    for (size_t i = 0; i < n_args; i++) {
        int count = 0;
        int64_t value = 0;
        sscanf(args, "%lld%n", &value, &count);
        StackPush(arguments, value);
        args += count;
    }

    CheckStringForEmpty(args);
}

void GetPushArgs(size_t* cmd_id, char *args, struct stack_t* arguments) {
    char parameters[40] = {};
    int count = 0;
    sscanf(args, "%39s", parameters);
    char *params = parameters;
    size_t params_sz = strlen(params);
    size_t n_args = 0;

    if (params[0] == '[' && params[params_sz - 1] == ']') {
        *cmd_id |= (1 << RAM_ARGS_POS);
        params[params_sz - 1] = '\0';
        params++;
    }

    char *plus_ch = strchr(params, '+');
    if (plus_ch != NULL) {
        plus_ch[0] = '\0';
        plus_ch++;
        *cmd_id |= (1 << REG_ARGS_POS);
        *cmd_id |= (1 << N_ARGS_POS);

        uint64_t reg_id = GetRegisterIdByName(params);
        if (reg_id >= proc_registers_sz) {
            COMPILER_FAIL();
        }
        StackPush(arguments, reg_id);

        int count = 0;
        stack_elem_t arg = 0;
        if (sscanf(plus_ch, "%lld%n", &arg, &count) != 1) {
            COMPILER_FAIL();
        }
        StackPush(arguments, arg);
        plus_ch += count;

        CheckStringForEmpty(plus_ch);

        return;
    }

    uint64_t reg_id = GetRegisterIdByName(params);
    if (reg_id >= proc_registers_sz) {
        stack_elem_t arg = 0;
        if (sscanf(params, "%lld%n", &arg, &count) != 1) {
            COMPILER_FAIL();
        }

        StackPush(arguments, 0);
        StackPush(arguments, arg);
        params += count;
        *cmd_id |= (1 << N_ARGS_POS);
    } else {
        StackPush(arguments, reg_id);
        StackPush(arguments, 0);
        *cmd_id |= (1 << REG_ARGS_POS);
        params += strlen(params);
    }

    CheckStringForEmpty(params);
}

void GetPopArgs(size_t* cmd_id, char *args, struct stack_t* arguments) {
    char parameters[40] = {};
    int count = 0;
    sscanf(args, "%39s", parameters);
    char *params = parameters;
    size_t params_sz = strlen(params);
    size_t n_args = 0;

    char *plus_ch = strchr(params, '+');

    if (params[0] == '[' && params[params_sz - 1] == ']') {
        *cmd_id |= (1 << RAM_ARGS_POS);
        params[params_sz - 1] = '\0';
        params++;

        if (plus_ch != NULL) {
            plus_ch[0] = '\0';
            plus_ch++;
            *cmd_id |= (1 << REG_ARGS_POS);
            *cmd_id |= (1 << N_ARGS_POS);

            uint64_t reg_id = GetRegisterIdByName(params);
            if (reg_id >= proc_registers_sz) {
                COMPILER_FAIL();
            }
            StackPush(arguments, reg_id);

            int count = 0;
            stack_elem_t arg = 0;
            if (sscanf(plus_ch, "%lld%n", &arg, &count) != 1) {
                COMPILER_FAIL();
            }
            StackPush(arguments, arg);
            plus_ch += count;

            CheckStringForEmpty(plus_ch);

            return;
        }
    }

    uint64_t reg_id = GetRegisterIdByName(params);
    if (reg_id >= proc_registers_sz) {
        stack_elem_t arg = 0;
        sscanf(params, "%lld%n", &arg, &count);

        StackPush(arguments, 0);
        StackPush(arguments, arg);
        params += count;
        *cmd_id |= (1 << N_ARGS_POS);

        return;
    } else {
        StackPush(arguments, reg_id);
        StackPush(arguments, 0);
        *cmd_id |= (1 << REG_ARGS_POS);
        params += strlen(params);
    }

    CheckStringForEmpty(params);
}

uint64_t GetCmdIdByName(const char* cmd_name) {
    HARD_ASSERT(cmd_name != NULL, NULL_ERROR);

    uint64_t cmd_hash = CalcHash(cmd_name, strlen(cmd_name));
    for (size_t i = 0; i < comands_sz; i++) {
        if (cmd_hash == comands[i].hash)
            return comands[i].cmd_id;
    }

    return 0;
}

uint64_t GetRegisterIdByName(const char* reg_name) {
    HARD_ASSERT(reg_name != NULL, NULL_ERROR);

    uint64_t reg_hash = CalcHash(reg_name, strlen(reg_name));
    for (size_t i = 0; i < proc_registers_sz; i++) {
        if (reg_hash == proc_registers[i].hash)
            return proc_registers[i].reg_id;
    }

    return proc_registers_sz + 1;
}


int CheckStringForEmpty(const char* source) {
    char str[100] = "";
    if (sscanf(source, "%99s", str) <= 0)
        return 1;
    
    if (str[0] == ';')
        return 1;

    COMPILER_FAIL();
    return 0;
}

void PrepareHash() {
    for (size_t i = 0; i < comands_sz; i++)
        comands[i].hash = CalcHash(comands[i].name.str, comands[i].name.len);

    for (size_t i = 0; i < proc_registers_sz; i++)
        proc_registers[i].hash = CalcHash(proc_registers[i].name.str, proc_registers[i].name.len);
}

void PrepareSz() {
    for (size_t i = 0; i < comands_sz; i++)
        comands[i].name.len = strlen(comands[i].name.str);

    for (size_t i = 0; i < proc_registers_sz; i++)
        proc_registers[i].name.len = strlen(proc_registers[i].name.str);
}

uint64_t CalcHashToEOS(const char* data) {  // хеш djb2
    HARD_ASSERT(data != NULL, NULL_ERROR);

    uint64_t hash = 5381;

    for (size_t i = 0; data[i] != 0; i++) {
        hash = ((hash << 5) + hash) + data[i];
    }

    return hash;
}

void CodeNameAndPathFromArgs(const int argc, const char* argv[], char* code_name) {
    HARD_ASSERT(argv != NULL, NULL_ERROR);
    HARD_ASSERT(code_name != NULL, NULL_ERROR);

    if (argc != 2)
        COMPILER_FAIL(WRONG_COMP_PROG_ARGS);
    
    strncat(code_name, argv[1], CODE_NAME_SZ - strlen(code_name));
}

/////////////////////////////////////////////////////////// GLOBAL ///////////////////////////////////////////////////////////
struct comand comands[] = {
    {{"NOCMD", 0}, 0, NOCMD},

    {{"PUSH", 0}, 0, PUSH},
    {{"POP",  0}, 0, POP},

    {{"ADD", 0}, 0, ADD},
    {{"SUB", 0}, 0, SUB},
    {{"MUL", 0}, 0, MUL},
    {{"DIV", 0}, 0, DIV},
    {{"POW", 0}, 0, POW},

    {{"STOP", 0}, 0, STOP},
    {{"RET",  0}, 0, RET},
    {{"OUT",  0}, 0, OUT},
    {{"OUTC", 0}, 0, OUTC},
    {{"IN",   0}, 0, IN},

    {{"JMP", 0}, 0, JMP},
    {{"JA",  0}, 0, JA},
    {{"JB",  0}, 0, JB},
    {{"JAE", 0}, 0, JAE},
    {{"JBE", 0}, 0, JBE},
    {{"JE",  0}, 0, JE},
    {{"JNE", 0}, 0, JNE},
    {{"CALL", 0}, 0, CALL},

    {{"SQRT", 0}, 0, SQRT},
};

struct proc_register proc_registers[] = {
    {{"OX", 0}, 0, OX},

    {{"AX", 0}, 0, AX},
    {{"BX", 0}, 0, BX},
    {{"CX", 0}, 0, CX},
    {{"DX", 0}, 0, DX},
    {{"EX", 0}, 0, EX},
    {{"FX", 0}, 0, FX},
    {{"GX", 0}, 0, GX},
};

size_t comands_sz = sizeof(comands) / sizeof(comands[0]);

size_t proc_registers_sz = sizeof(proc_registers) / sizeof(proc_registers[0]);