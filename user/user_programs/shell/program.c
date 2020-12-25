#include "userlib.h"

#define MAX_CHAR_PER_LINE 70

#define COMMAND_COUNT 13
static char* commands[] = {"?", "cd", "del", "desktop", "help", "ls", "mkdir", "mkfile", "picview", "print", "random", "reboot", "start"};
static char* cmd_help[] = {"- Show help", "<path> - Change working directory", "<path> - Delete file", "- Start desktop", "- Show help", "- List directory contents", "<path> - Create directory", "<path> - Create file", "<path> - View a picture", "<path> - Print file contents", "- Output random number", "", "<path> - Execute file"};

#define WORKPATH_LEN 35

void printHelp() {
    settextcolor(0xE, 0);
    puts("Available commands:\n");
    settextcolor(7, 0);
    for (uint16_t i = 0; i < COMMAND_COUNT; ++i) {
        putch('\t');
        puts(commands[i]);
        putch(' ');
        puts(cmd_help[i]);
        putch('\n');
    }
    settextcolor(15, 0);
}

int32_t binarySearch(char* strs[], size_t size, char* key) {
    int32_t left = 0;
    int32_t right = size - 1;
    int32_t mid;

    while (left <= right) {
        mid = (left + right) / 2;
        int temp = strcmp(strs[mid], key);
        if (temp < 0) {
            left = mid + 1;  // Value is in the right half of the current selection
        } else if (temp > 0) {
            right = mid - 1;  // Value is in the left half of the current selection
        } else {
            return mid;  // Value was found
        }
    }

    return -1;  // Value was not found
}

