#define USE_ERRNO_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <stdarg.h>
#include <locale.h>
#include <langinfo.h>

#include "file.h"
#include "error.h"
#include "util.h"
#include "textutils.h"
#include "list/list.h"
#include "lib/bloom/bloom.h"
#include "bits.h"
#include "dir.h"


/****************************************************************************** 
 * DIRECTORY ENTRY GENERATORS
 * 
 * Internally, this module uses the functions _diterate() and diterate()
 * to provide a uniform iteration interface for all other functions listed 
 * in this source file.
 *
 * A generator is a special function that controls the iteration behavior
 * of a loop. It generates a sequence of values, but instead of delivering
 * the values all at once, it yields them one at a time.
 *
 * The most familiar example of generator behavior is strtok(), which allows
 * for continutation. In practice, strtok() accepts a valid pointer to signal 
 * a fresh invocation, and subsequent "continuation" calls are indicated by
 * passing a NULL pointer in place of the valid one.
 *
 * There is a good example from the strtok_r manpage that constructs this 
 * convention as a for loop, Adapted to our purposes, it would look like:
 *
 *      struct dirent *entry; 
 *      DIR *dir;
 *
 *      for (entry = _diterate(dir));
 *           entry != NULL;
 *           entry = _diterate(NULL)) 
 *      {
 *              ...do something with entry
 *      }
 *
 * This still looks more like a hack than a generator. Wikipedia sayeth:
 *
 *       "In short, a generator *looks* like a function but *behaves* 
 *       like an iterator."
 *
 * The for loop construct is some halfway horror between these two ideals.
 * To nudge it toward readability, we move the canonical form to an
 * implementation function, _diterate(), and wrap this with diterate(). 
 *
 * diterate() keeps track of the continuation status of the generator,
 * so the caller can forgo the initialization step. This allows for a while 
 * loop, with the termination condition triggered implicitly.
 *
 * Thus sugar-coated, the generator assumes a brevity exemplified by the 
 * filecount() function:
 *
 *      int filecount(DIR *dir, int filter)
 *      {
 *              struct dirent *entry;
 *              int count = 0;
 *
 *              while ((entry = diterate(dir, filter))) {
 *                      count++;
 *              }
 *
 *              return count;
 *      }
 *
 * CAVEAT
 * These functions are not re-entrant safe!
 *
 ******************************************************************************/

/**
 * _diterate 
 * `````````
 * Provides iteration over directory contents to diterate()
 * 
 * @dir   : an open directory stream
 * @filter: a set of predicates (see note on file predicates)
 * Return : a struct dirent pointer set to the current item 
 *
 * USAGE
 * Invocation is similar to strtok(), see note above.
 *
 * NOTES
 * This is an internal function which is wrapped by diterate().
 *
 * CAVEAT
 * The call to stat() has been a rich source of bugs. It expects a
 * file path as its first argument, and many heads were scratched
 * before realizing that the name of the directory entry would only 
 * produce a valid path if the current working directory was the same 
 * as the path the DIR stream was opened at.
 *
 * cwd--+-somedir
 *      +-subdir--+-file0
 *                +-file1
 *                +-file2
 *
 * If we are executing in cwd, and open the DIR stream with 
 *
 *      DIR *dir = opendir(subdir),
 *
 * and we retreive the entries of the directory stream with
 *
 *      char *entry = readdir(dir);
 *
 * we're going to get values that look like
 *
 *      entry == "file0"
 *      entry == "file1"
 *      entry == "file2".
 *
 * When we give these paths to stat(), we're in trouble because stat()
 * naturally assumes that all relative paths are rooted in the current 
 * working directory, 'cwd'. 
 *
 * In fact, these paths are relative to the root of the DIR stream, 
 * 'subdir'. So stat() returns negative, unless the path of the DIR 
 * stream just happens to coincide with the current working directory, 
 * in which case it works as intended.
 *
 * The solution is to give stat() the true path.
 */
struct dirent *_diterate(DIR *dir, int filter)
{
        struct dirent *entry;
        struct stat dstat;
        static struct cwd_t cwd;
        static DIR *_dir;
        static int _dir_fd;
        static char dirpath[PATHSIZE];
        static char fdpath[PATHSIZE];

        if (dir != NULL) {
                _dir = dir;
                _dir_fd = dirfd(dir);
                snprintf(fdpath, PATHSIZE, "/proc/self/fd/%d", _dir_fd);
                readlink(fdpath, dirpath, PATHSIZE);
                cwd_mark(&cwd);
        }

