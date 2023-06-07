#include "stable.h"

STable::STable()
{
    // init bm
    bm = new BitMap(MAX_SEMAPHORE);
    // init semTab, initial value is NULL
    for (int i = 0; i < MAX_SEMAPHORE; i++)
    {
        semTab[i] = NULL;
    }
}

STable::~STable()
{
    // delete bitmap
    if (bm)
    {
        delete bm;
        bm = NULL;
    }

    // delete semTab
    for (int i = 0; i < MAX_SEMAPHORE; ++i)
    {
        if (semTab[i])
        {
            delete semTab[i];
            semTab[i] = NULL;
        }
    }
}

int STable::Create(char *name, int init)
{
    // check if lock "name" exists in bitmap
    for (int i = 0; i < MAX_SEMAPHORE; ++i)
    {
        if (bm->Test(i))
        {
            if (strcmp(name, semTab[i]->GetName()) == 0)
            {
                printf("Lock %s already exist!", name);
                return -1;
            }
        }
    }

    // find slot of semTab
    int id = this->bm->Find();

    // if full
    if (id < 0)
        return -1;

    // else
    this->semTab[id] = new Sem(name, init);
    return 0;
}

int STable::Wait(char *name)
{
    // check if lock "name" exists in bitmap, if yes call Lock->Acquire()
    for (int i = 0; i < MAX_SEMAPHORE; i++)
    {
        if (bm->Test(i))
        {
            if (strcmp(name, semTab[i]->GetName()) == 0)
            {
                semTab[i]->wait();
                return 0;
            }
        }
    }

    printf("This %s semaphore does not exist!", name);
    return -1;
}

// Up(sem)
int STable::Signal(char *name)
{
    // check if lock "name" exists in bitmap, if yes call Lock->Release()
    for (int i = 0; i < MAX_SEMAPHORE; i++)
    {
        if (bm->Test(i))
        { // Kiem tra o thu i co duoc nap vao BitMap chua
            if (strcmp(name, semTab[i]->GetName()) == 0)
            { // Neu semaphore da ton tai trong semTab
                // Up(sem)
                semTab[i]->signal();
                return 0;
            }
        }
    }

    printf("This %s semaphore does not exist!", name);
    return -1;
}

int STable::FindFreeSlot(int id)
{
    return bm->Find();
}