#!/usr/bin/env python3
from itertools import product

if __name__ == "__main__":
    lookup = 'static const LPTHREAD_START_ROUTINE workers[] =\n{\n'
    items = []
    for worker in product(['false', 'true'], repeat=5):
        lookup += '    worker_thread_wrapper<%s>,\n' % ', '.join(worker)
    lookup += '};'
    print(lookup)
