#ifndef PTABLE_H
#define PTABLE_H
#include "bitmap.h"
#include "pcb.h"

#define MAX_PROCESS 10
class PTable
{
private:
    BitMap *bm;
    PCB *pcb[MAX_PROCESS];
    int psize;
    Semaphore *bmsem; // dùng để ngăn chặn trường hợp nạp 2 tiến trình cùng lúc
public:
    PTable(int size);
    ~PTable();
    // Khởi tạo  size đối tượng pcb để lưu size process. Gán giá trị ban đầu là null. Nhớ khởi tạo *bm và *bmsem để sử dụng

    int ExecUpdate(char *); // Xử lý cho system call SC_Exec
    int ExitUpdate(int);    // Xử lý cho system call SC_Exit
    int JoinUpdate(int);    // Xử lý cho system call SC_Join

    int GetFreeSlot();     // tìm free slot để lưu thông tin cho tiến trình mới
    bool IsExist(int pid); // kiểm tra tồn tại processID này không?

    void Remove(int pid); // khi tiến trình kết thúc, delete processID ra khỏi mảng quản lý nó

    char *GetFileName(int id); // Trả về tên của tiến trình
};
#endif // PTABLE_H