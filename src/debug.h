#pragma once


void
begin_timed_block_impl(
    char const *    file
,   char const *    func
,   int const       line
) ;


void
end_timed_block_impl(
    char const *    file
,   char const *    func
,   int const       line
) ;


int
test_debug_file_func() ;


void
dump_all_debug_counter_keepers() ;


#ifdef  ENABLE_TIMED_BLOCK
#define begin_timed_block() begin_timed_block_impl(__FILE__, __func__, __LINE__)
#define end_timed_block()   end_timed_block_impl(__FILE__, __func__, __LINE__)
#else
#define begin_timed_block() def_noop
#define end_timed_block()   def_noop
#endif

