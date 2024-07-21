#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <mach/mach.h>
#include <mach/mach_vm.h>

#include <libproc.h>
#include <unistd.h>
#include <errno.h>

kern_return_t write_memory(pid_t pid, mach_vm_address_t addr, void* data, size_t size) {

    kern_return_t kr;
    mach_port_t task;

    kr = task_for_pid(mach_task_self(), pid, &task);
    if (kr != KERN_SUCCESS) {
        fprintf(stderr, "error getting task from pid %d: %s\n", pid, mach_error_string(kr));
        return kr;
    }

    kr = mach_vm_write(task, addr, (vm_offset_t)data, (mach_msg_type_number_t)size);
    if (kr != KERN_SUCCESS) {
        fprintf(stderr, "error writing memory: %s\n", mach_error_string(kr));
    }
    return kr;
}

pid_t string_to_pid(const char *name) {
    char *endptr;
    long pid_long;

    errno = 0;
    pid_long = strtol(name, &endptr, 10);

    if (errno == ERANGE && (pid_long == LONG_MAX || pid_long == LONG_MIN) || (errno != 0 && pid_long == 0)) {
        perror("strtol");
        exit(EXIT_FAILURE);
    }

    if (endptr == name) {
        fprintf(stderr, "No digits were found]n");
        exit(EXIT_FAILURE);
    }

    if (pid_long <= 0 || pid_long > INT_MAX) {
        fprintf(stderr, "PID out of range\n");
        exit(EXIT_FAILURE);
    }

    return (pid_t)pid_long;
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <process_name> <address> <new_memory> <pid>\n", argv[0]);
        return 1;
    }

    const char* processName = argv[1];
    mach_vm_address_t addr = strtoull(argv[2], NULL, 16);
    int new_memory = atoi(argv[3]);

    pid_t pid = string_to_pid(argv[4]);

    kern_return_t kr = write_memory(pid, addr, &new_memory, sizeof(new_memory));
}