#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <mach/mach.h>
#include <mach/mach_vm.h>

#include <libproc.h>
#include <unistd.h>

void scan_memory(uint8_t bytePattern[], size_t patternSize, pid_t pid) {

    mach_port_t task;
    kern_return_t kr = task_for_pid(mach_task_self(), pid, &task);
    if (kr != KERN_SUCCESS) {
        fprintf(stderr, "error getting task from pid %d: %s\n", pid, mach_error_string(kr));
        return;
    }

    mach_vm_address_t addr = 0;
    mach_vm_size_t size = 0;
    kern_return_t kr2;

    while (1) {
        mach_vm_size_t region_size;
        vm_region_basic_info_data_64_t info;
        mach_msg_type_number_t info_count = VM_REGION_BASIC_INFO_COUNT_64;
        mach_port_t object_name;

        kr2 = mach_vm_region(task, &addr, &size, VM_REGION_BASIC_INFO_64, (vm_region_info_t)&info, &info_count, &object_name);
        
        uint8_t* buf = malloc(size);
        if (!buf) {
            perror("Memory allocation error");
            break;
        }

        mach_vm_size_t bytes_read;
        kr2 = mach_vm_read_overwrite(task, addr, size, (mach_vm_address_t)buf, &bytes_read);

        if (kr2 != KERN_SUCCESS) {
            free(buf);
            addr += size;
            continue;
        }

        for (size_t i = 0; i < bytes_read; i++) {
            if (i + patternSize > bytes_read) break;
            if (memcmp(buf + i, bytePattern, patternSize) == 0) {
                printf("Pattern found at address: %p at pid %d\n", (void*)(addr + 1), pid);
            }
        }

        free(buf);
        addr += size;
    }
}

pid_t get_pid_by_process_name(const char *name) {
    int num_pids;
    pid_t* pid;
    char proc_name[PROC_PIDPATHINFO_MAXSIZE];

    num_pids = proc_listpids(PROC_ALL_PIDS, 0, NULL, 0);
    pid = malloc(num_pids * sizeof(pid_t));

    num_pids = proc_listpids(PROC_ALL_PIDS, 0, pid, num_pids * sizeof(pid_t));

    for (int i = 0; i < num_pids; i++) {
        if (pid[i] == 0) continue;

        if (proc_pidpath(pid[i], proc_name, sizeof(proc_name)) > 0 && strstr(proc_name, name) != NULL) {
            free(pid);
            return pid[0];
        }
    }

    free(pid);
    return -1;
}

int main() {
    const char* processName = "testapp.out";
    printf("process name: %s", processName);
    pid_t pid = get_pid_by_process_name(processName);

    uint8_t pattern[] = {0x0F, 0x84}; // JE Opcode (16-32 bit)
    scan_memory(pattern, sizeof(pattern), pid);
}