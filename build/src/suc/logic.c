#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <signal.h>

#include "meta.h"
#include "../common/io/file.h"
#include "../common/ipc/daemon.h"
#include "../common/ipc/channel.h"

#include "../common/error.h"
#include "../common/util.h"
#include "../common/configfiles.h"
#include "../common/textutils.h"
#include "../common/lib/bloom/bloom.h"


char LOGIC[20000];



