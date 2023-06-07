#include "syscall.h"
#include "copyright.h"

void close_input_output(SpaceId id_input, SpaceId id_output) {
    Close(id_input);
    Close(id_output);
}

int main()
{
    SpaceId id_input, id_output, id_sinhvien, id_voinuoc;
    int num_time;
    char temp_char; // temp char for read file

    if (CreateSemaphore("main", 0) == -1 
    || CreateSemaphore("sinhvien", 0) == -1 
    || CreateSemaphore("voinuoc", 0) == -1 
    || CreateSemaphore("main_vn", 0) == -1
    || CreateFile("output.txt") == -1)
    {
        return 1;
    }

    id_input = Open("input.txt", 1); // only read
    id_output = Open("output.txt"); //write 
    if (id_input == -1 || id_output == -1) {
        return 1;
    }

    num_time = 0;
    while (Read(&temp_char, 1, id_input) >= 1) {
        if (temp_char == '\n') {
            break;
        }

        if (temp_char >= '0' && temp_char <= '9') {
            num_time = num_time * 10 + (temp_char - 48);
        }
    }

    if (Exec("./test/sinhvien") == -1 || Exec("./test/voinuoc") == -1)
    {
        close_input_output(id_input, id_output);
        return 1;
    }

    while (num_time --) {
        //Để createfile & openfile trong loop để reset mỗi vòng lặp
        if (CreateFile("sinhvien.txt") == -1)
        {
            close_input_output(id_input, id_output);
            return 1;
        }

        id_sinhvien = Open("sinhvien.txt", 0);
        if (id_sinhvien == -1)
        {
            close_input_output(id_input, id_output);
            return 1;
        }

        while (Read(&temp_char, 1, id_input) >= 1) {
            if (temp_char == '\n') {
                break;
            }
            Write(&temp_char, 1, id_sinhvien);
        }

        Close(id_sinhvien);

        Signal("sinhvien");

        Wait("main");

        id_voinuoc = Open("voinuoc.txt", 1);
        if (id_voinuoc == -1) {
            close_input_output(id_input, id_output);
            return 1;
        }

        PrintString("\n Lan thu: ");
        PrintInt(num_time);
        PrintString("\n");


        //Mo lai file sinh vien de doc
        id_sinhvien = Open("sinhvien.txt", 1);
        if (id_sinhvien == -1)
        {
            close_input_output(id_input, id_output);
            return 1;
        }

        while (Read(&temp_char, 1, id_sinhvien) >= 1) {
            Write(&temp_char, 1, id_output);
            Write(" ", 1, id_output);
            if (temp_char == ' ')
            {
                while (Read(&temp_char, 1, id_voinuoc) >= 1) {
                    if (temp_char != '') {
                        Write(&temp_char, 1, id_output);
                        Write("     ", 5, id_output);
                    }
                }
            }
        }

        Write("\r\n", 2, id_output);
        Close(id_voinuoc);
        Signal("main_vn");

        Close(id_sinhvien);
    }

    close_input_output(id_input, id_output);
    return 0;
}