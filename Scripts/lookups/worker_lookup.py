#!/usr/bin/env python3
from itertools import product

if __name__ == "__main__":
    lookup = 'static const LPTHREAD_START_ROUTINE workers_gem[] =\n{\n'
    items = []
    for worker in product(['false', 'true'], repeat=6):
        lookup += '    worker_thread_wrapper<EncounterType::Gem, %s>,\n' % ', '.join(worker)
    lookup += '};\n'
    lookup += 'static const LPTHREAD_START_ROUTINE workers_dist[] =\n{\n'
    for worker in product(['false', 'true'], repeat=5):
        lookup += '    worker_thread_wrapper<EncounterType::Dist, false, %s>,\n' % ', '.join(worker)
    lookup += '};\n'
    print(lookup)
