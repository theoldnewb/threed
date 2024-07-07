

# return ((n + a - 1)) / a) * a ;
def align_number(n, a):
    assert(int(n) >= int(0))
    assert(int(a) > int(0))
    a_minus_one = a - 1
    t0 = int(n + a_minus_one)
    t1 = int(int(t0) / int(a))
    t2 = int(int(t1) * int(a))
    return int(t2)


# return (u64)0 == (n - ((n / a) * a)) ;
def is_aligned(n, a):
    t0 = int(int(n) / int(a))
    t1 = int(int(t0) * int(a))
    t2 = int(int(n) - int(t1))
    return int(0) == int(t2)
    #return int(0) == int(int(n) % int(a))


def normalize(t, n, m):
    return t * n / m


def next_power_of_two(n):
    assert(n > 0)
    return 1<<(n-1).bit_length()


def hello_math():
    print("hello_math")
    assert( 0 == align_number( 0, 8))
    assert( 8 == align_number( 1, 8))
    assert( 8 == align_number( 2, 8))
    assert( 8 == align_number( 3, 8))
    assert( 8 == align_number( 4, 8))
    assert( 8 == align_number( 5, 8))
    assert( 8 == align_number( 6, 8))
    assert( 8 == align_number( 7, 8))
    assert( 8 == align_number( 8, 8))
    assert(16 == align_number( 9, 8))
    assert(16 == align_number(10, 8))
    assert(16 == align_number(11, 8))
    assert(16 == align_number(12, 8))
    assert(16 == align_number(13, 8))
    assert(16 == align_number(14, 8))
    assert(16 == align_number(15, 8))
    assert(16 == align_number(16, 8))
    assert(24 == align_number(17, 8))

    assert(is_aligned( 0, 8) == True)
    assert(is_aligned( 1, 8) == False)
    assert(is_aligned( 2, 8) == False)
    assert(is_aligned( 3, 8) == False)
    assert(is_aligned( 4, 8) == False)
    assert(is_aligned( 5, 8) == False)
    assert(is_aligned( 6, 8) == False)
    assert(is_aligned( 7, 8) == False)
    assert(is_aligned( 8, 8) == True)
    assert(is_aligned( 9, 8) == False)
    assert(is_aligned(10, 8) == False)
    assert(is_aligned(11, 8) == False)
    assert(is_aligned(12, 8) == False)
    assert(is_aligned(13, 8) == False)
    assert(is_aligned(14, 8) == False)
    assert(is_aligned(15, 8) == False)
    assert(is_aligned(16, 8) == True)
    assert(is_aligned(17, 8) == False)

    assert( 1 == next_power_of_two( 1))
    assert( 2 == next_power_of_two( 2))
    assert( 4 == next_power_of_two( 3))
    assert( 4 == next_power_of_two( 4))
    assert( 8 == next_power_of_two( 5))
    assert( 8 == next_power_of_two( 6))
    assert( 8 == next_power_of_two( 7))
    assert( 8 == next_power_of_two( 8))
    assert(16 == next_power_of_two( 9))
    assert(16 == next_power_of_two(10))
    assert(16 == next_power_of_two(11))
    assert(16 == next_power_of_two(12))
    assert(16 == next_power_of_two(13))
    assert(16 == next_power_of_two(14))
    assert(16 == next_power_of_two(15))
    assert(16 == next_power_of_two(16))
    assert(32 == next_power_of_two(17))
    assert(64 == next_power_of_two(63))
    assert(64 == next_power_of_two(64))


