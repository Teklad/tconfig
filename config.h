#ifndef _TCONFIG_H_
#define _TCONFIG_H_
#include <stdbool.h>

typedef struct TConfig TConfig;

/**
 * @brief Initialized an empty TConfig struct pointer.  This is good for writing
 *        to a file that doesn't currently exist.
 * @return TConfig*
 */
TConfig* tconfig_init();
/**
 * @brief Reads the specified INI file and returns a TConfig struct pointer
 *        on success, or NULL on failure.
 * @param file
 * @return TConfig*
 */
TConfig* tconfig_open(const char* file);

/**
 * @brief Cleans up all the allocated data within the TConfig struct pointer
 *        so you don't have to!
 * @param conf
 */
void tconfig_close(TConfig* conf);

/**
 * @brief Attempts to write the TConfig struct data to the specified 'file'
 *        and returns true on success, or false otherwise.
 *        NOTE: Comments are not currently preserved with this function.
 * @param conf
 * @param file
 * @return bool
 */
bool tconfig_write(TConfig* conf, const char* file);

/**
 * @brief Attempts to create the property `key' with the specified `value'.  If
 *        the key already exists in the given section, it will be modified with
 *        the provided value.  If the given section does not exist, it will be
 *        created.  Returns an integer containing the index of `key', or -1 on 
 *        failure.
 * @param conf
 * @param section_name
 * @param key
 * @param value
 * @return int
 */
int  tconfig_create_property(TConfig* conf, const char* section_name,
        const char* key, const char* value);

/**
 * @brief Gets a direct pointer to the TConfig property for read-only access.
 *        This is probably the safest option in most cases.
 * @param conf
 * @param section_name
 * @param key
 * @return const char*
 */
const char* tconfig_get_property_raw(TConfig* conf, 
        const char* section_name,
        const char* key);

/**
 * @brief Gets the char array data from the TConfig property and removes any 
 *        leading or trailing quotation marks.  This is useful if you don't
 *        feel like formatting the string yourself.  Make sure to call free()
 *        on the newly created array when you're done!
 * @param conf
 * @param section_name
 * @param key
 * @return char*
 */
char* tconfig_get_property_as_char_array(TConfig* conf,
        const char* section_name,
        const char* key);

/**
 * @brief Gets the TConfig property converted to an integer.  This is a
 *        convenience function.  Returns an integer, or 0 if conversion failed.
 * @param conf
 * @param section_name
 * @param key
 * @return int
 */
int tconfig_get_property_as_int(TConfig* conf,
        const char* section_name,
        const char* key);

#endif//_TCONFIG_H_
