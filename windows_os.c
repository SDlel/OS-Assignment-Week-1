#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FILES 50
#define MAX_PROCS 20
#define TOTAL_MEM 1024
#define QUANTUM 2

typedef struct {
    char name[32];
    char content[256];
    int isDir;
    int inUse;
} VFile;

typedef struct {
    int pid;
    char name[32];
    int burst;
    int remaining;
    int mem;
    int active;
} PCB;

VFile fs[MAX_FILES];
PCB procs[MAX_PROCS];
int nextPid = 3;
int fileCount = 0;

void readLine(char *buf, int size) {
    if (fgets(buf, size, stdin) == NULL) { buf[0] = '\0'; return; }
    buf[strcspn(buf, "\r\n")] = '\0';
}

void pauseEnter() {
    printf("\nPress Enter to continue...");
    getchar();
}

void fsAdd(const char *name, int isDir, const char *content) {
    int i;
    for (i = 0; i < MAX_FILES; i++) {
        if (!fs[i].inUse) {
            strncpy(fs[i].name, name, 31); fs[i].name[31] = '\0';
            fs[i].isDir = isDir; fs[i].inUse = 1;
            if (content) strncpy(fs[i].content, content, 255);
            else fs[i].content[0] = '\0';
            fileCount++;
            return;
        }
    }
}

VFile* fsFind(const char *name) {
    int i;
    for (i = 0; i < MAX_FILES; i++)
        if (fs[i].inUse && strcmp(fs[i].name, name) == 0) return &fs[i];
    return NULL;
}

void procAdd(const char *name, int burst, int mem) {
    int i;
    for (i = 0; i < MAX_FILES; i++) {
        if (i < MAX_PROCS && !procs[i].active) {
            procs[i].pid = nextPid++;
            strncpy(procs[i].name, name, 31); procs[i].name[31] = '\0';
            procs[i].burst = burst;
            procs[i].remaining = burst;
            procs[i].mem = mem;
            procs[i].active = 1;
            return;
        }
    }
    printf("Process table full.\n");
}

int usedMemory() {
    int s = 0, i;
    for (i = 0; i < MAX_PROCS; i++)
        if (procs[i].active) s += procs[i].mem;
    return s;
}

void kernelInit() {
    int i;
    for (i = 0; i < MAX_FILES; i++) fs[i].inUse = 0;
    for (i = 0; i < MAX_PROCS; i++) procs[i].active = 0;
    fileCount = 0;
    fsAdd("C:", 1, NULL);
    fsAdd("Windows", 1, NULL);
    procAdd("explorer.exe", 6, 128); procs[0].pid = 1;
    procAdd("svchost.exe", 8, 64);  procs[1].pid = 2;
}

void myComputer() {
    char cmd[64], arg[32];
    while (1) {
        int i;
        printf("\n--- My Computer (C:) ---\n");
        for (i = 0; i < MAX_FILES; i++)
            if (fs[i].inUse)
                printf("%s %s\n", fs[i].isDir ? "<DIR>" : "     ", fs[i].name);
        printf("\n1. mkdir  2. del  3. type(view)  0. back\nChoice: ");
        readLine(cmd, sizeof(cmd));
        if (strcmp(cmd, "0") == 0) return;
        if (strcmp(cmd, "1") == 0) {
            printf("Folder/file name: "); readLine(arg, sizeof(arg));
            if (fsFind(arg)) printf("Already exists.\n");
            else { fsAdd(arg, strchr(arg, '.') ? 0 : 1, ""); printf("Created.\n"); }
        } else if (strcmp(cmd, "2") == 0) {
            VFile *f;
            printf("Name to delete: "); readLine(arg, sizeof(arg));
            f = fsFind(arg);
            if (!f) printf("Not found.\n");
            else { f->inUse = 0; fileCount--; printf("Deleted.\n"); }
        } else if (strcmp(cmd, "3") == 0) {
            VFile *f;
            printf("File name: "); readLine(arg, sizeof(arg));
            f = fsFind(arg);
            if (!f) printf("Not found.\n");
            else if (f->isDir) printf("It is a folder.\n");
            else printf("Content: %s\n", f->content);
        }
        pauseEnter();
    }
}

void notepad() {
    char fname[32], line[256];
    VFile *f;
    printf("\n--- Notepad ---\nFile name: ");
    readLine(fname, sizeof(fname));
    if (strlen(fname) == 0) return;
    f = fsFind(fname);
    if (!f) { fsAdd(fname, 0, ""); f = fsFind(fname); printf("(new file)\n"); }
    else if (f->isDir) { printf("That is a folder.\n"); pauseEnter(); return; }
    else printf("Current: %s\n", f->content);
    printf("Type content (empty = keep):\n> ");
    readLine(line, sizeof(line));
    if (strlen(line) > 0) { strncpy(f->content, line, 255); printf("Saved.\n"); }
    else printf("No changes.\n");
    pauseEnter();
}

