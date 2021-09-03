#include <stdio.h>  // printf(), fopen(), fseek(), fread(), ftell(), fclose(), FILE
#include <stdlib.h> // malloc(), free(), atoi()
#include <stdint.h> // uint16_t

#include "..\shared\shared_macros.h" // PROJECT_NAME, TRUE, FALSE
#include "..\shared\str.h"           // str_length(), str_is_equal(), str_new(), str_append()

// Program

#define FILE_FORMAT_NAME ".vbc"
#define HEX "0x"

// Error messages

#define VALUE_PREFIX_ERROR_MSG "you need to give ':' before the value"
#define UNSUPPORTED_KEYWORD_ERROR_MSG "Unsupported keyword "

// Syntax

#define S_VALUE_FORMAT_SETTER ':'
#define S_COMMENT '#'

#define S_LOCATION_POINTER '>'
#define S_LOCATION_POINTER_CALL ':'

#define S_CHAR '\''
#define S_STRING '"'

#define S_ESCAPER '\\'
#define S_NEWLINE 'n'
#define S_TAB 't'
#define S_NULLPOINT_TERMINATOR '\0'

// Assembling

char *input_buffer;
long input_len;

char cur_char;
long index = 0;

FILE *output;

void error(char *message)
{
    printf("%s ERROR! %s", PROJECT_NAME, message);
    free(message);

    getchar();
    exit(EXIT_FAILURE);
}

void get_next_char()
{
    if (index < input_len)
        cur_char = input_buffer[index++];
    else
        cur_char = '\0';
}

void demand_char(char ch, char *error_msg)
{
    if (cur_char != ch)
        error(error_msg);
    get_next_char();
}

// Location pointers

typedef struct location_pointers
{
    char *id;
    uint16_t mem_loc;
} loc_ptr;

// Used for keeping track of the memory location for
// location pointers
long cur_mem_loc;
loc_ptr loc_ptrs[UINT16_MAX];
uint16_t loc_ptrs_len = 0;

loc_ptr loc_ptr_calls[UINT16_MAX];
uint16_t loc_ptr_calls_len = 0;

// Used for saving the intermediate version from
// the final output, so missing location pointers
// can be filled in.
char output_buffer[UINT16_MAX];

int loc_ptr_exists(char *id)
{
    for (int i = 0; i < loc_ptrs_len; i++)
        if (str_equals(id, loc_ptrs[i].id))
            return 1;

    return 0;
}

uint16_t get_loc_ptr(char *id)
{
    for (int i = 0; i < loc_ptrs_len; i++)
        if (str_equals(id, loc_ptrs[i].id))
            return loc_ptrs[i].mem_loc;

    return 0;
}

// Char recognition functions

int is_blank()
{
    // 0XA = newline 0XD carriage return
    if (cur_char == 0xA || cur_char == 0xD)
        return 1;
    return 0;
}

int is_alphabet()
{
    if (
        cur_char >= 'A' && cur_char <= 'Z' ||
        cur_char >= 'a' && cur_char <= 'z')
        return 1;
    return 0;
}

int is_number()
{
    if (cur_char >= '0' && cur_char <= '9')
        return 1;
    return 0;
}

// Input reading functions

void skip_blanks()
{
    while (is_blank())
        get_next_char();
}

char *build_word()
{
    char *word = str_new("");

    while (is_alphabet() || is_number() || cur_char == '_')
    {
        if (cur_char >= 'a' && cur_char <= 'z')
            cur_char -= 32;

        word = str_combine(word, &cur_char);
        get_next_char();
    }

    return word;
}

// Writing functions

void write(unsigned char bytecode)
{
    output_buffer[cur_mem_loc] = bytecode;
    cur_mem_loc++;
}

void write_8bit_hex()
{
    demand_char(S_VALUE_FORMAT_SETTER, VALUE_PREFIX_ERROR_MSG);

    char *num = str_new(HEX);
    num = str_combine(num, build_word());

    write((unsigned char)strtol(num, NULL, 0));

    free(num);
}

void write_16bit_hex()
{
    demand_char(S_VALUE_FORMAT_SETTER, VALUE_PREFIX_ERROR_MSG);

    char *num = str_new(HEX);
    num = str_combine(num, build_word());

    write((unsigned char)strtol(num, NULL, 0));
    write((unsigned char)((uint16_t)strtol(num, NULL, 0) >> 8));

    free(num);
}

