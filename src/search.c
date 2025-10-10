#include "search.h"

#include "move_generation.h"
#include "uci.h"



int start_search(void* arguments) {
    assert(arguments != NULL);

    struct SearchState* search_state = (struct SearchState*)arguments;

    if (search_state->move_count == 0)
        search_state->move_count = generate_legal_moves(&main_position, search_state->movelist);

    search_state->best_move        = search_state->movelist[0];
    search_state->search_completed = true;

    printf("info pv ");
    print_move(stdout, search_state->best_move);
    putc('\n', stdout);
    fflush(stdout);

    printf("bestmove ");
    print_move(stdout, search_state->best_move);
    putc('\n', stdout);
    fflush(stdout);

    return 0;
}
