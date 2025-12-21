#ifndef WINDMOLEN_OPTIONS_H_
#define WINDMOLEN_OPTIONS_H_

#include <stddef.h>
#include <stdint.h>



enum OptionType {
    OPTION_TYPE_CHECK,
    OPTION_TYPE_SPIN,
    OPTION_TYPE_COMBO,
    OPTION_TYPE_BUTTON,
    OPTION_TYPE_STRING,

    OPTION_TYPE_COUNT
};


static constexpr const char OPTION_THREAD_COUNT_NAME[]    = "Threads";
static constexpr enum OptionType OPTION_THREAD_COUNT_TYPE = OPTION_TYPE_SPIN;
static constexpr size_t OPTION_THREAD_COUNT_DEFAULT       = 1;
static constexpr size_t OPTION_THREAD_COUNT_MIN           = 1;
static constexpr size_t OPTION_THREAD_COUNT_MAX           = 1024;

static constexpr const char OPTION_HASH_SIZE_NAME[]    = "Hash";
static constexpr enum OptionType OPTION_HASH_SIZE_TYPE = OPTION_TYPE_SPIN;
static constexpr uint64_t OPTION_HASH_SIZE_DEFAULT     = 1;
static constexpr uint64_t OPTION_HASH_SIZE_MIN         = 1;
static constexpr uint64_t OPTION_HASH_SIZE_MAX         = 1;

static constexpr const char OPTION_CLEAR_HASH_NAME[]    = "Clear Hash";
static constexpr enum OptionType OPTION_CLEAR_HASH_TYPE = OPTION_TYPE_BUTTON;

static constexpr const char OPTION_PONDER_MODE_NAME[]    = "Ponder";
static constexpr enum OptionType OPTION_PONDER_MODE_TYPE = OPTION_TYPE_CHECK;
static constexpr bool OPTION_PONDER_MODE_DEFAULT         = false;

static constexpr const char OPTION_MOVE_OVERHEAD_NAME[]    = "Move Overhead";
static constexpr enum OptionType OPTION_MOVE_OVERHEAD_TYPE = OPTION_TYPE_SPIN;
static constexpr uint64_t OPTION_MOVE_OVERHEAD_DEFAULT     = 10;  // 10 ms.
static constexpr uint64_t OPTION_MOVE_OVERHEAD_MIN         = 0;
static constexpr uint64_t OPTION_MOVE_OVERHEAD_MAX         = 5000;  // 5 s.


// This structure contains the values of the various options that are supported and can be changed by the UCI protocol.
struct Options {
    size_t thread_count;
    uint64_t hash_size;
    uint64_t move_overhead;
    bool ponder_mode;
};


// Sets all options in `options` to their default values.
void initialize_options(struct Options* options);



#endif /* #ifndef WINDMOLEN_OPTIONS_H_ */