        while ((entry = readdir(_dir)), entry != NULL) {

                cwd_shift(&cwd, dirpath);

                /* If we cannot stat a file, we move on. */
                if (stat(entry->d_name, &dstat) == -1) {
                        cwd_revert(&cwd);
                        continue;
                }

                cwd_revert(&cwd);

                /* If file is hidden and we don't have F_HID, move on */
                if (entry->d_name[0] == '.')
                        continue;

                /* If filetype is not included in the filter, move on */
                if (!(hasvalue(filter, F_TYPE(dstat.st_mode))))
                        continue;

                return entry;
        }
        /* Finished scanning directory */
        rewinddir(_dir);
        return NULL;
}


/**
 * diterate
 * ````````
 * Provide a uniform directory entry generator.
 *
 * @dir   : open directory stream
 * @filter: a set of predicates (see note on file predicates)
 * Return : struct dirent pointing to the current item
 *
 * NOTES
 * This is intended for internal use within this source file.
 */
struct dirent *diterate(DIR *dir, int filter)
{
        static bool running = false;
        struct dirent *entry;
        DIR *_dir = NULL; 

        if (!running) {
                _dir    = dir;
                running = true;
        }

        entry = _diterate(_dir, filter);

        if (entry == NULL)
                running = false;

        return entry;
}


/******************************************************************************
 * NON-GENERATIVE 
 *
 * These functions use the directory entry generators, but are not intended
 * to be used as generators themselves. They perform a one-and-done traversal
 * and report a single result with no continuation.
 *
 ******************************************************************************/


/**
 * filecount 
 * `````````
 * Count the number of files in a directory
 *
 * @dir   : open directory stream
 * @filter: a set of predicates (see note on file predicates)
 * Return : the number of files in 'dir' which passed the filter
 *
 * NOTE
 * This is a one-and-done traversal of the directory entries, with
 * a definite sum that is returned to the caller.
 */
int filecount(DIR *dir, int filter)
{
        struct dirent *entry;
        int count = 0;

        while ((entry = diterate(dir, filter))) {
                count++;
        }

        return count;
}


/******************************************************************************
 * GENERATIVE 
 *
 * These are the public generator functions availible for retreiving 
 * information about the set of files (entries) in a directory stream. 
 * They use the directory entry generators outlined above, yielding
 * data about each entry to the caller in a serial fashion.
 *
 ******************************************************************************/


/**
 * getfile
 * ```````
 * Yield the filenames of each entry in a directory stream.
 *
 * @dir   : open directory stream
 * @filter: a set of predicates (see note on file predicates)
 * Return : the filename of the current entry in the iteration
 */
const char *getfile(DIR *dir, int filter)
{
        struct dirent *entry;

        while ((entry = diterate(dir, filter))) {
                return entry->d_name;
        }

        return NULL;
}


/**
 * getdiff
 * ```````
 * Yield the filename of each entry in a directory stream exactly once.
 *
 * @dir   : open directory stream
 * @filter: a set of predicates (see note on file predicates)
 * Return : the filename of the current unique entry in the iteration
 *
 * TODO
 * A mechanism to re-set the Bloom filter.
 *
 * NOTES
 * getdiff() remembers the filenames it has already yielded, even across 
 * multiple continuations. This is not the same as maintaining state
 * between calls, like all generators, but between multiple traversals
 * of the directory entirely. 
 *
 * USAGE
 * getdiff() will only yield those filenames which have changed between 
 * continuations, so that you can repeatedly scan a directory without 
 * re-listing the same entries every time. 
 */
const char *getdiff(DIR *dir, int filter)
{
        static struct bloom_t *bloom;
        struct dirent *entry;

        /* Construct the Bloom filter */
        if (!bloom)
                bloom = bloom_new(250000, 3, fnv_hash, sdbm_hash, djb2_hash);

        while ((entry = diterate(dir, filter))) {
                /* 
                 * If the filename is in the Bloom filter, continue. 
                 */
                if (bloom_check(bloom, entry->d_name))
                        continue;

                /* 
                 * Otherwise, it is probably a new entry. 
                 * Add it to the Bloom filter and yield 
                 * it to the caller 
                 */
                bloom_add(bloom, entry->d_name);

                return entry->d_name;
        }
        return NULL; /* signals end of iteration run */
}


