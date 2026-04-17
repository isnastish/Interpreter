

// Alexey Yevtushenko.
// May 9, 2022

// TODO:
// 
// [] Implement a hash-table.
// [] Rewrite string interning using hash-table.

static String string_intern_range(String **intern_table, char *range_start, char *range_end){
    u32 length = (range_end - range_start);
    for(s32 i = 0; (i < darr_len(*intern_table)); i += 1){
        if(!strncmp((*intern_table+i)->data, range_start, length) &&
           ((*intern_table+i)->count == length))
            return(*(*intern_table+i));
    }
    String result = {0};
    result.data = (char *)malloc(length);
    result.count = length;
    memcpy(result.data, range_start, length);
    darr_push(*intern_table, result);
    return(result);
}

static String string_intern(String **intern_table, char *string){
    String result = {0};
    result = string_intern_range(intern_table, string, (string + string_len(string)));
    return(result);
}
 
