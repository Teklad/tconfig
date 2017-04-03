#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"

#define TC_INITIAL_ARRAYSIZE 10
#define TC_INCREMENT_SIZE 10
#define TC_INITIAL_BUFFERSIZE 100
#define TC_BUFFER_INCREMENT_SIZE 100

char* tc_current_section = NULL;

typedef struct TConfigSection {
    char*  name;
    char** keys;
    char** values;
    int properties_len;
    int allocated_len;
} TConfigSection;

typedef struct TConfig {
    struct TConfigSection* sections;
    int sections_len;
    int allocated_len;
} TConfig;


int tconfig_create_section(TConfig* t, const char* section_name)
{
    for (int i = 0; i < t->sections_len; i++) {
        if (strcmp(t->sections[i].name, section_name) == 0) {
            return i;
        }
    }
    if (t->sections_len + 1 > t->allocated_len) {
        t->allocated_len += TC_INCREMENT_SIZE;
        t->sections = realloc(t->sections, 
                sizeof(TConfigSection)*t->allocated_len);
    }
    TConfigSection* section = &t->sections[t->sections_len];
    section->name = malloc((strlen(section_name)+1) * sizeof(char));
    strcpy(section->name, section_name);
    section->keys = malloc(TC_INITIAL_ARRAYSIZE * sizeof(char*));
    section->values = malloc(TC_INITIAL_ARRAYSIZE * sizeof(char*));
    section->properties_len = 0;
    section->allocated_len = TC_INITIAL_ARRAYSIZE;
    t->sections_len++;
    return t->sections_len-1;
}


int tconfig_create_property(TConfig* t, const char* section_name,
        const char* key, const char* value)
{
    int index = tconfig_create_section(t, section_name);
    TConfigSection* section = &t->sections[index];
    index = -1;
    for (int i = 0; i < section->properties_len; i++) {
        if (strcmp(section->keys[i], key) == 0) {
            index = i;
            break;
        }
    }
    if (index < 0) {
        if (section->properties_len+1 > section->allocated_len) {
            section->allocated_len += TC_INCREMENT_SIZE;
            section->keys = realloc(section->keys, 
                    section->allocated_len * sizeof(char*));
            section->values = realloc(section->values,
                    section->allocated_len * sizeof(char*));
        }
        index = section->properties_len++;
        section->keys[index] = malloc((strlen(key)+1) * sizeof(char));
        section->values[index] = malloc((strlen(value)+1) * sizeof(char));
        strcpy(section->keys[index], key);
        strcpy(section->values[index], value);
    }else {
        free(section->values[index]);
        section->values[index] = malloc((strlen(value)+1) * sizeof(char));
        strcpy(section->values[index], value);
    }
    return index;
}

void tconfig_skip_to_end(FILE* f)
{
    fscanf(f, "%*[^\n]\n", NULL);   
}

void tconfig_realloc_if_required(char* buf, int* bufsize, int count)
{
    if (count >= *bufsize) {
        buf = realloc(buf, (*bufsize + TC_BUFFER_INCREMENT_SIZE) * sizeof(char));
    }
}

bool tconfig_read_section(TConfig* t, FILE* f)
{
    int bufsize = TC_INITIAL_BUFFERSIZE;
    char* buf = malloc(bufsize * sizeof(char));
    memset(buf, '\0', bufsize);
    int count = 0;
    int spaces = 0;
    int c;
    do {
        c = getc(f);
        switch(c) {
            case ' ':
                if(buf[0] != '\0') spaces++;
                continue;
            case ']': {
                int index = tconfig_create_section(t, buf);
                tc_current_section = t->sections[index].name;
                free(buf);
                return true;
                break;
                      }
            case '\r':
            case '\n':
                printf("Missing ] near %s\n", buf);
                free(buf);
                tconfig_close(t);
                return false;
            default:
                for(;spaces > 0; spaces--) {
                    buf[count++] = ' ';
                    tconfig_realloc_if_required(buf, &bufsize, count);
                }
                buf[count++] = c;
                tconfig_realloc_if_required(buf, &bufsize, count);
                break;
        }
    }while (c != EOF);
    printf ("Reached end of file\n");
    free(buf);
    tconfig_close(t);
    return false;
}

