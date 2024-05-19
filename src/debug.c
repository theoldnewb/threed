#include "app.h"
#include "debug.h"
#include "log.h"
#include <SDL3/SDL_stdinc.h>


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//
static void const *
get_FILE(
    char const * file
)
{
    return file ;
}


static void const *
get_FUNC(
    char const * func
)
{
    return func ;
}


static int
get_FILE_FUNC(
    void const *    file
,   void const *    func
)
{
    void const * file_addr = get_FILE(__FILE__) ;
    void const * func_addr = get_FUNC(__func__) ;
    log_debug_ptr(file_addr) ;
    log_debug_ptr(func_addr) ;
    return file_addr == file && func_addr != func ;
}


int
test_debug_file_func()
{
    void const * file_addr = get_FILE(__FILE__) ;
    void const * func_addr = get_FUNC(__func__) ;
    log_debug_ptr(file_addr) ;
    log_debug_ptr(func_addr) ;
    return get_FILE_FUNC(file_addr, func_addr) ;
}


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//
typedef struct file_func_line_info
{
    char const *    file_ ;
    char const *    func_ ;
    int             line_ ;

} file_func_line_info ;



#define max_counter_keeper      (1<<8)
#define max_delta_count         (1<<8)
#define max_delta_count_mask    (max_delta_count - 1)

typedef struct counter_keeper counter_keeper ;

typedef struct counter_keeper
{
    file_func_line_info begin_ ;
    file_func_line_info end_ ;
    counter_keeper *    next_ ;
    counter_keeper *    last_ ;
    counter_keeper *    parent_ ;
    uint32_t            delta_count_index_ ;
    uint32_t            indent_ ;
    uint64_t            hit_count_ ;
    uint64_t            start_count_ ;
    uint64_t            end_count_ ;
    uint64_t            elapsed_count_ ;
    uint64_t            delta_count_[max_delta_count] ;

} counter_keeper ;


typedef struct counter_keeper_storage
{
    uint32_t            stack_index_ ;
    uint32_t            counter_keeper_count_ ;
    counter_keeper      counter_keeper_[max_counter_keeper] ;
    counter_keeper *    counter_keeper_stack_[max_counter_keeper] ;
    counter_keeper *    counter_keeper_hash_[max_counter_keeper] ;
    uint64_t            begin_count_ ;
    uint64_t            end_count_ ;

} counter_keeper_storage ;



////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//
static counter_keeper_storage   the_cks_ = { 0 } ;
static counter_keeper_storage * cks_ = &the_cks_ ;


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//
static void
dump_file_func_line_info(
    file_func_line_info const * ffli
)
{
    require(ffli) ;
    log_debug_str(ffli->file_) ;
    log_debug_str(ffli->func_) ;
    log_debug_u32(ffli->line_) ;
}


static void
dump_counter_keeper(
    counter_keeper const *  ck
)
{
    require(ck) ;
    dump_file_func_line_info(&ck->begin_) ;
    dump_file_func_line_info(&ck->end_) ;
    log_debug_u32(ck->delta_count_index_) ;
    log_debug_u32(ck->indent_) ;
    log_debug_u64(ck->hit_count_) ;
    log_debug_u64(ck->start_count_) ;
    log_debug_u64(ck->end_count_) ;
    log_debug_u64(ck->elapsed_count_) ;
    log_debug_u64(ck->elapsed_count_/1000) ;
    log_debug_u64(ck->elapsed_count_/1000000) ;
    log_debug_u64(ck->elapsed_count_/1000000000) ;

    for(
        uint32_t i = 0
    ;   i < ck->delta_count_index_
    ;   ++i
    )
    {
        log_debug_u32(i) ;
        log_debug_u64(ck->delta_count_[i]) ;
    }
}


void
dump_all_debug_counter_keepers()
{
    require(cks_) ;

    log_debug_u32(cks_->counter_keeper_count_) ;

    for(
        uint32_t i = 0
    ;   i < cks_->counter_keeper_count_
    ;   ++i
    )
    {
        log_debug_u32(i) ;
        dump_counter_keeper(&cks_->counter_keeper_[i]) ;
    }

    log_debug_u64(cks_->begin_count_) ;
    log_debug_u64(cks_->end_count_) ;
}


static counter_keeper *
make_counter_keeper(
    counter_keeper_storage *    cks
,   file_func_line_info const * ffli
)
{
    require(cks) ;
    require(ffli) ;
    require(cks->counter_keeper_count_ < max_counter_keeper) ;
    counter_keeper * ck = &cks->counter_keeper_[cks->counter_keeper_count_++] ;
    require(ck) ;

    ck->begin_.file_        = ffli->file_ ;
    ck->begin_.func_        = ffli->func_ ;
    ck->begin_.line_        = ffli->line_ ;
    ck->end_.file_          = ffli->file_ ;
    ck->end_.func_          = ffli->func_ ;
    ck->end_.line_          = ffli->line_ ;
    ck->next_               = NULL ;
    ck->last_               = NULL ;
    ck->parent_             = NULL ;
    ck->delta_count_index_  = 0 ;
    ck->indent_             = 0 ;
    ck->hit_count_          = 0 ;
    ck->start_count_        = 0 ;
    ck->end_count_          = 0 ;
    SDL_memset(ck->delta_count_, 0, sizeof(ck->delta_count_)) ;

    return ck ;
}


