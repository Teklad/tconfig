#include "config.h"
#include "unistd.h"
#include "unity.h"
#include "unity_fixture.h"



TEST_GROUP(ProductionCode);

//sometimes you may want to get at local data in a module.
//for example: If you plan to pass by reference, this could be useful
//however, it should often be avoided
ini_table_s* config;
TEST_SETUP(ProductionCode)
{
    ini_table_s* config = ini_table_create();
		unlink("test.ini");
    
        puts("test.ini does not exist! Adding entries!");
        ini_table_create_entry(config, "Section", "one", "two");
        ini_table_create_entry(config, "Sec2", "left", "1");
        ini_table_write_to_file(config, "test.ini");

    ini_table_destroy(config);
}

TEST_TEAR_DOWN(ProductionCode)
{
  
}

TEST(ProductionCode, ini_table_get_entry_ShouldReturnTwo)
{
  //All of these should pass
  ini_table_s* config = ini_table_create();
		
	if (!ini_table_read_from_file(config, "test.ini")) {
	TEST_ASSERT_EQUAL_STRING("two",ini_table_get_entry(config, "Section", "one"));
	}
	ini_table_destroy(config);
}

TEST(ProductionCode, ini_table_get_entry_ShouldReturnOne)
{
  //All of these should pass
  ini_table_s* config = ini_table_create();
		
	if (!ini_table_read_from_file(config, "test.ini")) {
	TEST_ASSERT_EQUAL_STRING("1",ini_table_get_entry(config, "Sec2", "left"));
	}
	ini_table_destroy(config);
}

