#include "../platform_layer.h"
#ifdef demiwindows

#include <windows.h>
#include <commdlg.h>

void platform_delete_file(DemiFile* file) {
    file->loaded = 0;
    free(file->path);
}

static inline Encoding get_encoding(const wchar_t* file_path) {
    FILE* raw = _wfopen(file_path, L"rb");
    uint8_t BOM[4] = {0};
    fread(BOM, 1, 4, raw);
    fclose(raw);
    if (BOM[0] == 0x00 && BOM[1] == 0x00 && BOM[2] == 0xfe && BOM[3] == 0xff)
        return UTF32BE;
    else if (BOM[0] == 0xff && BOM[1] == 0xfe && BOM[2] == 0x00 && BOM[3] == 0x00)
        return UTF32LE;
    else if (BOM[0] == 0xfe && BOM[1] == 0xff)
        return UTF16BE;
    else if (BOM[0] == 0xff && BOM[1] == 0xfe)
        return UTF16LE;
    else if (BOM[0] == '%' && BOM[1] == 'P' && BOM[2] == 'D' && BOM[3] == 'F')
        return PDF;
    else
        return UTF8;
} 

DemiFile platform_open_file(Buffer* restrict buffer) {
    DemiFile demifile;
    demifile.loaded         = 0;
    demifile.saved_progress = 1;
    wchar_t file_path[MAX_PATH] = L"";

    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFilter = L"All Files\0*.*\0";
    ofn.lpstrFile = file_path;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    ofn.lpstrTitle = L"Select a file to open";

    if (GetOpenFileName(&ofn)) {
        demifile.path     = _wcsdup(file_path);
        demifile.encoding = get_encoding(file_path);
        FILE* file;
        switch(demifile.encoding) {
            case UTF8:
                file = _wfopen(file_path, L"rt+,ccs=UTF-8");
            break;
            case UTF16LE:
                file = _wfopen(file_path, L"rt+,ccs=UTF-16LE");
            break;
            default: // fopen flags for UTF-16BE, UTF-32LE etc. exists, but don't work in msvc.
                return demifile;
        }
        if (!file)
            return demifile;
        
        fseek(file, 0, SEEK_END);
        uint64_t size = ftell(file); // may exceed actual length due to BOM
        rewind(file);

        if (demifile.encoding == UTF16LE || demifile.encoding == UTF16BE)
            size = size >> 1; // size will be precicely double the encoding

        wchar_t* content = malloc((size) * sizeof(wchar_t));
        if (!content) {
            fatal_error(err_memory_allocation);
            fclose(file);
            return demifile;
        }

        size_t length = 0;
        while (fgetws(content + length, size * sizeof(wchar_t), file))
            length = wcslen(content);

        #ifdef demidebug
        info(__LINE__, __FILE__, "file loaded successfully");
        printf("file size: %llu conent length: %llu\n", size, length);
        #endif

        length = platform_validate_string(content);
        buffer_replace_content(buffer, content, length);
        free(content);
        fclose(file);
    }
    demifile.loaded = 1;
    return demifile;
}

extern inline void platform_save_file(DemiFile* restrict file, Buffer* restrict buffer) {
    if (!file->loaded) {
        wchar_t file_path[MAX_PATH] = L"";
        OPENFILENAME ofn;
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.lpstrFilter = L"All Files\0*.*\0";
        ofn.nFilterIndex = 1;
        ofn.lpstrFile = file_path;
        ofn.nMaxFile = MAX_PATH;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
        ofn.lpstrTitle = L"save file as";
        if (GetSaveFileName(&ofn) == TRUE) {
            file->path     = _wcsdup(file_path);
            file->encoding = UTF16LE;
            file->loaded         = 1;
            file->saved_progress = 1;
        }
        else
            return;
    }
    
    FILE *raw = _wfopen(file->path, L"wb");
    if (!raw) {
        error(L"failed to save file", L"warning");
        return;
    }
    wchar_t bom = 0xFEFF; // it will be FF FE in memory
    fwrite(&bom, sizeof(wchar_t), 1, raw);
    fwrite(buffer->content, sizeof(wchar_t), buffer->length, raw);
    fclose(raw);
}

uint64_t platform_validate_string(wchar_t* restrict string) {
    uint64_t i = 0, j = 0;
    while (string[i] != L'\0') {
        if ( string[i] == L'\n' || (iswprint(string[i]) && !iswcntrl(string[i])) )
            string[j++] = string[i];
        i++;
    }
    string[j] = L'\0';
    return j;
}

#endif