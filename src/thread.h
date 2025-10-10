#ifndef THREAD_H
#define THREAD_H


#include <threads.h>

#include "search.h"



bool start_search_thread(struct SearchState* search_state);
Move stop_search_thread(struct SearchState* search_state);



#endif /* #ifndef THREAD_H */
