#include <stdio.h>
#include "config.h"

int main()
{
    ini_table_s* config = ini_table_create();
    if (!ini_table_read_from_file(config, "test.ini")) {
        puts("test.ini does not exist! Adding entries!");
        ini_table_create_entry(config, "Section", "one", "two");
        ini_table_create_entry(config, "Section", "; comment", "");
        ini_table_create_entry(config, "Section", "; comment 2", "");
        ini_table_write_to_file(config, "test.ini");
    }else {
        printf("Entry one is: %s\n", ini_table_get_entry(config, "Section", "one"));
        puts("creating copy to second.ini with comments");
        ini_table_create_entry(config, "Section", "; comment manually added", "");
        ini_table_write_to_file(config, "second.ini");
        
    }
    ini_table_destroy(config);
    return 0;
}
