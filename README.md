# SPU - Реализация вычислительной машины

В данном проекте реализованы 2 части:
1. Компилятор
2. Процессор

## Компилятор

**compiler.c** – программа-ассемблер, преобразует текстовый код в байт-код

### Доступные команды
```с++
enum cmds {
    NOCMD = 0x0,

    STOP = 0x00000001,
    RET = 0x00000002,
    OUT = 0x00000003,
    IN = 0x00000004,
    PUSH = 0x00000005,
    POP = 0x00000006,
    OUTC = 0x00000007,

    JMP = 0x01000110,
    JA = 0x01000111,
    JB = 0x01000112,
    JAE = 0x01000113,
    JBE = 0x01000114,
    JE = 0x01000115,
    JNE = 0x01000116,
    CALL = 0x0100011F,

    ADD = 0x00020001,
    SUB = 0x00020002,
    MUL = 0x00020003,
    DIV = 0x00020004,
    POW = 0x00020005,

    SQRT = 0x00010001,
};
```

## Процессор

**processor.c** – программа для запуска скомпилированного кода

### Компиляция компилятора
- gcc compiler/compiler.c stack_program.c base.c -o compiler

### Запуск
.\compiler name.asm 

### Компиляция процессора
gcc processor/processor.c stack_program.c base.c -o processor

### Запуск
.\processor code/bin_name.asm.bin 

## Авторские права
© 2026 [yaroslonix-gif](https://github.com/yaroslonix-gif)