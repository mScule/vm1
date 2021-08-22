#include <stdio.h>  // printf(), fopen(), fseek(), fread(), ftell(), fclose(), FILE
#include <stdlib.h> // malloc(), free(), atoi()
#include <stdint.h> // uint16_t

#include "..\shared\shared_macros.h" // PROJECT_NAME, TRUE, FALSE
#include "..\shared\str.h" // str_length(), str_is_equal(), str_new(), str_append()

#define VERSION "0.0.1"
#define FILE_FORMAT_NAME "vbin"
#define HEX "0x"
#define VALUE_PREFIX_ERROR_MSG "you need to give ':' before the value"

// Syntax
#define S_VALUE_FORMAT_SETTER   ':'
#define S_COMMENT               '#'

#define S_LOCATION_POINTER      '>'
#define S_LOCATION_POINTER_CALL ':'

#define S_CHAR                  '\''
#define S_STRING                '"'

#define S_ESCAPER               '\\'
#define S_NEWLINE               'n'
#define S_TAB                   't'
#define S_NULLPOINT_TERMINATOR  '\0'

char* input_buffer; long input_len;
char  cur_char;     long index = 0;

typedef struct location_pointers { char *id; uint16_t mem_loc; } loc_ptr;

// output file
long cur_mem_loc;
loc_ptr loc_ptrs[UINT16_MAX];
loc_ptr ptr_call_buffer[UINT16_MAX];
uint16_t ptrs_len = 0, ptrs_call_buf_len = 0;

char virtual_output[UINT16_MAX];
FILE *output;

int loc_ptr_exists(char* id)
{
    for(int i = 0; i < ptrs_len; i++)
        if(str_is_equal(id, loc_ptrs[i].id));
            return 1;

    return 0;
}

uint16_t get_loc_ptr(char* id)
{
    for(int i = 0; i < ptrs_len; i++)
    {
        if(str_is_equal(id, loc_ptrs[i].id));
            return loc_ptrs[i].mem_loc;
    }
    return UINT16_MAX;
}

