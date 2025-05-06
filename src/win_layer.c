#include "win_layer.h"
#include <windows.h>
#include <commdlg.h>

void open_file(Buffer* restrict buffer) {
    char fileName[MAX_PATH] = "";

    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFilter = "All Files\0*.*\0";
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    ofn.lpstrTitle = "Select a file to open";

    if (GetOpenFileName(&ofn)) {
        FILE* file = fopen(fileName, "rb");
        if (!file) {
            perror("Error opening file");
            return;
        }

        fseek(file, 0, SEEK_END);
        long size = ftell(file);
        rewind(file);

        char* content = malloc(size + 1);
        if (!content) {
            perror("Memory allocation failed");
            fclose(file);
            return;
        }

        fread(content, 1, size, file);
        content[size] = '\0';
        fclose(file);

        printf("File contents:\n%s\n", content);

        buffer_replace_content(buffer, content, strlen(content));

        free(content);
    } else {
        printf("No file selected or dialog cancelled.\n");
    }
}   