#ifndef _DIR_LISTING_H
#define _DIR_LISTING_H


/* Procedures 
``````````````````````````````````````````````````````````````````````````````*/
int filecount(DIR *dir, int options);


/* Generators
``````````````````````````````````````````````````````````````````````````````*/
const char *getfile(DIR *dir, int options);
const char *getdiff(DIR *dir, int filter);


#endif