bool tconfig_read_property(TConfig* t, FILE* f, int first_char)
{
    int bufsize = TC_INITIAL_BUFFERSIZE;
    char* buf = malloc(bufsize * sizeof(char));
    char* key = NULL;
    memset(buf, '\0', bufsize);
    int count = 0;
    int spaces = 0;
    if (first_char != ' ') buf[count++] = first_char;
    int c;
    do {
        c = getc(f);
        switch(c) {
            case ' ':
                if(buf[0] != '\0') spaces++;
                continue;
            case ';':
                tconfig_skip_to_end(f);
            case '\n':
            case '\r':
                c = EOF;
                continue;
            case '=':
                if (key == NULL) {
                    key = malloc((strlen(buf)+1)*sizeof(char));
                    strcpy(key, buf);
                    memset(buf, '\0', bufsize);
                    count = 0;
                    spaces = 0;
                    break;
                }
            default:
                for (;spaces > 0; spaces--) {
                    buf[count++] = ' ';
                    tconfig_realloc_if_required(buf, &bufsize, count);
                }
                buf[count++] = c;
                tconfig_realloc_if_required(buf, &bufsize, count);
                break;
        }
    }while (c != EOF);
    if (key != NULL) {
        tconfig_create_property(t, tc_current_section, key, buf);
        free(buf);
        free(key);
        return true;
    }
    printf("Missing `=' operator near %s\n", buf);
    free(buf);
    tconfig_close(t);
    return false;
}

TConfig* tconfig_init()
{
    TConfig* t = malloc(sizeof(TConfig));
    t->sections = malloc(TC_INITIAL_ARRAYSIZE*sizeof(TConfigSection));
    t->sections_len = 0;
    t->allocated_len = TC_INITIAL_ARRAYSIZE;
    return t;
}

TConfig* tconfig_open(const char* filepath)
{
    TConfig* t = tconfig_init();
    FILE* f;
    int c;
    f = fopen(filepath, "r");
    if (f == NULL) {
        return NULL;
    }
    do {
        c = getc(f);
        switch(c) {
            case EOF:
            case '\n':
            case '\r':
            case ' ':
                continue;
            case ';':
                tconfig_skip_to_end(f); 
                break;
            case '[':
                if(!tconfig_read_section(t, f)) {
                    fclose(f);
                    return NULL;
                }
                break;
            default:
                if(!tconfig_read_property(t, f, c)) {
                    fclose(f);
                    return NULL;
                }
                break;
        }
    }while (c != EOF);
    fclose(f);
    return t;
}

void tconfig_close(TConfig* t)
{
    if (t == NULL) return;
    for (int i = 0; i < t->sections_len; i++) {
        TConfigSection* section = &t->sections[i];
        for (int q = 0; q < section->properties_len; q++) {
            free(section->keys[q]);
            free(section->values[q]);
        }
        free(section->keys);
        free(section->values);
        free(section->name);
    }
    free(t->sections);
    free(t);
}

bool tconfig_write(TConfig* t, const char* filename)
{
    FILE* f = fopen(filename, "w+");
    if (f == NULL) return false;
    for (int i = 0; i < t->sections_len; i++) {
        TConfigSection* section = &t->sections[i];
        fprintf(f, "\n[%s]\n", section->name);
        for (int q = 0; q < section->properties_len; q++) {
            fprintf(f, "%s = %s\n", section->keys[q], section->values[q]);
        }
    }
    fclose(f);
    return true;
}

const char* tconfig_get_property_raw(TConfig* t, const char* section_name,
        const char* key)
{
    for (int i = 0; i < t->sections_len; i++) {
        TConfigSection* section = &t->sections[i];
        if(strcmp(section->name, section_name) == 0) {
            for (int q = 0; q < section->properties_len; q++) {
                if (strcmp(section->keys[q], key) == 0) {
                    return section->values[q];
                }
            }
        }
    }
    return NULL;
}

int tconfig_get_property_as_int(TConfig* t, const char* section_name,
        const char* key)
{
    const char* tmp = tconfig_get_property_raw(t, section_name, key);
    if (tmp != NULL) {
        return atoi(tmp);
    }
    return 0;
}

char* tconfig_get_property_as_char_array(TConfig* t, const char* section_name,
        const char* key)
{
    const char* tmp = tconfig_get_property_raw(t, section_name, key);
    char* mod = malloc((strlen(tmp)+1)*sizeof(char));
    if (tmp[0] == '"') {
        strcpy(mod, tmp+1);
    }else {
        strcpy(mod, tmp);
    }
    int last = strlen(mod)-1;
    if (mod[last] == '"') {
        mod[last] = '\0';
    }
    return mod;
}

