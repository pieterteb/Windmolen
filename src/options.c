#include "options.h"

#include <assert.h>



void initialize_options(struct Options* options) {
    assert(options != nullptr);

    options->thread_count  = OPTION_THREAD_COUNT_DEFAULT;
    options->hash_size     = OPTION_HASH_SIZE_DEFAULT;
    options->move_overhead = OPTION_MOVE_OVERHEAD_DEFAULT;
    options->ponder_mode   = OPTION_PONDER_MODE_DEFAULT;
}
