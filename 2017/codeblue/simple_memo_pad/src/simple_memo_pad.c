#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "ctflib.h"

#define BUFSIZE 0x80
#define MEMOS 20

struct Memo{
    unsigned long cookie;
    int page_no;
    int used;
    char content[BUFSIZE];
    struct Memo *next;
    struct Memo *prev;
};

extern unsigned long get_cookie();
asm (
        ".intel_syntax noprefix\n"
        "get_cookie:\n"
        "mov rax, fs:0x28\n"
        "ret\n"
        ".att_syntax noprefix\n"
        );

void fatal(){
    write_string("Something went wrong...\n");
    _exit(1);
}

struct Memo *make_notes(){
    struct Memo *head = NULL;
    struct Memo *last = NULL;
    struct Memo *memo;
    int i;

    for(i = 0; i < MEMOS; i++){
        memo = malloc(sizeof(struct Memo));
        if(memo == NULL){
            fatal();
        }

        memo->cookie = get_cookie();
        memo->page_no = i + 1;
        memo->used = 0;
        memset(memo->content, 0, BUFSIZE);
        memo->next = NULL;
        memo->prev = NULL;

        if(head == NULL){
            strcpy(memo->content, "This is a sample!");
            memo->used = 1;
            head = last = memo;
        }else{
            last->next = memo;
            memo->prev = last;
            last = memo;
        }
    }

    return head;
}

void show_splash(){
    write_string("*******************************\n");
    write_string("*       Simple Memo Pad       *\n");
    write_string("*******************************\n");
    write_string("                     Ver. Alpha\n");
    write_string("\n");
}

void show_menu(){
    write_string("\n");
    write_string("1. Write a note on a blank area\n");
    write_string("2. Edit a note\n");
    write_string("3. Delete a note\n");
    write_string("4. Show a note\n");
    write_string("5. Quit\n");
    write_string("> ");
}

void lift_heap(){
    int fd;
    size_t size = 0;

    fd = open("/dev/urandom", O_RDONLY);
    if(fd < 0){
        fatal();
    }

    read(fd, &size, 2);
    malloc(size);

    close(fd);
}

int add_note(struct Memo *list){
    struct Memo *memo;

    memo = list;
    while(memo != NULL && memo->used){
        memo = memo->next;
    }
    if(memo == NULL){
        write_string("Out of paper.\n");
        return 1;
    }
    if(memo->cookie != get_cookie()){
        write_string("Linked list corruption detected :P\n");
        _exit(1);
    }

    write_string("Content: ");
    read_until(memo->content, BUFSIZE, '\n');
    memo->used = 1;

    write_string("Done!\n");

    return 0;
}

int edit_note(struct Memo *list){
    struct Memo *memo;
    int page_no;

    write_string("Index: ");
    page_no = read_int();
    if(page_no == 1){
        write_string("You can't edit sample page.\n");
        return 1;
    }

    memo = list;
    while(memo != NULL && memo->page_no != page_no){
        memo = memo->next;
    }
    if(memo == NULL){
        write_string("Page not found.\n");
        return 1;
    }
    if(memo->cookie != get_cookie()){
        write_string("Linked list corruption detected :P\n");
        _exit(1);
    }
    if(!memo->used){
        write_string("You can't edit a blank page.\n");
        return 1;
    }

    write_string("Content: ");
    read_until(memo->content, BUFSIZE + 8, '\n');

    write_string("Done!\n");

    return 0;
}

int delete_note(struct Memo *list){
    struct Memo *memo;
    struct Memo *next, *prev;
    int page_no;

    write_string("Index: ");
    page_no = read_int();
    if(page_no == 1){
        write_string("You can't delete sample page.\n");
        return 1;
    }

    memo = list;
    while(memo != NULL && memo->page_no != page_no){
        memo = memo->next;
    }
    if(memo == NULL){
        write_string("Page not found.\n");
        return 1;
    }
    if(memo->cookie != get_cookie()){
        write_string("Linked list corruption detected :P\n");
        _exit(1);
    }
    if(!memo->used){
        write_string("You can't delete a blank page.\n");
        return 1;
    }

    prev = memo->prev;
    next = memo->next;
    if(prev != NULL){
        prev->next = next;
    }
    if(next != NULL){
        next->prev = prev;
    }

    write_string("Done!\n");

    return 0;
}

int main(){
    struct Memo *list;
    int editted = 0;
    int deleted = 0;
    int menu;
    char buf[16];

    lift_heap();

    list = make_notes();
    show_splash();
    sleep(1);

    while(1){
        show_menu();
        menu = read_int();
        if(menu == 5){
            write_string("Are you sure to quit? (y/n): ");
            read_until(buf, sizeof(buf), '\n');
            if(strcmp(buf, "y") == 0){
                break;
            }else{
                continue;
            }
        }
        switch(menu){
            case 1:  // add
                add_note(list);
                break;
            case 2:  // edit
                if(editted){
                    write_string("Sorry, you can edit a note only once.\n");
                }else{
                    if(edit_note(list) == 0){
                        editted = 1;
                    }
                }
                break;
            case 3:  // delete
                if(deleted){
                    write_string("Sorry, you can delete a note only once.\n");
                }else{
                    if(delete_note(list) == 0){
                        deleted = 1;
                    }
                }
                break;
            case 4:  // show
                write_string("Sorry, not implemented yet.\n");
                break;
            default:
                break;
        }
    }

    return 0;
}
