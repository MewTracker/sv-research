#!/usr/bin/env python3


def gen_diff_0(i):
    if i > 80:
        return 2
    else:
        return 1


def gen_diff_1(i):
    if i > 70:
        return 3
    elif i > 30:
        return 2
    else:
        return 1


def gen_diff_2(i):
    if i > 70:
        return 4
    elif i > 40:
        return 3
    elif i > 20:
        return 2
    else:
        return 1


def gen_diff_3(i):
    if i > 75:
        return 5
    elif i > 40:
        return 4
    else:
        return 3


def gen_diff_4(i):
    if i > 70:
        return 5
    elif i > 30:
        return 4
    else:
        return 3


def gen_lookup(gen, count, per_row, indent=4):
    lookup = ''
    items = []
    for i in range(count):
        items.append(gen(i))
        if len(items) == per_row or i == count - 1:
            lookup += '%s%s,\n' % (' ' * indent, ', '.join([str(x) for x in items]))
            items.clear()
    return lookup


if __name__ == "__main__":
    generators = (
        gen_diff_0,
        gen_diff_1,
        gen_diff_2,
        gen_diff_3,
        gen_diff_4,
    )
    lookup = 'static const uint8_t star_count_lookup[] =\n{\n'
    for gen in generators:
        lookup += '    {\n'
        lookup += gen_lookup(gen, 100, 10, 8)
        lookup += '    },\n'
    lookup += '};\n'
    print(lookup)
