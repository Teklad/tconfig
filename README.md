A basic INI reader/writer built in C, with minimal dependencies.

This was an attempt to write something compact and easy to use for reading
and writing INI files in my projects.

It's able to read most INI formats without any issues.  Comments are delimited by
the ';' character.

A basic usage example would be something like this:
```c
#include <stdio.h>
#include "config.h"

int main()
{
        ini_table_s* config = ini_table_create();
        if (!ini_table_read_from_file(config, "test.ini")) {
                puts("test.ini does not exist! Adding entries!");
                ini_table_create_entry(config, "Section", "one", "two");
                ini_table_write_to_file(config, "test.ini");
        }else {
                printf("Entry one is: %s\n", ini_table_get_entry(config, "Section", "one"));
        }
        ini_table_destroy(config);
        return 0;
}
```

You don't need to check for NULL on ini_table_create(), since it creates an empty
structure.  Always make sure to call ini_table_destroy() when you're finished
using the data, however.

There's currently no support for saving comment lines, but I may bother to add it in the future if I get bored.

For more information on these functions, check out config.h, which I spent a bit of time documenting for easy reading.

