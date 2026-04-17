

// Alexey Yevtushenko
// May 9, 2022

#if !defined(IO_C)
#define IO_C

internal char *read_file_into_memory_and_null_terminate(const char *file_name);
internal String read_file_into_memory(const char *file_name);

internal char *read_file_into_memory_and_null_terminate(const char *file_name){
    char *file_contents = 0;
    FILE *file = fopen(file_name, "rb");
    if(file){
        fseek(file, 0, SEEK_END);
        u32 file_size = ftell(file);
        fseek(file, 0, SEEK_SET);

        file_contents = (char *)malloc(file_size + 1);
        fread(file_contents, file_size, 1, file);
        file_contents[file_size] = 0; // null terminate.

        fclose(file);
    }
    else{
        // TODO: error handling (unable to open the file for read).
    }
    return(file_contents);
}

internal String read_file_into_memory(const char *file_name){
    String file_contents = {0};
    FILE *file = fopen(file_name, "rb");
    if(file){
        fseek(file, 0, SEEK_END);
        file_contents.count = ftell(file);
        fseek(file, 0, SEEK_SET);

        file_contents.data = (char *)malloc(file_contents.count);
        fread(file_contents.data, file_contents.count, 1, file);

        fclose(file);
    }
    else{
        // TODO: error handling (unable to open the file for read).
    }
    return(file_contents);
}

#define read_file(file_name) read_file_into_memory_and_null_terminate(file_name);
#define s_read_file(file_name) read_file_inot_memory(file_name);

#endif // IO_C
