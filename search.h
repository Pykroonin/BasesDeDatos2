/*
 Created by roberto on 3/5/21.
*/

#ifndef NCOURSES_SEARCH_H
#define NCOURSES_SEARCH_H
#include "windows.h"
#include <string.h>
/*#include <unistd.h>*/
void results_search(char * from, char *to, char * date, int * n_choices,
                    char *** choices, char *** choices_info, int max_length, int max_rows);

#endif /*NCOURSES_SEARCH_H*/
