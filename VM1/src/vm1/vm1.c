#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include "..\shared\shared_macros.h"

// Utility

void print_binary_16bit(uint16_t decimalNum)
{
    for(int32_t i = 15; i >= 0; i--)
        printf("%d", (decimalNum >> i) & 1);
}

void error(char *message)
{
    printf("%s ERROR! %s", PROJECT_NAME, message);
    getchar();
    exit(EXIT_FAILURE);
}

// Virtual machine

// Program counter
long index = 0;

// Memory
unsigned char* memory;
unsigned long  memory_len;

// Registers
enum
{
    R_GENERAL1, R_GENERAL2, R_GENERAL3, R_GENERAL4, R_COUNT
};

uint16_t registers[R_COUNT];

// Used for checking wether the register exists or not.
void reg_access(uint16_t reg)
{
    if(reg >= R_COUNT)
        error("Non existing register");
}

// Instruction set
enum
{
    I_END, I_JUMP, I_POSITIVE_BRANCH, I_NEGATIVE_BRANCH,
    I_ADDITION, I_SUBTRACTION, I_MULTIPLICATION, I_DIVISION, I_REMAINDER,
    I_SET_REG_VAL, I_SET_REG_REG, I_SET_REG_MEM, I_SET_MEM_REG,
    I_IS_EQUAL, I_IS_LESS_THAN, I_IS_MORE_THAN,
    I_IS_LESS_OR_EQUAL_TO, I_IS_MORE_OR_EQUAL_TO,
    I_PUT
};

// Flags
enum
{
    F_ZERO, F_POSITIVE, F_NEGATIVE, 
    F_EQUAL, F_LESS_THAN, F_MORE_THAN,
    F_LESS_OR_EQUAL_TO, F_MORE_OR_EQUAL_TO,
    F_COUNT
};

unsigned char flags[F_COUNT];

// Sets every flag to zero.
void reset_flags()
{
    for(char i = 0; i < F_COUNT; i++)
        flags[i] = 0;
}

// Returns flag in given index.
// Returns error if flag doesn't exist.
unsigned char get_flag(unsigned char flag)
{
    if(flag < F_COUNT)
        return flags[flag];
    else
        error("Non existing flag");
}

// Returns the value from the memory where program counter
// points to, and increments the program counter.
// Returns 8bit value.
unsigned char get_value_8bit()
{
    if(index < memory_len)
        return memory[index++];
    else
        error("End of memory");
}

// Returns the value from two sequential memory locations.
// Returns 16 bit value.
uint16_t get_value_16bit()
{
    uint16_t value = get_value_8bit();
    value += get_value_8bit() << 8;

    return value;
}

uint16_t update_flags(uint16_t reg)
{
    reset_flags();

    if      (registers[reg] >        0) flags[F_POSITIVE] = 1;
    else if (registers[reg] ==       0) flags[F_ZERO]     = 1;
    else if (registers[reg] >> 15 == 1) flags[F_NEGATIVE] = 1;
}

// Op code functions

void i_jump() { index = get_value_16bit(); }

void i_positive_branch()
{
    char     flag = get_value_8bit();
    uint16_t loc  = get_value_16bit();

    if(get_flag(flag) == 1) index = loc;
}

void i_negative_branch()
{
    char     flag = get_value_8bit();
    uint16_t loc  = get_value_16bit();

    if(get_flag(flag) == 0) index = loc;
}

void i_addition()
{
    char reg1 = get_value_8bit(), reg2 = get_value_8bit();
    reg_access(reg1);
    reg_access(reg2);

    registers[reg1] += registers[reg2];
    update_flags(reg1);
}

void i_subtraction()
{
    char reg1 = get_value_8bit(), reg2 = get_value_8bit();
    reg_access(reg1);
    reg_access(reg2);

    registers[reg1] -= registers[reg2];
    update_flags(reg1);
}

void i_multiplication()
{
    char reg1 = get_value_8bit(), reg2 = get_value_8bit();
    reg_access(reg1);
    reg_access(reg2);

    registers[reg1] *= registers[reg2];
    update_flags(reg1);
}

void i_division()
{
    char reg1 = get_value_8bit(), reg2 = get_value_8bit();
    reg_access(reg1);
    reg_access(reg2);

    registers[reg1] /= registers[reg2];
    update_flags(reg1);
}

void i_remainder()
{
    char reg1 = get_value_8bit(), reg2 = get_value_8bit();
    reg_access(reg1);
    reg_access(reg2);

    registers[reg1] %= registers[reg2];
    update_flags(reg1);
}

void i_set_reg_val()
{
    char reg     = get_value_8bit();
    uint16_t val = get_value_16bit();

    reg_access(reg);
    registers[reg] = val;

    update_flags(reg);
}

void i_set_reg_reg()
{
    char reg1 = get_value_8bit(), reg2 = get_value_8bit();
    reg_access(reg1);
    reg_access(reg2);

    registers[reg1] = registers[reg2];

    update_flags(reg1);
}

void i_set_reg_mem()
{
    char reg     = get_value_8bit();
    uint16_t mem = get_value_16bit();

    reg_access(reg);
    registers[reg] = memory[mem];

    update_flags(reg);
}

