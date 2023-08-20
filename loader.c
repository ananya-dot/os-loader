#include "loader.h"

Elf32_Ehdr *ehdr;
int fd;

/*
 * Release memory and other cleanups
 */
void loader_cleanup() {
    free(ehdr);
}

/*
 * Load and run the ELF executable file
 */
void load_and_run_elf(char** exe) {
    fd = open(*exe, O_RDONLY);
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

    // Read ELF header
    if (read(fd, ehdr, sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr)) {
        perror("read");
        free(ehdr);  // Free allocated memory on error
        close(fd);
        return;
    }

    // Find the loadable segment containing the entry point
    Elf32_Addr adjusted_entry = ehdr->e_entry;  // Initialize with the default entry point
    for (int i = 0; i < ehdr->e_phnum; ++i) {
        Elf32_Phdr phdr;
        lseek(fd, ehdr->e_phoff + i * ehdr->e_phentsize, SEEK_SET);
        if (read(fd, &phdr, sizeof(Elf32_Phdr)) != sizeof(Elf32_Phdr)) {
            perror("read");
            free(ehdr);  // Free allocated memory on error
            close(fd);
            return;
        }

        if (phdr.p_type == PT_LOAD) {
            if (ehdr->e_entry >= phdr.p_vaddr && ehdr->e_entry < phdr.p_vaddr + phdr.p_memsz) {
                // Entry point is within this segment
                adjusted_entry = ehdr->e_entry - phdr.p_vaddr + phdr.p_offset;
                break;
            }
        }
    }

    // Load the segment and adjust the entry point
    for (int i = 0; i < ehdr->e_phnum; ++i) {
        Elf32_Phdr phdr;
        lseek(fd, ehdr->e_phoff + i * ehdr->e_phentsize, SEEK_SET);
        if (read(fd, &phdr, sizeof(Elf32_Phdr)) != sizeof(Elf32_Phdr)) {
            perror("read");
            free(ehdr);  // Free allocated memory on error
            close(fd);
            return;
        }

        // if (phdr.p_type == PT_LOAD) {
            // void *segment_addr = mmap(
            //     NULL,
            //     phdr.p_memsz,
            //     PROT_READ|PROT_WRITE|PROT_EXEC,
            //     MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED,
            //     0,
            //     0
            // );
            if (phdr.p_type == PT_LOAD) {
    void *segment_addr = mmap(
        (void *)phdr.p_vaddr,  // Use p_vaddr as the desired virtual address
        phdr.p_memsz,
        PROT_READ | PROT_WRITE | PROT_EXEC,
        MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED,
        -1,
        0
    );

    // if (segment_addr == MAP_FAILED) {
    //     perror("mmap");
    //     free(ehdr);  // Free allocated memory on error
    //     close(fd);
    //     return;
    // }

    lseek(fd, phdr.p_offset, SEEK_SET);
    if (read(fd, segment_addr, phdr.p_filesz) != phdr.p_filesz) {
        perror("read");
        free(ehdr);  // Free allocated memory on error
        close(fd);
        return;
    }
}

            
            // if (segment_addr == MAP_FAILED) {
            //     perror("mmap");
            //     free(ehdr);  // Free allocated memory on error
            //     close(fd);
            //     return;
            // }

            // lseek(fd, phdr.p_offset, SEEK_SET);
            // if (read(fd, segment_addr, phdr.p_filesz) != phdr.p_filesz) {
            //     perror("read");
            //     free(ehdr);  // Free allocated memory on error
            //     close(fd);
            //     return;
            }
        
    

    // Start execution at adjusted entry point


    // Clean up: Free allocated memory and close file descriptor
    loader_cleanup();
    close(fd);
}

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: %s <ELF Executable> \n", argv[0]);
        exit(1);
    }

    // 1. Carry out necessary checks on the input ELF file (not shown)
    // 2. Passing it to the loader for carrying out the loading/execution
    load_and_run_elf(argv);
    return 0;
}