static uint8_t
calc_file_func_hash(
    void const * file
,   void const * func
)
{
    return ((uintptr_t)file + (uintptr_t)func) % 251 ;
}


static counter_keeper *
find_begin_counter_keeper(
    counter_keeper_storage *    cks
,   file_func_line_info const * ffli
)
{
    require(cks) ;
    require(ffli) ;

    uint8_t const hk = calc_file_func_hash(ffli->file_, ffli->func_) ;

    for(
        counter_keeper * p = cks->counter_keeper_hash_[hk]
    ;   p
    ;   p = p->next_
    )
    {
        if(
            p->begin_.file_ == ffli->file_
        &&  p->begin_.func_ == ffli->func_
        &&  p->begin_.line_ == ffli->line_
        )
        {
            return p ;
        }
    }

    return NULL ;
}


static counter_keeper *
find_end_counter_keeper(
    counter_keeper_storage *    cks
,   file_func_line_info const * ffli
)
{
    require(cks) ;
    require(ffli) ;

    uint8_t const hk = calc_file_func_hash(ffli->file_, ffli->func_) ;

    for(
        counter_keeper * p = cks->counter_keeper_hash_[hk]
    ;   p
    ;   p = p->next_
    )
    {
        if(
            p->begin_.file_ == ffli->file_
        &&  p->begin_.func_ == ffli->func_
        &&  p               == cks->counter_keeper_stack_[cks_->stack_index_]
        )
        {
            return p ;
        }
    }

    return NULL ;
}

static void
insert_counter_keeper(
    counter_keeper_storage *    cks
,   counter_keeper *            ck
)
{
    require(cks) ;
    require(ck) ;

    uint8_t const hk = calc_file_func_hash(ck->begin_.file_, ck->begin_.func_) ;

    counter_keeper ** p = &cks->counter_keeper_hash_[hk] ;
    counter_keeper *  q = *p ;

    if(q)
    {
        if(q->last_)
        {
            require(!q->last_->next_) ;
            require(!q->last_->last_) ;
            q->last_->next_ = ck ;
            q->last_        = ck ;
        }
        else
        {
            require(!q->next_) ;
            q->next_ = ck ;
            q->last_ = ck ;
        }
    }
    else
    {
        *p = ck ;
    }
}


void
begin_timed_block_impl(
    char const *    file
,   char const *    func
,   int const       line
)
{
    require(file) ;
    require(func) ;
    require(cks_) ;

    log_output_impl(file, func, line, LOG_PRI_DEBUG, "enter %s", func) ;

    ++cks_->begin_count_ ;

    file_func_line_info ffli = { 0 } ;
    ffli.file_ = file ;
    ffli.func_ = func ;
    ffli.line_ = line ;

    counter_keeper * ck = find_begin_counter_keeper(cks_, &ffli) ;
    if(!ck)
    {
        ck = make_counter_keeper(cks_, &ffli) ;
        require(ck) ;
        insert_counter_keeper(cks_, ck) ;
        require(find_begin_counter_keeper(cks_, &ffli)) ;
    }

    require(cks_->stack_index_ < max_counter_keeper) ;
    ck->indent_ = cks_->stack_index_ ;
    ck->parent_ = cks_->counter_keeper_stack_[cks_->stack_index_++] ;
    cks_->counter_keeper_stack_[cks_->stack_index_] = ck ;

    ck->hit_count_++ ;
    ck->start_count_ = get_app_time() ;
}


void
end_timed_block_impl(
    char const *    file
,   char const *    func
,   int const       line
)
{
    require(file) ;
    require(func) ;
    require(cks_) ;


    log_output_impl(file, func, line, LOG_PRI_DEBUG, "leave %s", func) ;

    ++cks_->end_count_ ;

    file_func_line_info ffli = { 0 } ;
    ffli.file_ = file ;
    ffli.func_ = func ;
    ffli.line_ = line ;

    counter_keeper * ck = find_end_counter_keeper(cks_, &ffli) ;
    require(ck) ;
    require(cks_->stack_index_ < max_counter_keeper) ;
    --cks_->stack_index_ ;

    require(ck->begin_.file_ == ck->end_.file_) ;
    require(ck->begin_.func_ == ck->end_.func_) ;

    ck->end_.file_      = ffli.file_ ;
    ck->end_.func_      = ffli.func_ ;
    ck->end_.line_      = ffli.line_ ;
    ck->end_count_      = get_app_time() ;
    ck->elapsed_count_  = ck->end_count_ - ck->start_count_ ;

    ck->delta_count_[ck->delta_count_index_] = ck->elapsed_count_ ;

    ++ck->delta_count_index_ ;
    ck->delta_count_index_ &= max_delta_count_mask ;
}


int
check_begin_end_timed_block_mismatch()
{
    log_debug_u64(cks_->begin_count_) ;
    log_debug_u64(cks_->end_count_) ;
    return cks_->begin_count_ == cks_->end_count_ ;
}


