#!/usr/bin/env python3


def gen_main(ratio):
    if ratio == 255:
        return 2
    elif ratio == 254:
        return 1
    elif ratio == 0:
        return 0
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
    lookup = 'static const uint8_t fixed_lookup[] =\n{\n'
    lookup += gen_lookup(gen_main, 256, 16)
    lookup += '};\n'
    ratio_data = (
        (0x1F, 12),
        (0x3F, 25),
        (0x7F, 50),
        (0xBF, 75),
        (0xE1, 89),
    )
    def gen_ratio_lookup_index(ratio):
        for i, data in enumerate(ratio_data):
            if data[0] == ratio:
                return i
        return len(ratio_data)
    lookup += 'static const uint8_t ratio_lookup_index[] = \n{\n'
    lookup += gen_lookup(gen_ratio_lookup_index, 256, 16)
    lookup += '};\n'
    lookup += 'static const uint8_t ratio_lookup_gender[%d][100] =\n{\n' % len(ratio_data)
    for ratio, threshold in ratio_data:
        lookup += '    {\n'
        lookup += gen_lookup(lambda i: 1 if i < threshold else 0, 100, 10, 8)
        lookup += '    },\n'
    lookup += '};\n'
    print(lookup)
