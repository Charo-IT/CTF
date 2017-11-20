#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#define MAX_PROGRAM_SIZE 512

#define MEMORY_SIZE 0x10000

typedef union {
    unsigned short v;
    unsigned long padding;
} reg;

typedef struct {
    reg pc;
    reg delta_pc;
    reg ax;
    reg bx;
    reg cx;
    reg px;
} cpu;

char **program;
unsigned char *memory;
cpu *vmcpu;
unsigned char dummybuf;

unsigned short showtime(){
#ifdef DEBUG
    int count = 0;
#endif

    memset(vmcpu, 0, sizeof(cpu));

    while(program[vmcpu->pc.v] != NULL){
#ifdef DEBUG
        count += 1;
        printf("0x%04x: %s", vmcpu->pc.v, program[vmcpu->pc.v]);
        printf("\tax: 0x%x, bx: 0x%x, cx: 0x%x, px: 0x%x, delta_pc: 0x%x -> ", vmcpu->ax.v, vmcpu->bx.v, vmcpu->cx.v, vmcpu->px.v, vmcpu->delta_pc.v);
#endif
        snprintf(&dummybuf, 1, program[vmcpu->pc.v],
            vmcpu->ax.v,  // 1
            vmcpu->bx.v,  // 2
            vmcpu->cx.v,  // 3
            vmcpu->px.v,  // 4
            memory[vmcpu->px.v],  // 5
            &vmcpu->ax.v,  // 6
            &vmcpu->bx.v,  // 7
            &vmcpu->cx.v,  // 8
            &vmcpu->px.v,  // 9
            memory + vmcpu->px.v, // 10
            vmcpu->ax.v == 0,  // 11
            (vmcpu->ax.v & 0x8000) != 0,  // 12
            &vmcpu->delta_pc.v,  // 13
            0  // 14
        );
#ifdef DEBUG
        printf("ax: 0x%x, bx: 0x%x, cx: 0x%x, px: 0x%x, delta_pc: 0x%x\n", vmcpu->ax.v, vmcpu->bx.v, vmcpu->cx.v, vmcpu->px.v, vmcpu->delta_pc.v);
#endif
        vmcpu->pc.v += 1;
        vmcpu->pc.v += vmcpu->delta_pc.v;
        vmcpu->delta_pc.v = 0;
    }

#ifdef DEBUG
    printf("inscount = %d\n", count);
#endif

    return vmcpu->ax.v;
}

char **load_program(char *filename){
    FILE *fp;
    char **p;
    char *line;
    unsigned long i;
    size_t len;

    fp = fopen(filename, "rb");
    if(fp == NULL){
        perror("fopen");
        exit(1);
    }

    p = calloc(MAX_PROGRAM_SIZE, sizeof(char *));
    if(p == NULL){
        perror("calloc");
        fclose(fp);
        exit(1);
    }

    for(i = 0; i < MAX_PROGRAM_SIZE; i++){
        line = NULL;
        if(getline(&line, &len, fp) < 0){
            break;
        }
        p[i] = line;
    }

    fclose(fp);

    return p;
}

unsigned char *initialize_memory(char *input){
    unsigned char *p;
    int fd;

    fd = open(input, O_RDONLY);
    if(fd < 0){
        perror("open");
        exit(1);
    }

    p = calloc(MEMORY_SIZE, 1);
    if(p == NULL){
        perror("calloc");
        close(fd);
        exit(1);
    }

    read(fd, p, 0x100);
    close(fd);

    return p;
}

void initialize_vm(){
    vmcpu = calloc(sizeof(cpu), 1);
    if(vmcpu == NULL){
        perror("calloc");
        exit(1);
    }
}

int main(int argc, char **argv){
    unsigned short result;

    if(argc < 2){
        fprintf(stderr, "Usage: %s program_file [input_file]\n", argv[0]);
        exit(1);
    }

    program = load_program(argv[1]);
    if(argc >= 3){
        memory = initialize_memory(argv[2]);
    }else{
        memory = initialize_memory("/dev/null");
    }

    initialize_vm();

    result = showtime();

    if(result == 0){
        puts("Congratz!");
    }else{
        puts("Nope :(");
    }

    return 0;
}
