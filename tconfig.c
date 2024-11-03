/*
MIT License

Copyright (c) 2019 Justin Kinnaird

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tconfig.h"

static ini_entry_s* _ini_entry_create(ini_section_s* section,
        const char* key, const char* value)
{
    if ((section->size % 10) == 0) {
        section->entry =
            realloc(section->entry, (10+section->size) * sizeof(ini_entry_s));
    }
    ini_entry_s* entry = &section->entry[section->size++];
    strncpy(entry->key, key, INI_MAXLEN);
    strncpy(entry->value, value, INI_MAXLEN);
    return entry;
}

static ini_section_s* _ini_section_create(ini_table_s* table,
    const char* section_name)
{
    if ((table->size % 10) == 0) {
        table->section =
            realloc(table->section, (10+table->size) * sizeof(ini_section_s));
    }
    ini_section_s* section = &table->section[table->size++];
    section->size = 0;
    strncpy(section->name, section_name, INI_MAXLEN);
    section->entry = malloc(10 * sizeof(ini_entry_s));
    return section;
}

static ini_section_s* _ini_section_find(ini_table_s* table, const char* name)
{
    for (int i = 0; i < table->size; i++) {
        if (!strncmp(table->section[i].name, name, INI_MAXLEN)) {
            return &table->section[i];
        }
    }
    return NULL;
}

static ini_entry_s* _ini_entry_find(ini_section_s* section, const char* key)
{
    for (int i = 0; i < section->size; i++) {
        if (!strncmp(section->entry[i].key, key, INI_MAXLEN)) {
            return &section->entry[i];
        }
    }
    return NULL;
}

static ini_entry_s* _ini_entry_get(ini_table_s* table, const char* section_name,
        const char* key)
{
    ini_section_s* section = _ini_section_find(table, section_name);
    if (section == NULL) {
        return NULL;
    }
    ini_entry_s* entry = _ini_entry_find(section, key);
    if (entry == NULL) {
        return NULL;
    }
    return entry;
}

ini_table_s* ini_table_create(void)
{
    ini_table_s* table = malloc(sizeof(ini_table_s));
    table->size = 0;
    table->section = malloc(10 * sizeof(ini_section_s));
    return table;
}

void ini_table_destroy(ini_table_s* table)
{
    for (int i = 0; i < table->size; i++) {
        ini_section_s* section = &table->section[i];
        free(section->entry);
    }
    free(table->section);
    free(table);
}

bool ini_table_read_from_file(ini_table_s* table, const char* file)
{
    FILE* f = fopen(file, "r");
    if (f == NULL) return false;

    enum {Section, Key, Value, Comment} state = Section;
    int      c;
    unsigned position = 0;
    int      spaces   = 0;
    int      line     = 0;
    size_t   buffer_size = 128 * sizeof(char);
    char*    buf   = malloc(buffer_size);
    char*    value = NULL;

    ini_section_s* current_section = NULL;

    memset(buf, '\0', buffer_size);
    while((c = getc(f)) != EOF) {
        if (position > buffer_size-2) {
            buffer_size += 128 * sizeof(char);
            size_t value_offset = value == NULL ? 0 : value - buf;
            buf = realloc(buf, buffer_size);
            memset(buf+position, '\0', buffer_size-position);
            if (value != NULL)
                value = buf + value_offset;
        }
        switch(c) {
            case ' ':
                switch(state) {
                    case Value: if (value[0] != '\0') spaces++; break;
                    default: if (buf[0] != '\0') spaces++; break;
                }
                break;
            case ';':
                if (state == Value) {
                    buf[position++] = c;
                    break;
                } else {
                    state = Comment;
                    buf[position++] = c;
                    while (c != EOF && c != '\n') {
                       c = getc(f);
                       if (c != EOF && c != '\n') buf[position++] = c;
                    }
                }
            // fallthrough
            case '\n':
            // fallthrough
            case EOF:
                line++;
                if (state == Value) {
                    if (current_section == NULL) {
                        current_section = _ini_section_create(table, "");
                    }
                    _ini_entry_create(current_section, buf, value);
                    value = NULL;
                } else if (state == Comment) {
                    if (current_section == NULL) {
                        current_section = _ini_section_create(table, "");
                    }
                    _ini_entry_create(current_section, buf, "");
                } else if (state == Section) {
                    fprintf(stderr, "TConfig [Line %d]: Section `%s'"
                        " missing `]' operator.", line, buf);
                } else if(state == Key && position) {
                    fprintf(stderr, "TConfig [Line %d]: Key `%s'"
                        " missing `=' operator.", line, buf);
                }
                memset(buf, '\0', buffer_size);
                state = Key;
                position = 0;
                spaces = 0;
                break;
            case '[':
                state = Section;
                break;
            case ']':
                current_section = _ini_section_create(table, buf);
                memset(buf, '\0', buffer_size);
                position = 0;
                spaces = 0;
                state = Key;
                break;
            case '=':
                if (state == Key) {
                    state = Value;
                    buf[position++] = '\0';
                    value = buf + position;
                    spaces = 0;
                    continue;
                }
            default:
                for(;spaces > 0; spaces--) buf[position++] = ' ';
                buf[position++] = c;
                break;
        }
    }
    free(buf);
    fclose(f);
    return true;
}

bool ini_table_write_to_file(ini_table_s* table, const char* file)
{
    FILE* f = fopen(file, "w+");
    if (f == NULL) return false;
    for (int i = 0; i < table->size; i++) {
        ini_section_s* section = &table->section[i];
        fprintf(f, i > 0 ? "\n[%s]\n" : "[%s]\n", section->name);
        for (int q = 0; q < section->size; q++) {
            ini_entry_s* entry = &section->entry[q];
            if (entry->key[0] == ';') {
                fprintf(f, "%s\n", entry->key);
            } else {
                fprintf(f, "%s = %s\n", entry->key, entry->value);
            }
        }
    }
    fclose(f);
    return true;
}


void ini_table_create_entry(ini_table_s* table, const char* section_name,
        const char* key, const char* value)
{
    ini_section_s* section = _ini_section_find(table, section_name);
    if (section == NULL) {
        section = _ini_section_create(table, section_name);
    }
    ini_entry_s* entry = _ini_entry_find(section, key);
    if (entry == NULL) {
        entry = _ini_entry_create(section, key, value);
    }else {
        strncpy(entry->value, value, INI_MAXLEN);
    }
}

bool ini_table_check_entry(ini_table_s* table, const char* section_name,
        const char* key)
{
    return (_ini_entry_get(table, section_name, key) != NULL);
}

const char* ini_table_get_entry(ini_table_s* table, const char* section_name,
        const char* key)
{
    ini_entry_s* entry = _ini_entry_get(table, section_name, key);
    if (entry == NULL) {
        return NULL;
    }
    return entry->value;
}

bool ini_table_get_entry_as_int(ini_table_s* table, const char* section_name,
        const char* key, int* value)
{
    const char* val = ini_table_get_entry(table, section_name, key);
    if (val == NULL) {
        return false;
    }
    *value = atoi(val);
    return true;
}

bool ini_table_get_entry_as_bool(ini_table_s* table, const char* section_name,
        const char* key, bool* value)
{
    const char* val = ini_table_get_entry(table, section_name, key);
    if (val == NULL) {
        return false;
    }
    if (!strncmp(val, "on", INI_MAXLEN) || !strncmp(val, "true", INI_MAXLEN)) {
        *value = true;
    }else {
        *value = false;
    }
    return true;
}
