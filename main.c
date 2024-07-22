#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <mach/mach.h>
#include <mach/mach_vm.h>

#include <libproc.h>
#include <unistd.h>

#include "common.h"

kern_return_t write_memory(pid_t pid, mach_vm_address_t addr, mach_port_t task) {

    kern_return_t kr;
    uint16_t newopcode = 0x75;

    kr = mach_vm_write(task, addr, (vm_offset_t)&newopcode, sizeof(newopcode));
    if (kr != KERN_SUCCESS) {
        fprintf(stderr, "error writing memory: %s\n", mach_error_string(kr));
    }
    return kr;
}



void scan_memory_for_pattern(uint8_t bytePattern, uint8_t replacebyte, pid_t pid, mach_vm_address_t begin, mach_vm_address_t end) {

    
    mach_port_t task;
    if (task_for_pid(mach_task_self(), pid, &task) != KERN_SUCCESS) {
        fprintf(stderr, "task_for_pid failed\n");
    }

    mach_vm_address_t addr = begin;
    mach_vm_size_t    size = 0;
    kern_return_t kr2;

    while (addr < end) {
        
        mach_vm_size_t region_size;
        vm_region_basic_info_data_64_t info;
        mach_msg_type_number_t info_count = VM_REGION_BASIC_INFO_COUNT_64;
        mach_port_t object_name;

        kr2 = mach_vm_region(task, &addr, &size, VM_REGION_BASIC_INFO_64, (vm_region_info_t)&info, &info_count, &object_name);

        if (kr2 != KERN_SUCCESS) {
            fprintf(stderr, "%s\n", mach_error_string(kr2));
            addr += size;
            continue;
        }

        uint8_t* buffer = malloc(size);
        if (!buffer) {
            fprintf(stderr, "memory allocation failed\n");
            break;
        }

        mach_vm_size_t bytes_read;
        kr2 = mach_vm_read_overwrite(task, addr, size, (mach_vm_address_t)buffer, &bytes_read);
        if (kr2 != KERN_SUCCESS) {
            free(buffer);
            addr += size;
            continue;
        }

        for (size_t i = 0; i < bytes_read; i++) {
            if (buffer[i] == bytePattern) {
                printf("Found 0x%02x at address 0x%llx\n", bytePattern, addr + i);

                mach_vm_address_t new_addr = addr;
                kr2 = mach_vm_write(task, new_addr, (vm_offset_t)&replacebyte, sizeof(replacebyte));

                if (kr2 != KERN_SUCCESS) {
                    fprintf(stderr, "An error occured: %s\n", mach_error_string(kr2));
                }
                else {
                    return;
                }
            }
        }
        free(buffer);
        addr += size;
    }
}

int main() {
    
    const char* processName = "testapp.out";
    printf("process name: %s\n", processName);
    pid_t pid = get_pid_by_process_name(processName);

    uint8_t pattern = 0x74; // JE Opcode (16-32 bit)
    uint8_t replace_bytes = 0x75; // JNE Opcode

    printf("pid: %i\n", pid);
    
    scan_memory_for_pattern(pattern, replace_bytes, pid, 0x10000, 0xFFFFFFFFFFFF);
}