void write_8bit_int()
{
    demand_char(S_VALUE_FORMAT_SETTER, VALUE_PREFIX_ERROR_MSG);

    char *num = build_word();

    write((unsigned char)atoi(num));

    free(num);
}

void write_16bit_int()
{
    demand_char(S_VALUE_FORMAT_SETTER, VALUE_PREFIX_ERROR_MSG);

    char *num = build_word();

    write((unsigned char)atoi(num));
    write((unsigned char)((uint16_t)atoi(num) >> 8));

    free(num);
}

void export()
{
    printf("Export:\n");

    for (int i = 0; i < cur_mem_loc; i++)
    {
        printf("%d ", output_buffer[i]);
        fwrite(&output_buffer[i], sizeof(char), 1, output);
    }
}

// Assembling functions

void write_keyword(char *word)
{
    // Op codes

    // Program flow related
    if (str_equals(word, "END"))
        write(0x0);
    else if (str_equals(word, "JMP") || str_equals(word, "JUMP"))
        write(0x1);
    else if (str_equals(word, "PBR") || str_equals(word, "POSITIVE_BRANCH"))
        write(0x2);
    else if (str_equals(word, "NBR") || str_equals(word, "NEGATIVE_BRANCH"))
        write(0x3);

    // ALU related
    else if (str_equals(word, "ADD"))
        write(0x4);
    else if (str_equals(word, "SUB") || str_equals(word, "SUBTRACT"))
        write(0x5);
    else if (str_equals(word, "MUL") || str_equals(word, "MULTIPLY"))
        write(0x6);
    else if (str_equals(word, "DIV") || str_equals(word, "DIVIDE"))
        write(0x7);
    else if (str_equals(word, "REM") || str_equals(word, "REMINDER"))
        write(0x8);

    // Memory management related
    else if (str_equals(word, "SRV") || str_equals(word, "SET_REGISTER_VALUE"))
        write(0x9);
    else if (str_equals(word, "SRR") || str_equals(word, "SET_REGISTER_REGISTER"))
        write(0xA);
    else if (str_equals(word, "SRM") || str_equals(word, "SET_REGISTER_MEMORY"))
        write(0xB);
    else if (str_equals(word, "SMR") || str_equals(word, "SET_MEMORY_REGISTER"))
        write(0xC);

    // Conditionals related
    else if (str_equals(word, "IEQ") || str_equals(word, "IS_EQUAL"))
        write(0xD);
    else if (str_equals(word, "ILT") || str_equals(word, "IS_LESS_THAN"))
        write(0xE);
    else if (str_equals(word, "IMT") || str_equals(word, "IS_MORE_THAN"))
        write(0xF);
    else if (str_equals(word, "ILQ") || str_equals(word, "IS_LESS_OR_EQUAL_TO"))
        write(0x10);
    else if (str_equals(word, "IMQ") || str_equals(word, "IS_MORE_OR_EQUAL_TO"))
        write(0x11);

    // Output related
    else if (str_equals(word, "OUT") || str_equals(word, "OUTPUT"))
        write(0x12);

    // Registers
    else if (str_equals(word, "RG1") || str_equals(word, "REGISTER1"))
        write(0x0);
    else if (str_equals(word, "RG2") || str_equals(word, "REGISTER2"))
        write(0x1);
    else if (str_equals(word, "RG3") || str_equals(word, "REGISTER3"))
        write(0x2);
    else if (str_equals(word, "RG4") || str_equals(word, "REGISTER4"))
        write(0x3);

    // Flags
    else if (str_equals(word, "ZRO") || str_equals(word, "ZERO"))
        write(0x0);
    else if (str_equals(word, "POS") || str_equals(word, "POSITIVE"))
        write(0x1);
    else if (str_equals(word, "NEG") || str_equals(word, "NEGATIVE"))
        write(0x2);
    else if (str_equals(word, "EQL") || str_equals(word, "EQUAL"))
        write(0x3);
    else if (str_equals(word, "LTH") || str_equals(word, "LESS_THAN"))
        write(0x4);
    else if (str_equals(word, "MTH") || str_equals(word, "MORE_THAN"))
        write(0x5);
    else if (str_equals(word, "LQT") || str_equals(word, "LESS_OR_EQUAL_TO"))
        write(0x6);
    else if (str_equals(word, "MQT") || str_equals(word, "MORE_OR_EQUAL_TO"))
        write(0x7);

    // Data formats

    // Hex
    else if (str_equals(word, "SX") || str_equals(word, "SINGLE_HEX"))
        write_8bit_hex();
    else if (str_equals(word, "DX") || str_equals(word, "DOUBLE_HEX"))
        write_16bit_hex();

    // Int
    else if (str_equals(word, "SI") || str_equals(word, "SINGLE_INT"))
        write_8bit_int();
    else if (str_equals(word, "DI") || str_equals(word, "DOUBLE_INT"))
        write_16bit_int();

    else // Keyword is unsupported
    {
        char *err_msg = str_new("");

        err_msg = str_combine(err_msg, UNSUPPORTED_KEYWORD_ERROR_MSG);
        err_msg = str_combine(err_msg, word);

        error(err_msg);
    }
}

