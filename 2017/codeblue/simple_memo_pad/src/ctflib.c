#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "ctflib.h"

size_t read_until(char *buf, size_t len, char delim){
    size_t i = 0;
    ssize_t n;
    char c;

    if(len < 1){
        return 0;
    }

    while(i < len - 1){
        n = read(0, &c, 1);
        if(n == -1){
            if(errno == EAGAIN || errno == EINTR){
                continue;
            }
            exit(1);
        }
        if(n != 1){
            exit(1);
        }
        if(c == delim){
            buf[i] = '\0';
            return i - 1;
        }
        buf[i] = c;
        i += 1;
    }
    buf[i] = '\0';

    return i - 1;
}

int read_int(){
    char buf[16];

    read_until(buf, sizeof(buf), '\n');
    return atoi(buf);
}

size_t write_bytes(char *buf, size_t len){
    size_t i = 0;
    ssize_t n;

    while(i < len){
        n = write(1, buf + i, len - i);
        if(n == -1){
            if(errno == EAGAIN || errno == EINTR){
                continue;
            }
            exit(1);
        }
        i += n;
    }

    return i;
}

size_t write_string(char *buf){
    return write_bytes(buf, strlen(buf));
}