void i_set_mem_reg()
{
    uint16_t mem = get_value_16bit();
    char reg     = get_value_8bit();

    reg_access(reg);
    memory[mem] = registers[reg];

    update_flags(reg);
}

void i_is_equal()
{
    char reg1 = get_value_8bit(), reg2 = get_value_8bit();
    reg_access(reg1);
    reg_access(reg2);

    reset_flags();
    if(registers[reg1] == registers[reg2])
        flags[F_EQUAL] = 1;
}

void i_is_less_than()
{
    char reg1 = get_value_8bit(), reg2 = get_value_8bit();
    reg_access(reg1);
    reg_access(reg2);

    reset_flags();
    if(registers[reg1] < registers[reg2])
        flags[F_LESS_THAN] = 1;
}

void i_is_more_than()
{
    char reg1 = get_value_8bit(), reg2 = get_value_8bit();
    reg_access(reg1);
    reg_access(reg2);

    reset_flags();
    if(registers[reg1] > registers[reg2])
        flags[F_MORE_THAN] = 1;
}

void i_is_less_or_equal_to()
{
    char reg1 = get_value_8bit(), reg2 = get_value_8bit();
    reg_access(reg1);
    reg_access(reg2);

    reset_flags();
    if(registers[reg1] <= registers[reg2])
        flags[F_LESS_OR_EQUAL_TO] = 1;
}

void i_is_more_or_equal_to()
{
    char reg1 = get_value_8bit(), reg2 = get_value_8bit();
    reg_access(reg1);
    reg_access(reg2);

    reset_flags();
    if(registers[reg1] >= registers[reg2])
        flags[F_MORE_OR_EQUAL_TO] = 1;
}

void i_put()
{
    char reg1 = get_value_8bit(), format_i = get_value_8bit();
    reg_access(reg1);

    char *format = NULL;
    uint16_t output;

    switch(format_i)
    {
        case 0: print_binary_16bit(registers[reg1]);     break; // binary
        case 1: format = "%x"; output = registers[reg1]; break; // hex
        case 2: format = "%d"; output = registers[reg1]; break; // integer
        case 3: format = "%c"; output = registers[reg1]; break; // ascii
    }

    if(format != NULL)
        printf(format, output);
}

// Main loop
void compute()
{
    // initializing flags
    reset_flags();

    uint16_t run = 1;

    while(run)
    {
        int16_t op_code = get_value_8bit();

        switch(op_code)
        {
            case I_END:                 run = 0;                 break;
            case I_JUMP:                i_jump();                break;
            case I_POSITIVE_BRANCH:     i_positive_branch();     break;
            case I_NEGATIVE_BRANCH:     i_negative_branch();     break;

            case I_ADDITION:            i_addition();            break;
            case I_SUBTRACTION:         i_subtraction();         break;
            case I_MULTIPLICATION:      i_multiplication();      break;
            case I_DIVISION:            i_division();            break;
            case I_REMAINDER:           i_remainder();           break;

            case I_SET_REG_VAL:         i_set_reg_val();         break;
            case I_SET_REG_REG:         i_set_reg_reg();         break;
            case I_SET_REG_MEM:         i_set_reg_mem();         break;
            case I_SET_MEM_REG:         i_set_mem_reg();         break;

            case I_IS_EQUAL:            i_is_equal();            break;
            case I_IS_LESS_THAN:        i_is_less_than();        break;
            case I_IS_MORE_THAN:        i_is_more_than();        break;
            case I_IS_LESS_OR_EQUAL_TO: i_is_less_or_equal_to(); break;
            case I_IS_MORE_OR_EQUAL_TO: i_is_more_or_equal_to(); break;

            case I_PUT:                 i_put();                 break;

            default:
                error("Unsupported operation");
                break;
        }
    }
}

// Program
int main(int argc, const char* argv[])
{
    // No input file given
    if(argc < 2) return 0;

    printf("%s Virtual machine\n", PROJECT_NAME);
    printf("File: %s\n", argv[1]);

    // Counting file size and setting memory length accordingly
    FILE *file = fopen(argv[1],"rb");
    fseek(file, 0x0, SEEK_END);
    memory_len = ftell(file);

    printf("Program size: %d bytes\n", memory_len);

    // Reading file
    fseek(file, 0x0, SEEK_SET);
    memory = malloc(sizeof(char) * memory_len);
    fread(memory, sizeof(char) * memory_len, 1, file);
    fclose(file);

    // Virtual machine at work

    compute();

    printf
    (
        "\nRegisters [%d,%d,%d,%d] Flags [ZRO %d,POS %d,NEG %d,EQL %d,LTH %d,MTH %d,LQT %d,MQT %d]\n",

        registers[R_GENERAL1], registers[R_GENERAL2],
        registers[R_GENERAL3], registers[R_GENERAL4],

        flags[F_ZERO], flags[F_POSITIVE], flags[F_NEGATIVE],
        flags[F_EQUAL],
        flags[F_LESS_THAN], flags[F_MORE_THAN],
        flags[F_LESS_OR_EQUAL_TO], flags[F_MORE_OR_EQUAL_TO]
    );

    getchar();
    return 0;
}
