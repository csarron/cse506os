#include <sys/sbunix.h>
#include <sys/printf.h>
#include <sys/stdlib.h>
#include <sys/string.h>
#include <sys/process.h>

// variable and function declarations used by terminal are in sbunix.h
#define CURSOR 135
char terminal_buffer[MAX_BUFF];
int terminal_buf_count; // number of char in the buffer
extern volatile int press_over;

int user_input;
// for terminal write
int terminal_write(int fd, char *buf, int count) {
    // TODO: stdout vs stderr
    console_color = make_color(COLOR_LIGHT_GREY, COLOR_BLACK);
    print_string(buf, count);
    // TODO: return value
    return count;
}

//void local_echo() {
//    printf("%c", terminal_buffer[terminal_buf_count-1]);
//}
void clear_bottom(){
    for (size_t y = console_row+1; y < VGA_HEIGHT - 1; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            console_buffer[index] = make_vgaentry(' ', console_color);
        }
    }
}

void do_clear() {
    for (size_t y = 0; y < VGA_HEIGHT - 1; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            console_buffer[index] = make_vgaentry(' ', console_color);
        }
    }
    console_row = 0;
    console_column = 0;
}



int terminal_read(char *buf, int count) {
    // isr_keyboard puts char into terminal buffer

    // copy terminal buffer to libc scanf buffer

    press_over = 0;
    __asm__ __volatile__ ("sti");

    user_input = 1; //set local echo flag

    printf("%c", CURSOR); // input cursor ¦
    console_column--;

    clear_bottom();

//    current->task_state = TASK_BLOCKED;
//    pid_t io_pid = current->pid;
//
//    __asm__ __volatile__ ("int $0x80;");// call schedule

    while (press_over == 0) {
        // local_echo();
    }

//    task_struct* io = find_task_struct(io_pid);
//    if (io) {
//        io->task_state = TASK_READY;
//    }

    memcpy((void*) buf, (void*) terminal_buffer, count);
    int n = count > terminal_buf_count ? terminal_buf_count : count; // number of chars put to buffer

//    do {
//        if (n > count)
//            break;
//        else {
//            dprintf("[in terminal] char is %c, count is %d\n",
//                    terminal_buffer[n], n);
//            *(buf + n) = *(terminal_buffer + n);
//            n++;
//
//        }
//    } while (terminal_buffer[n] != '\n');
//    *(buf + n) ='\n';
    memset(terminal_buffer, 0, MAX_BUFF);
    terminal_buf_count = 0;

//    __asm__ __volatile__ ("cli;");
    user_input = 0;
    return n;
}

void terminal_get_char(uint8_t ch) {
    console_color = make_color(COLOR_LIGHT_GREY, COLOR_BLACK);

    if (ch == 0x08) { // backspace \b
        if (terminal_buf_count > 0) {
            terminal_buf_count--;
            terminal_buffer[terminal_buf_count] = ' ';

            console_putchar(' ');
            console_column--;

            console_column--;

            console_putchar(CURSOR); // input cursor
            console_column--;

        }

    }
//    else if (ch == 0x0A) { // line feed \n
//
//        // TODO: push content to user process waiting for input
//    }
    else { // normal char
        if (terminal_buf_count < MAX_BUFF) {
            terminal_buffer[terminal_buf_count] = ch;
            terminal_buf_count++;

            //judge whether or not to local_echo user input
            if (user_input == 1) {
                if (ch == '\n') {
                    console_putchar(' ');
                }

                console_putchar(ch);

                console_putchar(CURSOR); // input cursor ¦
                console_column--;

            }
            //dprintf("char is %c, buffer count is %d\n", ch, terminal_buf_count);
        } else {
            printf("Terminal buffer is full!! And will be cleared !\n");
            memset(terminal_buffer, 0, MAX_BUFF);
            terminal_buf_count = 0;
            // __asm__ __volatile("hlt");
        }
    }
    //terminal_local_echo();
}
//
//void terminal_local_echo () {
//    
//}
