#include <stdlib.h>
#include <sys/syscall.h>
#include <syscall.h>

// files

// mode(unused): enum { O_RDONLY = 0, O_WRONLY = 1, O_RDWR = 2, O_CREAT = 0x40, O_DIRECTORY = 0x10000 };
int open(char *name, struct file* file, int flags) {
     return syscall_3(SYS_open, (uint64_t) name,(uint64_t) file, flags);
}

int check_file(char* name) {
    return syscall_1(SYS_checkfile, (uint64_t) name);
}

ssize_t read(struct file* fd, void *buf, size_t count) {
    return syscall_3(SYS_read, (uint64_t) fd, (uint64_t) buf, count);
}

ssize_t write(int fd, const void *buf, size_t count) {
    return syscall_3(SYS_write, fd, (uint64_t) buf, count);
}

void* opendir(const char* name) {
    return (void*) syscall_1(SYS_opendir, (uint64_t) name);
}


int readdir(void* fd, struct dirent *dirp) {
    return syscall_2(SYS_readdir, (uint64_t) fd, (uint64_t) dirp);
}

int closedir(struct dirent* close) {
    return syscall_1(SYS_closedir, (uint64_t) close);
}

char* get_cwd(char* buf) {
    return (char*) syscall_1(SYS_getcwd, (uint64_t) buf);
}

char* set_cwd(char* buf) {
    return (char*) syscall_1(SYS_setcwd, (uint64_t) buf);
}

void read_rootfs(struct dirent *dirp){
     syscall_1(SYS_readrootfs, (uint64_t)dirp);
}

////enum { SEEK_SET = 0, SEEK_CUR = 1, SEEK_END = 2 };
////typedef uint64_t off_t;
//
//off_t lseek(int fildes, off_t offset, int whence) {
//    return syscall_3(SYS_lseek, fildes, offset, whence);
//}
//
void close(struct file* fd) {
    syscall_1(SYS_close, (uint64_t) fd);
}

//duplicate a file descriptor
//int dup(int oldfd) {
//    return syscall_1(SYS_dup, oldfd);
//}
//
//int dup2(int oldfd, int newfd) {
//    return syscall_2(SYS_dup2, oldfd, newfd);
//}
//

// processes
/* terminates the calling process "immediately".
 * Any open file descriptors belonging to the process are closed;
 * any children of the process are inherited by process 1, init,
 * and the process's parent is sent a SIGCHLD signal.
 *
 * The value status is returned to the parent process as the process's exit status,
 * and can be collected using one of the wait(2) family of calls
 */
void exit(int status) {
    syscall_1(SYS_exit, status);
}

pid_t fork(void) {
    return syscall_0(SYS_fork);
}

int execve(const char *filename, char * const argv[], char * const envp[]) {
    return syscall_3(SYS_execve, (uint64_t) filename, (uint64_t) argv,
            (uint64_t) envp);
}

pid_t getpid(void) {
    return syscall_0(SYS_getpid);
}

// get parent pid
pid_t getppid(void) {
    return syscall_0(SYS_getppid);
}

// in POSIX, return value type is int
void yield(void) {
    syscall_0(SYS_yield);
}

pid_t waitpid(pid_t pid, int *status, int options) {
    return syscall_3(SYS_wait4, pid, (uint64_t) status, options);
}

void sleep(uint32_t seconds) {
    syscall_1(SYS_sleep, seconds);
}

int kill(pid_t pid) {
   return syscall_1(SYS_kill, pid);
}

// memory
//typedef uint64_t size_t;
void* malloc(size_t size) {
    void *a = sbrk(size);
    if (a == ((void*) (-1))) {
        return NULL;
    } else {
        return a;
    }
}

//void free(void *ptr){
//    size_t length;
//    length=sizeof(*ptr);
//    syscall_2(SYS_munmap,(uint64_t)ptr,length);
//}
//
//int brk(void* addr) {
//	void* base = sbrk(0);
//
//	return sbrk(((uint64_t) addr - (uint64_t) base));
//}

//if success return previous heap break, otherwise return (void*) -1
void* sbrk(size_t size) {
    return (void*) syscall_1(SYS_sbrk, size);
}

//process related functions

int ps(ps_t ps) {
    return syscall_1(SYS_ps, (uint64_t) ps);
}

void clear_screen() {
    syscall_0(SYS_clear);
}

//
//
//// signals
//
//unsigned int alarm(unsigned int seconds){
//    return (unsigned int)syscall_1(SYS_alarm,seconds);
//}
//#define SIG_DFL ((__sighandler_t)0)
//#define SIG_IGN ((__sighandler_t)1)
//#define SIGINT     2
//#define SIGKILL    9
//#define SIGUSR1   10
//#define SIGSEGV   11
//#define SIGALRM   14
//#define SIGTERM   15
//#define SIGCHLD   17
//typedef void (*sighandler_t)(int);
//sighandler_t signal(int signum, sighandler_t handler);
//int kill(pid_t pid, int sig){
//    return syscall_2(SIGKILL,pid,sig);
//}
//
//
//// paths
//char *getcwd(char *buf, size_t size) {
//    return (char *)syscall_2(SYS_getcwd, (uint64_t)buf, size);
//}
//
//int chdir(const char *path) {
//    return syscall_1(SYS_chdir, (uint64_t)path);
//}
//
//// directories
//#define NAME_MAX 255
///*struct dirent
// {
// long d_ino;
// off_t d_off;
// unsigned short d_reclen;
// char d_name [NAME_MAX+1];
// };
// */
//
//void *opendir(const char *name) {
//    int* fd = malloc(sizeof(int));
//    if((*fd = open(name, O_RDONLY|O_DIRECTORY)) < 0) {
//        return NULL;
//    }
//    return fd;
//}
//
void *memset(void *s, int ch, size_t n) {
    char* tmp = s;
    while (n > 0) {
        *tmp = ch;
        tmp++;
        n--;
    }
    return s;
}
//
//int getdents(uint32_t fd, struct dirent *dirp, uint32_t count) {
//    return syscall_3(SYS_getdents, fd, (uint64_t)dirp, count);
//}
//
//struct dirent *readdir(void *dir) {
//
//    int BUF_SIZE=1024,nread=0;
//    //char buf[BUF_SIZE];
//    //memset(buf, 0, BUF_SIZE);
//    struct dirent* dirp=malloc(BUF_SIZE*sizeof(struct dirent*));
//    if((nread = getdents(*((int *)dir), (struct dirent *)dirp, BUF_SIZE)) <= 0) {
//        return NULL;
//    }
//    //dirp = (struct dirent *) buf;
//    return dirp;
//}
//
//int closedir(void *dir) {
//    int n;
//    n = close(*(int *)dir);
//    free(dir);
//    return n;
//}
// convert string to integer (positive)
int stoi(char* s) {
    int i = 0;
    int result = 0;

    while (s[i] >= '0' && s[i] <= '9') {
        result *= 10;
        result += (s[i] - '0');
        i++;
    }

    if (i == 0) {
        return -1;
    }

    return result;
}

#define LINE_LENGTH 1024

// return a line of given string
int read_line(char* str, char* line) {
    int i = 0;

    if (str != NULL && line != NULL) {
        while (str[i] != '\n') {
            line[i] = str[i];
            i++;
        }
    }

    line[i] = '\0';

    return i;
}
