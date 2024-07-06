

def normalize(t, n, m):
    return t * n / m

def next_power_of_two(n):
    assert(n > 0)
    return 1<<(n-1).bit_length()
