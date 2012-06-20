#define USAGE_MESSAGE \
"  NAME                                                                 \n"\
"         pump -- build persistent pipe engines                         \n"\
"                                                                       \n"\
"  SYNOPSIS                                                             \n"\
"         pump [operator] ([option]) [operand]                          \n"\
"                                                                       \n"\
"  DESCRIPTION                                                          \n"\
"         Pump does lots of stuff for you that you don't want to do     \n"\
"         on your own, or you don't have time to do. Maybe you don't    \n"\
"         want to bother using the crontab, or maybe you want to set    \n"\
"         up a pipeline but want it to sit still while you play around  \n"\
"         with something else. Maybe you would rather mv files than     \n"\
"         wrestle with stdin and getopt(s).                             \n"\
"                                                                       \n"\
"  OPERATORS                                                            \n"\
"         The following operators are supported:                        \n"\
"                                                                       \n"\
"         init         create a new pump in the working directory       \n"\
"         logic <path> supply a path to external logic (script)         \n"\
"         var   <name> print value of an option in the config file      \n"\
"         config       open the config file for editing in a text editor\n"\
"                                                                       \n"\
"  OPTIONS                                                              \n"\
"         Options aren't supported yet                                  \n"\
"                                                                       \n"\
"  OPERANDS                                                             \n"\
"         Tired of writing this now                                     \n"

