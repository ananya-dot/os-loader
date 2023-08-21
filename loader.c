#include "loader.h"

Elf32_Ehdr *ehdr;
int fd;

void loader_cleanup() {
    free(ehdr);
}

void load_and_run_elf(char** exe) {
    fd = open(exe[1], O_RDONLY);
    
    if (fd < 0) {
        perror("open");
        return;
    }
    
    ehdr = (Elf32_Ehdr *)malloc(sizeof(Elf32_Ehdr));
    if (ehdr == NULL) {
        perror("malloc");
        close(fd);
        return;
    }
    

    if (read(fd, ehdr, sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr)) {
        perror("read");
        free(ehdr);
        close(fd);
        return;
    }

    Elf32_Addr entry = ehdr->e_entry;
    
    
    for (int i = 0; i < ehdr->e_phnum; ++i) {
        Elf32_Phdr phdr;
  
        
        lseek(fd, ehdr->e_phoff + i * ehdr->e_phentsize, SEEK_SET);
        if (read(fd, &phdr, sizeof(Elf32_Phdr)) != sizeof(Elf32_Phdr)) {
            perror("read");
            free(ehdr);
            close(fd);
            return;
        }
        // printf("hi");
        if (phdr.p_type == PT_LOAD) {
            void *segment_addr = mmap(
                NULL,
                phdr.p_memsz,
                PROT_READ | PROT_WRITE | PROT_EXEC,
                MAP_ANONYMOUS | MAP_PRIVATE,
                -1,
                0
            );

            if (segment_addr == MAP_FAILED) {
                perror("mmap");
                free(ehdr);
                close(fd);
                return;
            }

            lseek(fd, phdr.p_offset, SEEK_SET);
            if (read(fd, segment_addr, phdr.p_filesz) != phdr.p_filesz) {
                perror("read");
                free(ehdr);
                close(fd);
                return;
            }

            if (entry >= phdr.p_vaddr && entry < (phdr.p_vaddr + phdr.p_memsz)) {
                entry = (entry - phdr.p_vaddr) + (Elf32_Addr)segment_addr;
                int (*_start)() = (int (*)())entry;
                // _start();
                printf("entry: %d", _start());
                // printf("User _start return value = %d\n",8);
                break;
            }
        }
    }
     
  
  
    // loader_cleanup();
    close(fd);
}

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: %s <ELF Executable> \n", argv[0]);
        exit(1);
    }

    load_and_run_elf(argv);
    return 0;
}
