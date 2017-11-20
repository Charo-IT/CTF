#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFSIZE 0x100
#define LETTERS 5

struct Letter {
    int used;
    size_t length;
    unsigned char buf[BUFSIZE];
};

void fatal(){
    puts("Something went wrong.\n");
    _exit(1);
}

int read_int(){
    char buf[16];
    fgets(buf, sizeof(buf), stdin);
    return atoi(buf);
}

size_t read_string(char *buf, size_t length){
    size_t i = 0;
    char c;

    while(i < length - 1){
        if(fread(&c, sizeof(char), 1, stdin) != 1){
            fatal();
        }
        if(c == '\n'){
            break;
        }
        buf[i++] = c;
    }
    buf[i] = '\0';

    return i;
}

size_t write_plain(FILE *fp, unsigned char *buf, size_t length){
    size_t size_written;

    size_written = fwrite(buf, sizeof(unsigned char), length, fp);
    if(size_written != length){
        fatal();
    }

    return size_written;
}

size_t write_xor(FILE *fp, unsigned char *buf, size_t length){
    unsigned char temp_buf[BUFSIZE] = {};
    size_t i;
    size_t size_written;

    for(i = 0; i < length; i++){
        temp_buf[i] = buf[i] ^ 0xff;
    }

    size_written = write_plain(fp, temp_buf, length);

    return size_written;
}

size_t write_reverse(FILE *fp, unsigned char *buf, size_t length){
    unsigned char temp_buf[BUFSIZE] = {};
    size_t i;
    size_t size_written;

    for(i = 0; i < length; i++){
        temp_buf[i] = buf[length - i - 1];
    }

    size_written = write_plain(fp, temp_buf, length);

    return size_written;
}

void show_splash(){
    puts("*** Secret Mailer Service ***");
    puts("Welcome to Secret Mailer Service!");
    puts("Post your secret letters here ;)");
}

void print_menu(){
    puts("");
    puts("1. Add a letter");
    puts("2. Delete a letter");
    puts("3. Post a letter");
    puts("4. Quit");
    printf("> ");
}

void print_filters(){
    puts("0. No filter");
    puts("1. XOR filter");
    puts("2. Reverse filter");
    printf("> ");
}

size_t (*filters[])(FILE*, unsigned char*, size_t length) = {
    write_plain,
    write_xor,
    write_reverse
};

void add_letter(struct Letter *letters){
    int i;
    size_t length;

    for(i = 0; i < LETTERS; i++){
        if(!letters[i].used){
            break;
        }
    }
    if(i == LETTERS){
        puts("Too many letters :P");
        return;
    }

    printf("\nInput your contents: ");
    letters[i].length = read_string(letters[i].buf, BUFSIZE);
    letters[i].used = 1;

    puts("\nDone!");
    return;
}

void delete_letter(struct Letter *letters){
    int id;

    puts("\nWhich letter do you want to delete?");
    printf("ID (0-%d): ", LETTERS - 1);
    
    id = read_int();

    if(id < 0 || id >= LETTERS || !letters[id].used){
        puts("Invalid ID.");
        return;
    }

    letters[id].used = 0;
    letters[id].length = 0;
    memset(letters[id].buf, 0, BUFSIZE);

    puts("\nDone!");
    return;
}

void post_letter(struct Letter *letters, FILE *fp){
    int id;
    int filter;

    puts("\nWhich letter do you want to post?");
    printf("ID (0-%d): ", LETTERS - 1);

    id = read_int();

    if(id < 0 || id >= LETTERS || !letters[id].used){
        puts("Invalid ID.");
        return;
    }

    puts("\nWhich filter do you want to apply?");
    print_filters();

    filter = read_int();
    if(filter >= (int)(sizeof(filters) / sizeof(void*))){
        puts("Invalid filter.");
        return;
    }

    filters[filter](fp, letters[id].buf, letters[id].length);

    puts("\nDone!");
    return;
}

int func(){
    struct {
        FILE *fp;
        int end;
        int selection;
        struct Letter letters[LETTERS];
    } locals;

    locals.end = 0;
    memset(locals.letters, 0, sizeof(locals.letters));;

    locals.fp = fopen("/dev/null", "a");
    if(locals.fp == NULL){
        fatal();
    }

    show_splash();
    sleep(2);

    while(!locals.end){
        print_menu();
        locals.selection = read_int();
        switch(locals.selection){
            case 1:  // add
                add_letter(locals.letters);
                break;
            case 2:  // delete
                delete_letter(locals.letters);
                break;
            case 3:  // post
                post_letter(locals.letters, locals.fp);
                break;
            case 4:  // quit
                locals.end = 1;
                break;
        }
    }

    puts("Thank you for using our service :)");
    fclose(locals.fp);

    return 0;
}

int main(){
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);
    return func();
}
