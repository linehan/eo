#ifndef _PITHY_PTHREAD_H
#define _PITHY_PTHREAD_H

#include <pthread.h>

typedef void *(*pth_function)(void *);
typedef pthread_t           pth_thread;
typedef pthread_attr_t      pth_attr;
typedef pthread_mutex_t     pth_mutex;
typedef pthread_mutexattr_t pth_mutex_attr;


struct pth_t {
        pth_thread     thread;
        pth_attr       *attr; 
        pth_function   func;
        pth_mutex      mutex;
        pth_mutex_attr *mutex_attr;
};


static inline void init_pth(struct pth_t *pth, pth_function func)
{
        pth->attr       = NULL;
        pth->mutex_attr = NULL;
        pth->func       = func;

        pthread_mutex_init(&pth->mutex, pth->mutex_attr);
}


static inline void pth_fork(struct pth_t *pth, void *arg)
{
        pthread_create(&pth->thread, pth->attr, pth->func, arg);
}


static inline void pth_exit(struct pth_t *pth, void *retval)
{
        pthread_exit(retval);
}




static inline int pth_lock(pth_mutex *mutex) 
{
        return pthread_mutex_lock(mutex);
}

static inline int pth_unlock(pth_mutex *mutex)
{
        return pthread_mutex_unlock(mutex); 
}

/* Returns EBUSY if busy */
static inline int pth_trylock(pth_mutex *mutex)
{
        return pthread_mutex_trylock(mutex);
}


#endif
