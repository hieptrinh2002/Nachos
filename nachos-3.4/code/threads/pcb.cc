// pcb.cc, "Process Control Block"
// All rights reserved.

#include "pcb.h"
#include "utility.h"
#include "system.h"
#include "thread.h"
#include "addrspace.h"

PCB::PCB(int id)
{
    // Tien trinh dau tien la tien trinh cha cua 1, 2, 3, ...
    if (id == 0)
        this->parentID = -1;
    else
        this->parentID = currentThread->processID;

    this->numwait = this->exitcode = 0;
    this->thread = NULL;

    // Tien trinh doc quyen truy xuat thi de la 1
    this->joinsem = new Semaphore("joinsem", 0);
    this->exitsem = new Semaphore("exitsem", 0);
    this->mutex = new Semaphore("mutex", 1);
}

PCB::~PCB()
{

    if (joinsem != NULL)
        delete this->joinsem;
    if (exitsem != NULL)
        delete this->exitsem;
    if (mutex != NULL)
        delete this->mutex;
    if (thread != NULL)
    {
        thread->Finish();
    }
}

int PCB::GetNumWait() { return this->numwait; }

// Process tranlation to block
// Wait for JoinRelease to continue exec
void PCB::JoinWait()
{
    // 1. Tiến trình cha đợi tiến trình con kết thúc
    // Gọi joinsem->P() để tiến trình chuyển sang trạng thái block và ngừng lại, chờ JoinRelease để thực hiện tiếp.
    joinsem->P();
}

// JoinRelease process calling JoinWait
void PCB::JoinRelease()
{
    // 2. Báo cho tiến trình cha thực thi tiếp
    // Gọi joinsem->V() để giải phóng tiến trình gọi JoinWait().
    joinsem->V();
}

// Let process tranlation to block state
// Waiting for ExitRelease to continue exec
void PCB::ExitWait()
{
    // 3. Cho phép tiến trình con kết thúc
    //  Gọi exitsem->P() để tiến trình chuyển sang trạng thái block và ngừng lại, chờ ExitRelease để thực hiện tiếp.
    exitsem->P();
}

// Release wating process
void PCB::ExitRelease()
{
    // 4. Tiến trình con kết thúc
    // Gọi exitsem-->V() để giải phóng tiến trình đang chờ.
    exitsem->V();
}

void PCB::IncNumWait()
{
    mutex->P(); // để giúp tránh tình trạng nạp 2 tiến trình cùng 1 lúc.
    ++numwait;
    mutex->V(); // Nha CPU de nhuong CPU cho tien trinh khac
}

void PCB::DecNumWait()
{
    mutex->P(); // để giúp tránh tình trạng nạp 2 tiến trình cùng 1 lúc.
    if (numwait > 0)
        --numwait;
    mutex->V(); // Nha CPU de nhuong CPU cho tien trinh khac
}

int PCB::Exec(char *filename, int id)
{
    // Gọi mutex->P(); để giúp tránh tình trạng nạp 2 tiến trình cùng 1 lúc.
    mutex->P();

    // Kiểm tra thread đã khởi tạo thành công chưa, nếu chưa thì báo lỗi là không đủ bộ nhớ, gọi mutex->V() và return.
    this->thread = new Thread(filename); // (./threads/thread.h)

    if (this->thread == NULL)
    {
        printf("\nPCB::Exec: Not enough memory!\n");
        mutex->V(); // Nha CPU de nhuong CPU cho tien trinh khac
        return -1;  // Tra ve -1 neu that bai
    }

    //  Đặt processID của thread này là id.
    this->thread->processID = id;
    // Đặt parrentID của thread này là processID của thread gọi thực thi Exec
    this->parentID = currentThread->processID;
    // Gọi thực thi Fork(StartProcess_2,id) => Ta cast thread thành kiểu int, sau đó khi xử ký hàm StartProcess ta cast Thread về đúng kiểu của nó.
    // this->thread->Fork(StartProcess_2, id);

    mutex->V();
    // Trả về id.
    return id;
}