void error(char *message)
{
    printf("%s ERROR! %s", PROJECT_NAME, message);
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

void demand_char(char ch, char* error_msg)
{
    if(cur_char != ch)
        error(error_msg);
    get_next_char();
}

void write(unsigned char bytecode)
{
    virtual_output[cur_mem_loc] = bytecode;
    cur_mem_loc++;
}

void export()
{
    printf("Export:\n");
    for(int i = 0; i < cur_mem_loc; i++)
    {
        printf("%d ", virtual_output[i]);
        fwrite(&virtual_output[i], sizeof(char), 1, output);
    }
}

int is_blank()
{
    // 0XA = newline 0XD carriage return
    if (cur_char == 0xA || cur_char == 0xD)
        return 1;
    return 0;
}

int is_alphabet()
{
    if
    (
        cur_char >= 'A' && cur_char <= 'Z' ||
        cur_char >= 'a' && cur_char <= 'z'
    )
        return 1;
    return 0;
}

int is_number()
{
    if (cur_char >= '0' && cur_char <= '9')
        return 1;
    return 0;
}

void skip_blanks()
{
    while (is_blank()) get_next_char();
}

char *build_word()
{
    char *word = str_new("");

    while(is_alphabet() || is_number() || cur_char == '_')
    {
        if(cur_char >= 'a' && cur_char <= 'z')
            cur_char -= 32;

        word = str_append(word, &cur_char);
        get_next_char();
    }

    word[str_length(word)] = '\0';

    return word;
}

void write_8bit_hex()
{
    demand_char(S_VALUE_FORMAT_SETTER, VALUE_PREFIX_ERROR_MSG);

    char *num = str_new(HEX);
    num = str_append(num, build_word());

    write((unsigned char)strtol(num, NULL, 0));

    free(num);
}

void write_16bit_hex()
{
    demand_char(S_VALUE_FORMAT_SETTER, VALUE_PREFIX_ERROR_MSG);

    char *num = str_new(HEX);
    num = str_append(num, build_word());

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

int main(int argc, char *argv[])
{
    // If no file is passed, exit
    if(argc < 2) return 0;

    printf("%s Assembler %s\n", PROJECT_NAME, VERSION);
    printf("File: %s\n", argv[1]);

    FILE *input_file = fopen(argv[1], "rb");

    // Counting file length
    fseek(input_file, 0, SEEK_END);
    input_len = ftell(input_file);
    fseek(input_file, 0, SEEK_SET);

    // Creating input buffer with input length size
    // and add contents from input file to it
    input_buffer = malloc(sizeof(char) * input_len + 1);

    fread(input_buffer, sizeof(char) * input_len, 1, input_file);
    fclose(input_file);

    input_buffer[input_len] = '\0';

    printf
    (
        "Contents:\n%s\nInput length: %d Input buffer size: %d\n",
        input_buffer,
        input_len,
        str_length(input_buffer)
    );

    // Opening outputfile for writing
    char *output_file_name;

    if(argc < 3)
    {
        output_file_name = "a";
        str_append(output_file_name, FILE_FORMAT_NAME);
    }
    else
        output_file_name = argv[2];

    // Assembling
    get_next_char();
    char *cur_word;

    while (cur_char != '\0')
    {
        skip_blanks();

        // Build word
        if(is_alphabet())
        {
            cur_word = build_word();

            // Op codes
            if      (str_is_equal(cur_word, "END")) write(0x0);
            else if (str_is_equal(cur_word, "JMP")) write(0x1);
            else if (str_is_equal(cur_word, "PBR")) write(0x2);
            else if (str_is_equal(cur_word, "NBR")) write(0x3);

            else if (str_is_equal(cur_word, "ADD")) write(0x4);
            else if (str_is_equal(cur_word, "SUB")) write(0x5);
            else if (str_is_equal(cur_word, "MUL")) write(0x6);
            else if (str_is_equal(cur_word, "DIV")) write(0x7);
            else if (str_is_equal(cur_word, "REM")) write(0x8);

            else if (str_is_equal(cur_word, "SRV")) write(0x9);
            else if (str_is_equal(cur_word, "SRR")) write(0xA);
            else if (str_is_equal(cur_word, "SRM")) write(0xB);
            else if (str_is_equal(cur_word, "SMR")) write(0xC);

            else if (str_is_equal(cur_word, "IEQ")) write(0xD);
            else if (str_is_equal(cur_word, "ILT")) write(0xE);
            else if (str_is_equal(cur_word, "IMT")) write(0xF);
            else if (str_is_equal(cur_word, "ILQ")) write(0x10);
            else if (str_is_equal(cur_word, "IMQ")) write(0x11);

            else if (str_is_equal(cur_word, "PUT")) write(0x12);

            // Registers
            else if (str_is_equal(cur_word, "GN1")) write(0x0);
            else if (str_is_equal(cur_word, "GN2")) write(0x1);
            else if (str_is_equal(cur_word, "GN3")) write(0x2);
            else if (str_is_equal(cur_word, "GN4")) write(0x3);

            // Flags
            else if (str_is_equal(cur_word, "ZRO")) write(0x0);
            else if (str_is_equal(cur_word, "POS")) write(0x1);
            else if (str_is_equal(cur_word, "NEG")) write(0x2);
            else if (str_is_equal(cur_word, "EQL")) write(0x3);
            else if (str_is_equal(cur_word, "LTH")) write(0x4);
            else if (str_is_equal(cur_word, "MTH")) write(0x5);
            else if (str_is_equal(cur_word, "LQT")) write(0x6);
            else if (str_is_equal(cur_word, "MQT")) write(0x7);

            // Data formats
            else if (str_is_equal(cur_word, "SX")) write_8bit_hex();
            else if (str_is_equal(cur_word, "DX")) write_16bit_hex();

            else if (str_is_equal(cur_word, "SI")) write_8bit_int();
            else if (str_is_equal(cur_word, "DI")) write_16bit_int();

            free(cur_word);
        }

        switch(cur_char)
        {
            case S_LOCATION_POINTER_CALL :
                get_next_char();

                char* id = build_word();

                if(loc_ptr_exists(id))
                {
                    uint16_t loc = get_loc_ptr(id);
                    write(loc);
                    write(loc >> 8);

                    printf
                    (
                        "\nNon buffered location call %s: %i",
                        id,
                        loc
                    );

                    free(id);
                }
                else
                {
                    ptr_call_buffer[ptrs_call_buf_len].id = id;
                    ptr_call_buffer[ptrs_call_buf_len].mem_loc = cur_mem_loc;
                    ptrs_call_buf_len++;

                    write(0x0);
                    write(0x0);
                }
                break;

            case S_LOCATION_POINTER :
                get_next_char();

                loc_ptrs[ptrs_len].id      = build_word();
                loc_ptrs[ptrs_len].mem_loc = cur_mem_loc;

                ptrs_len++;
                break;

            case S_COMMENT :
                while(cur_char != '\n' && cur_char != '\0')
                    get_next_char();
                break;
            
            // Character
            case '\'':
                break;

            // String
            case '"' :
                break;
        }

        get_next_char();
    }

    // Filling missing location pointers
    for(int i = 0; i < ptrs_call_buf_len; i++)
    {
        if(loc_ptr_exists(ptr_call_buffer[i].id))
        {
            uint16_t loc = get_loc_ptr(ptr_call_buffer[i].id);
            virtual_output[ptr_call_buffer[i].mem_loc] = loc;
            virtual_output[ptr_call_buffer[i].mem_loc + 1] = loc >> 8;

            printf
            (
                "\nLocation call %s: %i",
                ptr_call_buffer[i].id,
                loc
            );
        }
        else
        {
            char* errmsg = str_new("There isn't memory location specified for \"");
            errmsg = str_append(errmsg, ptr_call_buffer[i].id);
            errmsg = str_append(errmsg, "\"\0");
            error(errmsg);
        }
    }

    // Exporting
    output = fopen(output_file_name, "w+");
    export();
    fclose(output);

    free(input_buffer);

    // Printing all location pointers
    for(uint16_t i = 0; i < ptrs_len; i++)
    {
        printf("\nLocation %s: %i", loc_ptrs[i].id, loc_ptrs[i].mem_loc);
        free(loc_ptrs[i].id);
    }

    getchar();
}
