#ifndef PCB_H
#define PCB_H
#include "thread.h"
#include "synch.h"
class PCB
{
private:
    Semaphore *joinsem; // semaphore cho quá trình join
    Semaphore *exitsem; // semaphore cho quá trình exit
    Semaphore *mutex;
    int exitcode;
    Thread *thread;
    int pid;
    int numwait; // số tiến trình đã join

public:
    int parentID; // ID của tiến trình cha
    PCB(int id);
    ~PCB();
    int Exec(char *filename, int pid); // nạp chương trình có tên lưu trong biến filename và //processID sẽ là pid
    int GetID() { return pid; }
    int GetNumWait();
    void JoinWait();
    void ExitWait();
    void JoinRelease();
    void ExitRelease();
    void IncNumWait();
    void DecNumWait();
    void SetExitCode(int ec) { exitcode = ec; }
    int GetExitCode() { return exitcode; }
};
#endif