void calculator() {
    char expr[64];
    double a, b, r;
    char op;
    printf("\n--- Calculator ---\nEnter like 12 + 5 (0 to go back)\n");
    while (1) {
        printf("calc> ");
        readLine(expr, sizeof(expr));
        if (strcmp(expr, "0") == 0) return;
        if (sscanf(expr, "%lf %c %lf", &a, &op, &b) == 3) {
            if (op == '+') r = a + b;
            else if (op == '-') r = a - b;
            else if (op == '*') r = a * b;
            else if (op == '/') {
                if (b == 0) { printf("Divide by zero.\n"); continue; }
                r = a / b;
            } else { printf("Use + - * /\n"); continue; }
            printf("= %.2f\n", r);
        } else printf("Use e.g. 9 * 3\n");
    }
}

void taskManager() {
    char cmd[64], arg[32];
    int i, pid, found;
    while (1) {
        printf("\n--- Task Manager ---\nPID  NAME           BURST LEFT MEM\n");
        for (i = 0; i < MAX_PROCS; i++)
            if (procs[i].active)
                printf("%-4d %-14s %-5d %-4d %dKB\n", procs[i].pid, procs[i].name,
                       procs[i].burst, procs[i].remaining, procs[i].mem);
        printf("\n1. run  2. kill  0. back\nChoice: ");
        readLine(cmd, sizeof(cmd));
        if (strcmp(cmd, "0") == 0) return;
        if (strcmp(cmd, "1") == 0) {
            printf("Process name: "); readLine(arg, sizeof(arg));
            if (usedMemory() + 64 > TOTAL_MEM) printf("Out of memory.\n");
            else { procAdd(arg, 6, 64); printf("Started.\n"); }
            pauseEnter();
        } else if (strcmp(cmd, "2") == 0) {
            printf("PID to kill: "); readLine(arg, sizeof(arg));
            pid = atoi(arg); found = 0;
            for (i = 0; i < MAX_PROCS; i++)
                if (procs[i].active && procs[i].pid == pid) {
                    procs[i].active = 0; found = 1;
                    printf("Killed %d, freed %dKB.\n", pid, procs[i].mem);
                    break;
                }
            if (!found) printf("No such PID.\n");
            pauseEnter();
        }
    }
}

void memInfo() {
    int used = usedMemory();
    int i;
    printf("\n--- Memory Info ---\nTotal: %dKB Used: %dKB Free: %dKB\n",
           TOTAL_MEM, used, TOTAL_MEM - used);
    for (i = 0; i < MAX_PROCS; i++)
        if (procs[i].active)
            printf("PID %d %-14s %dKB\n", procs[i].pid, procs[i].name, procs[i].mem);
    printf("(run = allocate, kill = free)\n");
    pauseEnter();
}

void scheduler() {
    int i, t = 0, done = 0;
    for (i = 0; i < MAX_PROCS; i++)
        if (procs[i].active) procs[i].remaining = procs[i].burst;
    printf("\n--- CPU Scheduler (Round Robin, q=2) ---\nGantt: ");
    while (!done) {
        done = 1;
        for (i = 0; i < MAX_PROCS; i++) {
            int run;
            if (!procs[i].active || procs[i].remaining <= 0) continue;
            done = 0;
            run = procs[i].remaining < QUANTUM ? procs[i].remaining : QUANTUM;
            printf("[P%d:%d] ", procs[i].pid, run);
            procs[i].remaining -= run;
            t += run;
        }
    }
    printf("\nTotal time: %d units.\n", t);
    for (i = 0; i < MAX_PROCS; i++)
        if (procs[i].active && procs[i].pid > 2) procs[i].active = 0;
    for (i = 0; i < MAX_PROCS; i++)
        if (procs[i].active) procs[i].remaining = procs[i].burst;
    pauseEnter();
}

int main() {
    char ch[16], tmp[16];
    kernelInit();
    printf("Mini-Windows 1.0 - OS Model (Windows concepts)\n");
    printf("Booting... kernel loaded. File system mounted.\n");
    printf("Press Enter to login as Student...");
    readLine(tmp, sizeof(tmp));
    printf("Welcome!\n");
    while (1) {
        printf("\n====== Main Menu ======\n");
        printf("1. My Computer\n2. Notepad\n3. Calculator\n");
        printf("4. Task Manager\n5. Memory Info\n6. Scheduler Demo\n0. Shutdown\n");
        printf("Enter your choice: ");
        readLine(ch, sizeof(ch));
        if (strcmp(ch, "1") == 0) myComputer();
        else if (strcmp(ch, "2") == 0) notepad();
        else if (strcmp(ch, "3") == 0) calculator();
        else if (strcmp(ch, "4") == 0) taskManager();
        else if (strcmp(ch, "5") == 0) memInfo();
        else if (strcmp(ch, "6") == 0) scheduler();
        else if (strcmp(ch, "0") == 0) { printf("Shutting down...\n"); break; }
        else printf("Invalid choice (0-6).\n");
    }
    return 0;
}