int main(int argc, char *argv[])
{
    // No input file given, exit
    if (argc < 2)
        return 0;

    printf("%s Assembler %s\nFile: %s\n", PROJECT_NAME, ASM_VERSION, argv[1]);

    // Reading input file
    FILE *input_file = fopen(argv[1], "rb");

    // Counting file length
    fseek(input_file, 0x0, SEEK_END);
    input_len = ftell(input_file);
    fseek(input_file, 0x0, SEEK_SET);

    // Creating input buffer with input file length size
    // and add contents from input file to it
    input_buffer = malloc(sizeof(char) * input_len + 1);

    fread(input_buffer, sizeof(char) * input_len, 1, input_file);
    fclose(input_file);

    input_buffer[input_len] = '\0';

    printf("Program size: %d bytes\n", input_len);

    // Creating name for the output
    char *output_file_name = str_new("");

    output_file_name = str_combine(output_file_name, argv[1]);
    output_file_name = str_combine(output_file_name, FILE_FORMAT_NAME);

    output_file_name[str_length(output_file_name)] = '\0';

    printf("Exporting to: %s\n", output_file_name);

    // Assembling
    get_next_char();
    char *cur_word;

    while (cur_char != '\0')
    {
        skip_blanks();

        // Stated
        switch (cur_char)
        {
        case S_LOCATION_POINTER_CALL:
            get_next_char();

            char *id = build_word();

            if (loc_ptr_exists(id))
            {
                uint16_t loc = get_loc_ptr(id);
                write(loc);
                write(loc >> 8);

                printf("Non buffered location call %s: %i\n", id, loc);

                free(id);
            }
            else
            {
                loc_ptr_calls[loc_ptr_calls_len].id = id;
                loc_ptr_calls[loc_ptr_calls_len].mem_loc = cur_mem_loc;
                loc_ptr_calls_len++;

                write(0x0);
                write(0x0);
            }
            break;

        case S_LOCATION_POINTER:
            get_next_char();

            loc_ptrs[loc_ptrs_len].id = build_word();
            loc_ptrs[loc_ptrs_len].mem_loc = cur_mem_loc;

            loc_ptrs_len++;
            break;

        case S_COMMENT:
            while (cur_char != '\n' && cur_char != '\0')
                get_next_char();
            break;

        case '\"':           // String
            get_next_char(); // for the starting '"'
            while (cur_char != '\0' && cur_char != '"' && cur_char != '\n')
            {
                write(cur_char);
                get_next_char();
            }
            get_next_char(); // for the trailing '"'
            break;
        }

        // Keywords
        if (is_alphabet())
        {
            cur_word = build_word();
            write_keyword(cur_word);
            free(cur_word);
        }

        get_next_char();
    }

    // Filling missing location pointers
    for (int i = 0; i < loc_ptr_calls_len; i++)
    {
        if (loc_ptr_exists(loc_ptr_calls[i].id))
        {
            uint16_t loc = get_loc_ptr(loc_ptr_calls[i].id);

            output_buffer[loc_ptr_calls[i].mem_loc] = loc;
            output_buffer[loc_ptr_calls[i].mem_loc + 1] = loc >> 8;

            printf("Location call %s: %i\n", loc_ptr_calls[i].id, loc);
        }
        else
        {
            char *errmsg = str_new("There isn't memory location specified for \"");
            errmsg = str_combine(errmsg, loc_ptr_calls[i].id);
            errmsg = str_combine(errmsg, "\"");
            error(errmsg);
        }
    }

    // Exporting

    output = fopen(output_file_name, "w+");
    export();
    fclose(output);

    free(input_buffer);

    // Printing all location pointers
    for (uint16_t i = 0; i < loc_ptrs_len; i++)
    {
        printf("Location %s: %i\n", loc_ptrs[i].id, loc_ptrs[i].mem_loc);
        free(loc_ptrs[i].id);
    }

    getchar();
}
