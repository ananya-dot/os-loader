#include "loader.h"

Elf32_Ehdr *ehdr;
// Elf32_Phdr *phdr;
int fd;

void loader_cleanup() {
    free(ehdr);
}

void load_and_run_elf(char** exe) {
    fd = open(exe[1], O_RDONLY);
    if (fd < 0) {
        printf("error in opening file");
        return;
    }
    
    ehdr = (Elf32_Ehdr*)malloc(sizeof(Elf32_Ehdr));
    if (ehdr == NULL) {
        printf("error in maalloc");
        close(fd);
        return;
    }

    size_t size_elf = sizeof(Elf32_Ehdr);
    
    ssize_t reader = read( fd ,ehdr, size_elf);
    

    Elf32_Addr entry = ehdr->e_entry;   // setting default entry as entry of elf header entry

    for (int i = 0; i < ehdr->e_phnum; i++) {
        Elf32_Phdr phdr;
        size_t size_phdr = sizeof(Elf32_Phdr);
  
        
        lseek(fd, ehdr->e_phoff + i * ehdr->e_phentsize, SEEK_SET);

        reader = read(fd, &phdr, size_phdr);   // reading phdr contents

        // printf("hi");
        if (phdr.p_type == PT_LOAD) {
          // printf("hi");
            void *segment_address= mmap(NULL, phdr.p_memsz, PROT_READ|PROT_WRITE| PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE,0,0);
            
            lseek(fd, phdr.p_offset, SEEK_SET);
            reader = read(fd, segment_address, phdr.p_filesz);

            if (entry >= phdr.p_vaddr && entry < phdr.p_vaddr + phdr.p_memsz) {
                entry = entry + (Elf32_Addr)segment_address - phdr.p_vaddr;
                int (*_start)() = (int (*)())entry;
                // _start();
                int result = _start();
                printf("User _start return value = %d\n",result);
                break;
                
            }
        }
    }
    // printf("nothing found");
}

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: %s <ELF Executable> \n", argv[0]);
        exit(1);
    }
    
    load_and_run_elf(argv);
    loader_cleanup();
    return 0;
}
