#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP_RUNNER(ProductionCode)
{
  RUN_TEST_CASE(ProductionCode, ini_table_get_entry_ShouldReturnTwo);  
	RUN_TEST_CASE(ProductionCode, ini_table_get_entry_ShouldReturnOne);  
	
}