#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <mach/mach.h>
#include <mach/mach_vm.h>

#include <libproc.h>
#include <unistd.h>
#include "common.h"

kern_return_t write_memory(mach_vm_address_t addr, mach_port_t task) {

    kern_return_t kr;

    uint8_t newopcode = 0x75;

    kr = mach_vm_write(task, addr, (vm_offset_t)&newopcode, sizeof(newopcode));
    if (kr != KERN_SUCCESS) {
        fprintf(stderr, "  ⤷ error writing memory: %s\n\n", mach_error_string(kr));
    }
    return kr;
}


void print_memory_regions(pid_t pid) {

    mach_port_t                     task;
    mach_vm_size_t                  size;
    mach_vm_address_t               address = 0;

    kern_return_t                   kr;

    vm_region_basic_info_data_64_t  info;
    mach_msg_type_number_t          info_count;

    memory_object_name_t            object;

    char protection[4];

    kr = task_for_pid(mach_task_self(), pid, &task);
    if (kr != KERN_SUCCESS) {
        fprintf(stderr, "task_for_pid: %s\n", mach_error_string(kr));
        exit(EXIT_FAILURE);
    }

    while (1) {
        info_count = VM_REGION_BASIC_INFO_COUNT_64;
        kr = mach_vm_region(task, &address, &size, VM_REGION_BASIC_INFO, (vm_region_info_t)&info, &info_count, &object);
        if (kr != KERN_SUCCESS) {
            break;
        }

        protection[0] = (info.protection & VM_PROT_READ) ? 'r' : '-';
        protection[1] = (info.protection & VM_PROT_WRITE) ? 'w' : '-';
        protection[2] = (info.protection & VM_PROT_EXECUTE) ? 'x' : '-';
        protection[3] = '\0';

        printf("Region: 0x%08llx - 0x%llx | Size: %lld | Protection: %s\n",
               address, address + size, size, protection);

        uint8_t* buffer = malloc(size);
        if (!buffer) {
            fprintf(stderr, "memory allocation failed\n");
            break;
        }

        mach_vm_size_t bytes_read;
        kr = mach_vm_read_overwrite(task, address, size, (mach_vm_address_t)buffer, &bytes_read);
        if (kr != KERN_SUCCESS) {
            free(buffer);
            address += size;
            continue;
        }

        // Finding the addresses with the 0x84 byte
        for (size_t i = 0; i < bytes_read; i++) {
            if (buffer[i] == 0x84) {
                printf("⤷ Found 0x%02x at address 0x%llx\n", 0x84, address + i);

                // Writing memory
                write_memory(address, task);
                break;
            }
        }

        // Read the first 256 bytes of the memory address
        int bytes = 256;
        for (size_t i = 0; i < bytes; i++) { printf("0x%X ", buffer[i]); }
        printf("\n");
        free(buffer);

        address += size;
    }
}


int main(int argc, char *argv[]) {
    
    
    pid_t pid = get_pid_by_process_name("testapp.out");
    print_memory_regions(pid);

    return EXIT_SUCCESS;
}