//
// Created by tliu on 2019-11-25.
//

#ifndef ECE650_PROJECT_THREAD_FUNCTIONS_H
#define ECE650_PROJECT_THREAD_FUNCTIONS_H

#include "min_vertex_cover.h"
#define THREAD_NUM 4
void *run_approx_vc_1_thread(void *pVoid);
void *run_approx_vc_2_thread(void *pVoid);
void *run_io_thread(void* pVoid);




#endif //ECE650_PROJECT_THREAD_FUNCTIONS_H
