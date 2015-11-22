//#include <stdarg.h>
//#include <stdio.h>
//
//void write_char(char c) {
//    write(1, &c, 1);
//}
//
////for kernel_printf
//
//int printf(const char *format, ...) {
//    va_list 	val;
//    int 		printed = 0;
//    int 		desire_length = 0;
//    int 		val_int = 0;
//    uint64_t    val_long = 0;
//    char 		val_char = 0;
//    char* 		val_string = NULL;
//    
//    va_start(val, format);
//    
//    while(*format) {
//        if (*format == '%') {
//            desire_length = 0;
//            format++;
//            if (*format <= 0x39 && *format >= 0x30) {
//                desire_length = *format - 0x30;
//                format ++;
//            }
//            switch (*format){
//                case 'd':
//                    val_int = va_arg(val, int);
//                    print_int(val_int,desire_length);
//                    break;
//                case 'c':
//                    val_char = va_arg(val, int);
//                    print_char(val_char);
//                    break;
//                case 's':
//                    val_string = va_arg(val, char*);
//                    print_string(val_string);
//                    break;
//                case 'x':
//                    val_long = va_arg(val, uint64_t);
//                    print_hex_or_ptr(val_long, hex_x);
//                    break;
//                case 'p':
//                    val_long = va_arg(val, uint64_t);
//                    print_hex_or_ptr(val_long, ptr_p);
//                    break;
//            }
//        } else if (*format == '\n'){
//                    console_column=0;
//                    console_row = (console_row+1)%VGA_HEIGHT;
//        } else {
//            write_char(*format);
//        }
//    printed++;
//    format++;
//    }
//    va_end(val);
//    return printed;
//}
//
//inline void print_char(char arg) {
//    if (arg == '\n'){
//        console_column=0;
//        console_row = (console_row+1)%VGA_HEIGHT;
//    } else {
//        write_char(arg);
//    }
//}
//
//void print_string(char* arg) {
//    while(*arg) {//be careful here
//        print_char(*arg);
//        arg++;
//    }
//}
//
//void print_int(int arg, int desire_length) {
//    int stack[10],top=-1;
//    for (int i = 0; i < 10; i ++) stack[i] = 0; 
//    if (arg < 0) {
//        print_char('-');
//        arg = -arg;
//    }
//    while (arg/10){
//        top++;
//        stack[top] = arg%10;
//        arg/=10;
//    }
//    top++;
//    stack[top]=arg;
//    if ((top + 1) < desire_length) top = desire_length - 1;
//    for (int i = top; i >= 0; i --){
//        print_char(stack[i]+'0');
//    }
//}
//
////mode = hex or ptr
//void print_hex_or_ptr(uint64_t arg,int mode) {
//    int stack[16],top=-1;
//    for (int i = 0; i < 16; i ++) stack[i] = 0; 
//    print_string("0x");
//    while (arg/16){
//        top++;
//        stack[top] = arg%16;
//        arg/=16;
//    }
//    top++;
//    stack[top]=arg;
//    if (mode == ptr_p)//if %p, print full 64-bit address
//        top = 15;
//    for (int i = top; i >= 0; i --){
//        if (stack[i] < 10) {
//            print_char(stack[i]+'0');//print 0-9
//        } else {
//            print_char(stack[i] - 10 + 'A');//print A-F
//        }
//        if (i % 4 == 0 && mode == ptr_p) {
//            print_char(' ');
//        }
//    }
//}
//