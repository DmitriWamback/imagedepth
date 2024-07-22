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