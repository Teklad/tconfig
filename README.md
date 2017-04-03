A basic INI reader/writer built in C, with minimal dependencies.

This was an attempt to write something compact and easy to use for reading
and writing INI files in my projects.

It's able to read most INI formats without any issues.  Comments are delimited by
the ';' character.

A basic usage example would be something like this:
```c
int main(void)
{
    TConfig* conf = tconfig_open("test.ini");
    if (conf == NULL) {
        return -1;
    }
    printf("%s\n", tconfig_get_property_raw(conf, "Section", "Lemons");
    tconfig_close(conf);
    return 0;
}
```

If you want to modify an existing property:
```c
int main(void)
{
    TConfig* conf = tconfig_open("test.ini");
    if (conf == NULL) {
        return -1;
    }
    tconfig_create_property(conf, "Section", "Lemons", "are great");
    tconfig_write(conf, "test.ini");
    tconfig_close(conf);
    return 0;
}
```

If there's not currently an INI file, you can use:
```c
int main(void)
{
    TConfig* conf = tconfig_init();
    tconfig_create_property(conf, "Section", "Lemons", "are great");
    tconfig_write(conf, "test.ini");
    tconfig_close(conf);
    return 0;
}
```

You don't need to check for NULL on tconfig_init(), since it just initializes an
empty struct.  The memory more or less manages itself so long as you call tconfig_close() when you're finished, with the exception of tconfig_get_property_as_char_array(), which returns a modifiable version of the value.  You'll need to call free() if you use that function.

For more information on these functions, check out config.h, which I spent a bit of time documenting for easy reading.

