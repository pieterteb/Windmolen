#ifndef WINDMOLEN_THREAD_H_
#define WINDMOLEN_THREAD_H_


#include <threads.h>

#include "search.h"



bool start_search_thread(struct SearchState* search_state);
Move stop_search_thread(struct SearchState* search_state);



#endif /* #ifndef WINDMOLEN_THREAD_H_ */
