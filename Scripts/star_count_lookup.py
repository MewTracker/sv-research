#!/usr/bin/env python3
if __name__ == "__main__":
    lookup = 'static const int8_t star_count_lookup[] =\n{\n'
    items = []
    for i in range(100):
        count = 3
        if i > 70:
            count = 5
        elif i > 30:
            count = 4
        items.append(count)
        if len(items) == 10:
            lookup += '    %s,\n' % ', '.join([str(x) for x in items])
            items.clear()
    lookup += '};'
    print(lookup)