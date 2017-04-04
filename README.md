A basic INI reader/writer built in C, with minimal dependencies.

This was an attempt to write something compact and easy to use for reading
and writing INI files in my projects.

It's able to read most INI formats without any issues.  Comments are delimited by
the ';' character.

A basic usage example would be something like this:
```c
#include "config.h"
int main(void)
{
    ini_table_s* conf = ini_read("test.ini");
    if (conf == NULL) {
        return -1;
    }
    const char* tasty = ini_entry_get_value(conf, "section", "key");
    printf("%s == winner\n", tasty);
    ini_table_destroy(conf);
    return 0;
}
```

If you want to create/modify an entry:
```c
#include "config.h"
int main(void)
{
    ini_table_s* conf = ini_read("test.ini");
    if (conf == NULL) {
        return -1;
    }
    ini_entry_create(conf, "Section", "Lemons", "are great");
    ini_write(conf, "test.ini");
    ini_table_destroy(conf);
    return 0;
}
```

If there's not currently an INI file, you can use:
```c
int main(void)
{
    ini_table_s* conf = ini_table_create();
    ini_entry_create(conf, "Section", "Lemons", "are great");
    ini_write(conf, "test.ini");
    ini_table_destroy(conf);
    return 0;
}
```

You don't need to check for NULL on ini_table_create(), since it creates an empty
structure.  Always make sure to call ini_table_destroy() when you're finished
using the data, however.

There's currently no support for saving comment lines, but I may bother to add it in the future if I get bored.

For more information on these functions, check out config.h, which I spent a bit of time documenting for easy reading.

