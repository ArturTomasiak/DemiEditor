#include "platform_layer.h"
#ifdef demiwindows

#include <windows.h>
#include <commdlg.h>
// TODO: detect encoding type and read accordingly
void open_file(Buffer* restrict buffer) {
    wchar_t fileName[MAX_PATH] = L"";

    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFilter = L"All Files\0*.*\0";
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    ofn.lpstrTitle = L"Select a file to open";

    if (GetOpenFileName(&ofn)) {
        FILE* file = _wfopen(fileName, L"rt,ccs=unicode");
        if (!file) {
            return;
        }
        
        fseek(file, 0, SEEK_END);
        uint64_t len = ftell(file);
        rewind(file);

        wchar_t* content = malloc((len + 1) * sizeof(wchar_t));
        if (!content) {
            error(err_memory_allocation);
            fclose(file);
            return;
        }

        size_t length = 0;
        while (fgetws(content + length, len * sizeof(wchar_t), file))
            length = wcslen(content);

        fclose(file);

        printf("%llu %llu \n", len, length);

        buffer_replace_content(buffer, content, length);
        free(content);
    }
}   

#endif