int main() {
    randomize();
    char work_path[WORKPATH_LEN];
    getBootPath(work_path);
    settextcolor(0xD, 1);
    puts("Subscribe to PewDiePie\n\n");
    settextcolor(15, 0);
    for (;;) {
        puts(work_path);
        puts("> ");

        char input_line[MAX_CHAR_PER_LINE + 1] = {0};
        uint8_t input_pos = 0;

        for (;;) {
            KEY_t key = getkey();
            switch (key) {
                case KEY_LEFT:
                    if (input_pos > 0) {
                        move_cursor_left();
                        --input_pos;
                    }
                    break;
                case KEY_RIGHT:
                    if ((input_pos < MAX_CHAR_PER_LINE) && (input_line[input_pos] != 0)) {
                        move_cursor_right();
                        ++input_pos;
                    }
                    break;
                case KEY_BACK:
                    if (input_pos > 0) {
                        putch('\b');
                        input_line[--input_pos] = 0;
                    }
                    break;
                case KEY_ENTER:
                    puts(" <--\n");
                    input_line[input_pos] = 0;
                    goto exec_cmd;
                    break;
                default:;
                    unsigned char input_char = (unsigned char)keyToASCII(key);  // Has to be unsigned because of the if statement
                    if ((input_char >= ' ') && (input_pos < MAX_CHAR_PER_LINE)) {
                        putch(input_char);
                        input_line[input_pos] = input_char;
                        ++input_pos;
                    }
                    break;
            }

            sleepMilliSeconds(10);
        }

    exec_cmd:;

        size_t argc = 0;
        uint8_t quote = 0;
        uint8_t space = 1;

        // Find out argc
        for (size_t i = 0; i < MAX_CHAR_PER_LINE + 1; ++i) {
            if (input_line[i] == 0) {
                if ((!quote) && (!space)) {
                    ++argc;
                    break;
                }
            } else if (input_line[i] == '"') {
                quote = !quote;
            } else if (input_line[i] == ' ') {  // argument end
                if ((!quote) && (!space)) {
                    ++argc;
                    space = 1;
                }
            } else {
                space = 0;
            }
        }

        // Write a pointer for each argument to argv
        char* argv[argc];
        quote = 0;
        space = 1;
        uint8_t inarg = 0;
        size_t cur_arg = 0;
        for (size_t i = 0; input_line[i] != 0; ++i) {
            if (input_line[i] == '"') {
                quote = !quote;
                if (!quote) {
                    space = 1;
                    input_line[i] = 0;
                    inarg = 0;
                }
            } else if (input_line[i] == ' ') {  // argument end
                if ((!quote) && (!space)) {
                    space = 1;
                    input_line[i] = 0;
                    inarg = 0;
                }
            } else {
                space = 0;
                if (!inarg) {
                    argv[cur_arg] = input_line + i;
                    ++cur_arg;
                    inarg = 1;
                }
            }
        }

        /*ls [filename] [-s]*/

        int32_t cmd_index = -1;
        if (argc != 0) {
            strlwr(argv[0]);

            cmd_index = binarySearch(commands, COMMAND_COUNT, argv[0]);
            if (cmd_index == -1) {
                settextcolor(0xE, 0);
                puts("Unknown command\n");
                settextcolor(15, 0);
                continue;
            }
        }

        switch (cmd_index) {
            case 0:  // ?
                printHelp();
                break;
            case 1:  // cd
                settextcolor(7, 0);
                if (argc != 2) {
                    puts("Syntax error\n");
                } else {
                    uint8_t filepath_len = WORKPATH_LEN + strlen(argv[1]) + 2;
                    char filepath[filepath_len];
                    for (uint8_t i = 0; i < filepath_len; ++i) {
                        filepath[i] = 0;
                    }
                    if (argv[1][0] == '/' || argv[1][0] == '\\') {
                        strcpy(filepath, argv[1]);
                    } else {
                        strcpy(filepath, work_path);
                        strcat(filepath, "/");
                        strcat(filepath, argv[1]);
                    }
                    if (file_isDirectory(filepath)) {  // if filepath is directory
                        file_squashPath(filepath);
                        for (uint8_t i = 0; i < WORKPATH_LEN; ++i) {
                            work_path[i] = 0;
                        }
                        strcpy(work_path, filepath);
                    } else {
                        puts("Not found\n");
                    }
                }
                settextcolor(15, 0);
                break;
            case 2:  // del
                settextcolor(7, 0);
                if (argc != 2) {
                    puts("Syntax error\n");
                } else {
                    uint8_t filepath_len = strlen(work_path) + strlen(argv[1]) + 2;
                    char filepath[filepath_len];
                    for (uint8_t i = 0; i < filepath_len; ++i) {
                        filepath[i] = 0;
                    }
                    if (argv[1][0] == '/' || argv[1][0] == '\\') {
                        strcpy(filepath, argv[1]);
                    } else {
                        strcpy(filepath, work_path);
                        strcat(filepath, "/");
                        strcat(filepath, argv[1]);
                    }
                    puts("Deleting file...\n");

                    if (file_delete(filepath)) {
                        puts("Success\n");
                    } else {
                        puts("Not found\n");
                    }
                }
                settextcolor(15, 0);
                break;
            case 3:  // desktop
                desktop_enable();
                break;
            case 4:  // help
                printHelp();
                break;
            case 5:  // ls
                settextcolor(0xE, 0);

                file_t file_inst = {{0}, {0}, 0, 0, 0};
                uint32_t i = 0;
                while (file_findByIndex(&file_inst, work_path, i)) {
                    puts(file_inst.name);
                    putch('\t');
                    if (file_inst.attribute & FILE_ATTR_VOLLABEL)
                        puts("(LABEL)");
                    else if (file_inst.attribute & FILE_ATTR_SUBDIR)
                        puts("(DIR)");
                    else {
                        char file_size_str[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
                        uitoa(file_inst.size, file_size_str, 10);
                        puts(file_size_str);
                        putch('\t');
                        if (file_inst.attribute & FILE_ATTR_ARCHIVE) puts("(ARCH)");
                    }
                    putch('\n');
                    ++i;
                }
                settextcolor(15, 0);
                break;
            case 6:  // mkdir
                settextcolor(7, 0);

                if (argc != 2) {
                    puts("Syntax error");
                } else {
                    uint8_t filepath_len = strlen(work_path) + strlen(argv[1]) + 2;
                    char filepath[filepath_len];
                    for (uint8_t i = 0; i < filepath_len; ++i) {
                        filepath[i] = 0;
                    }
                    if (argv[1][0] == '/' || argv[1][0] == '\\') {
                        strcpy(filepath, argv[1]);
                    } else {
                        strcpy(filepath, work_path);
                        strcat(filepath, "/");
                        strcat(filepath, argv[1]);
                    }
                    puts("Creating ");
                    puts("directory...\n");

                    if (file_createDirectory(filepath)) {
                        puts("Created or already exists");
                    } else {
                        puts("Couldn't be created");
                    }
                }
                settextcolor(15, 0);
                putch('\n');
                break;
            case 7:  // mkfile
                settextcolor(7, 0);

                if (argc != 2) {
                    puts("Syntax error");
                } else {
                    uint8_t filepath_len = strlen(work_path) + strlen(argv[1]) + 2;
                    char filepath[filepath_len];
                    for (uint8_t i = 0; i < filepath_len; ++i) {
                        filepath[i] = 0;
                    }
                    if (argv[1][0] == '/' || argv[1][0] == '\\') {
                        strcpy(filepath, argv[1]);
                    } else {
                        strcpy(filepath, work_path);
                        strcat(filepath, "/");
                        strcat(filepath, argv[1]);
                    }
                    puts("Creating ");
                    puts("file...\n");

                    file_t file_inst = {{0}, {0}, 0, 0, 0};
                    if (file_create(&file_inst, filepath)) {
                        puts("Created or already exists");
                    } else {
                        puts("Couldn't be created");
                    }
                }
                settextcolor(15, 0);
                putch('\n');
                break;
            case 8:  // picview
                settextcolor(7, 0);

                if (argc != 2) {
                    puts("Syntax error");
                } else {
                    uint8_t filepath_len = strlen(work_path) + strlen(argv[1]) + 2;
                    char filepath[filepath_len];
                    for (uint8_t i = 0; i < filepath_len; ++i) {
                        filepath[i] = 0;
                    }
                    if (argv[1][0] == '/' || argv[1][0] == '\\') {
                        strcpy(filepath, argv[1]);
                    } else {
                        strcpy(filepath, work_path);
                        strcat(filepath, "/");
                        strcat(filepath, argv[1]);
                    }
                    puts("Looking for file...\n");

                    file_t file_inst = {{0}, {0}, 0, 0, 0};
                    if (file_find(&file_inst, filepath) == 0) {
                        puts("File not found");
                    } else {
                        puts("File found\n");
                        uint8_t file_buffer[file_inst.size];
                        file_readContents(&file_inst, file_buffer, 0, file_inst.size);
                        video_set_mode(2);
                        draw_picture(file_buffer, 0, 0);
                        while (getkey() == __KEY_INVALID);
                        video_set_mode(0);  // Default text mode
                    }
                }
                settextcolor(15, 0);
                putch('\n');
                break;
            case 9:  // print
                settextcolor(7, 0);

                if (argc != 2) {
                    puts("Syntax error");
                } else {
                    uint8_t filepath_len = strlen(work_path) + strlen(argv[1]) + 2;
                    char filepath[filepath_len];
                    for (uint8_t i = 0; i < filepath_len; ++i) {
                        filepath[i] = 0;
                    }
                    if (argv[1][0] == '/' || argv[1][0] == '\\') {
                        strcpy(filepath, argv[1]);
                    } else {
                        strcpy(filepath, work_path);
                        strcat(filepath, "/");
                        strcat(filepath, argv[1]);
                    }
                    puts("Looking for file...\n");

                    file_t file_inst = {{0}, {0}, 0, 0, 0};
                    if (file_find(&file_inst, filepath) == 0) {
                        puts("File not found");
                    } else {
                        puts("File found\n");
                        settextcolor(6, 0);
                        for (uint32_t i = 0; i < file_inst.size; i += 0x8000) {
                            uint32_t file_buffer_size = MIN(0x8000, file_inst.size - i);
                            uint8_t file_buffer[file_buffer_size];
                            file_readContents(&file_inst, file_buffer, i, file_buffer_size);
                            for (uint32_t j = 0; j < file_buffer_size; ++j)
                                putch(file_buffer[j]);
                        }
                    }
                }
                settextcolor(15, 0);
                putch('\n');
                break;
            case 10:  // random
                settextcolor(0xE, 0);
                char rand_str[11] = {0};
                uitoa(random(), rand_str, 10);
                puts(rand_str);
                settextcolor(15, 0);
                putch('\n');
                break;
            case 11:  // reboot
                settextcolor(0xE, 0);
                puts("Rebooting...\n");
                reboot();
                settextcolor(15, 0);
                break;
            case 12:
                settextcolor(7, 0);

                if (argc != 2) {
                    puts("Syntax error");
                } else {
                    uint8_t filepath_len = strlen(work_path) + strlen(argv[1]) + 2;
                    char filepath[filepath_len];
                    for (uint8_t i = 0; i < filepath_len; ++i) {
                        filepath[i] = 0;
                    }
                    if (argv[1][0] == '/' || argv[1][0] == '\\') {
                        strcpy(filepath, argv[1]);
                    } else {
                        strcpy(filepath, work_path);
                        strcat(filepath, "/");
                        strcat(filepath, argv[1]);
                    }
                    puts("Executing file...");

                    if (file_execute(filepath)) {
                        puts("Success\n");
                    } else {
                        puts("Failed\n");
                    }
                }
                settextcolor(15, 0);
                putch('\n');
                break;
        }
    }
}
