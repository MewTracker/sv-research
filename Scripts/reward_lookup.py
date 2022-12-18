#!/usr/bin/env python3
if __name__ == "__main__":
    lookup = 'static const int8_t random_lookup[] =\n{\n'
    items = []
    for i in range(100):
        index = 4
        if i < 10:
            index = 0
        elif i < 40:
            index = 1
        elif i < 70:
            index = 2
        elif i < 90:
            index = 3
        items.append(index)
        if len(items) == 10:
            lookup += '    %s,\n' % ', '.join([str(x) for x in items])
            items.clear()
    lookup += '};'
    print(lookup)
