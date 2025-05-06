#include "platform_layer.h"
#ifdef demiwindows

#include <windows.h>
#include <commdlg.h>
// This doesn't actually work, first step is to check the file's encoding
void open_file(Buffer* restrict buffer) {
    char fileName[MAX_PATH] = "";

    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFilter = "All Files\0*.*\0";
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    ofn.lpstrTitle = "Select a file to open";

    if (GetOpenFileNameA(&ofn)) {
        FILE* file = fopen(fileName, "rb");
        if (!file) {
            return;
        }

        fseek(file, 0, SEEK_END);
        uint64_t size = ftell(file);
        rewind(file);

        char* content = malloc(size + 1);
        if (!content) {
            error(err_memory_allocation);
            fclose(file);
            return;
        }

        fread(content, 1, size, file);
        content[size] = '\0';
        fclose(file);

        buffer_replace_content(buffer, content, size + 1);
        free(content);
    }
}   

#endif