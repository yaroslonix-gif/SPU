IN
POP AX
PUSH AX
CALL calc_factorial
OUT
STOP
:calc_factorial
PUSH AX
PUSH 1
JBE ret
PUSH AX
PUSH 1
SUB
POP AX
PUSH AX
MUL
CALL calc_factorial
:ret
RET