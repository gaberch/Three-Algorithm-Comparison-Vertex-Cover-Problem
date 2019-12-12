//
// Created by tliu on 2019-11-25.
//
#include <sstream>
#include <string>
#include<vector>
#include<algorithm>
#include<queue>
#include <pthread.h>
#include <semaphore.h>
#include "timer.h"
#include "min_vertex_cover.h"
#include "graph_algorithms.h"
#include "thread_functions.h"

int vertex_num = 0;
volatile bool program_live = true;
pthread_mutex_t std_output_mutex = PTHREAD_MUTEX_INITIALIZER;

min_vertex_cover *new_cover = nullptr;
static string edge_str_vc1 = "";
static string edge_str_vc2 = "";
static string edge_str;
extern sem_t input_ready_sem[THREAD_NUM];
extern sem_t min_vc_done;
extern sem_t approx_vc_done[2];
extern pthread_t thread_id[THREAD_NUM];
extern clockid_t cid[THREAD_NUM];


/*
 * String array to store three Algorithm's result
 * vc_output[0]: CNF-SAT-VC
 * vc_output[1]: APPROX-VC-1
 * vc_output[2]: APPROX-VC-2
*/
string vc_output[THREAD_NUM] = {"\0","\0","\0"};

/*
 * Function prototype
 */
long get_pclock_in_ns(clockid_t cid);
void output_cpu_time(long start_time, long end_time, int id);

/*
 * Customized Timer For ECE650 Fall 2019 Project
*/

class ECE650Timer : public CppTimer {

    void timerEvent() {
        stop();
        vc_output[0] = "CNF-SAT-VC: Time Out\0";
        sem_post(&min_vc_done);
        pthread_kill(thread_id[0], SIGINT);
    }
};

static void interruptFn (int sig)
{
    if (new_cover){
        new_cover->stop_solver();
    }
    pthread_cancel(thread_id[0]);
}

void *run_min_vertex_cover_thread(void *pVoid){
    vector<struct_edge> edge_list;
    string err_msg;
    long timeout_interval = 120000000;       // us
    long start_time, end_time;
    int sig = SIGINT;
    sigset_t sigSet;
    sigemptyset(&sigSet);
    sigaddset(&sigSet, sig);

    pthread_sigmask(SIG_UNBLOCK, &sigSet, NULL);

    struct sigaction act;
    act.sa_handler = interruptFn;
    sigaction(sig, &act, NULL);
    sem_wait(&input_ready_sem[0]);
    vc_output[0].clear();
    if (parsing_edge_list(edge_str, edge_list, err_msg)) {
            new_cover = new min_vertex_cover(edge_list, vertex_num);
            if (new_cover) {
                ECE650Timer *myTimer1 = new ECE650Timer();
                myTimer1->start(timeout_interval);
                start_time = get_pclock_in_ns(cid[0]);
                new_cover->find_min_vertex_cover();
                end_time = get_pclock_in_ns(cid[0]);
                myTimer1->stop();
                delete myTimer1;
#ifdef ANALYSIS
                output_cpu_time(start_time, end_time, 0);
#endif
                vc_output[0] = new_cover->get_output();
                sem_post(&min_vc_done);
            }
            delete new_cover;
    } else {
        vc_output[0] = err_msg;
    }
    vector<struct_edge>().swap(edge_list);
    return 0;
}

void approx_vc_func(string &err_msg, string &edge_str, vector<struct_edge> &edge_list, int func_id) {
    long start_time, end_time;

    sem_wait(&input_ready_sem[func_id]);
    err_msg.erase(err_msg.begin(), err_msg.end());
    vc_output[func_id].clear();

    if (parsing_edge_list(edge_str, edge_list, err_msg)) {
        start_time = get_pclock_in_ns(cid[func_id]);
        if ( func_id == 1) {
            vc_output[1] = approx_vc_1(vertex_num, edge_list);
        }
        else if (func_id == 2) {
            vc_output[func_id] = approx_vc_2(vertex_num, edge_list);
        }
        end_time = get_pclock_in_ns(cid[func_id]);
#ifdef ANALYSIS
        output_cpu_time(start_time, end_time, func_id);
#endif
    } else {
        vc_output[func_id] = err_msg;
    }
    sem_post(&approx_vc_done[func_id-1]);
}

void *run_approx_vc_1_thread(void *pVoid){
    string err_msg;
    vector<struct_edge> edge_list_vc1;
    while(1){
        approx_vc_func(err_msg, edge_str_vc1, edge_list_vc1, 1);
        vector<struct_edge>().swap(edge_list_vc1);
    }
    return 0;
}


void *run_approx_vc_2_thread(void *pVoid){
    string err_msg;
    vector<struct_edge> edge_list_vc2;
    while (1) {
        approx_vc_func(err_msg, edge_str_vc2, edge_list_vc2, 2);
        vector<struct_edge>().swap(edge_list_vc2);
    }
    return 0;
}

void *run_io_thread(void* pVoid){
    int status;

    while (!std::cin.eof()) {
        // read a line of input until EOL and store in a string
        std::string readin_string;
        std::getline(std::cin, readin_string);

        // if nothing was read, go to top of the while to check for eof
        if (readin_string.empty()) {
            if (std::cin.eof()) {
                break;
            }
            std::cerr << "Error: No input" << endl;
            continue;
        }

        char cmd = 0;
        int arg = 0;
        std::string err_msg;
        std::string cmd_str;
        if (parse_command(readin_string, cmd, arg, err_msg)) {
            switch (cmd) {
                case 'V':
                    vertex_num = arg;
                    break;
                case 'E': {
                    size_t pos = readin_string.find("E ");
                    if (pos == string::npos) {
                        std::cerr << "Error: Invalid input " << readin_string << endl;
                    }
                    edge_str_vc1 = readin_string.substr(pos + 1);
                    edge_str_vc2 = readin_string.substr(pos + 1);
                    edge_str = readin_string.substr(pos + 1);
                    pthread_create( &thread_id[0], NULL, run_min_vertex_cover_thread, NULL);
                    status = pthread_getcpuclockid(thread_id[0], &cid[0]);
                    if (status != 0) {
                        cerr << "ERROR: get run_min_vertex_cover_thread cpu clock id failed" << endl;
                    }
                    sem_post(&input_ready_sem[0]);
                    sem_post(&input_ready_sem[1]);
                    sem_post(&input_ready_sem[2]);

                    sem_wait(&min_vc_done);
                    pthread_mutex_lock(&std_output_mutex);
                    cout << vc_output[0] << endl;
                    pthread_mutex_unlock(&std_output_mutex);
                    sem_wait(&approx_vc_done[0]);
                    pthread_mutex_lock(&std_output_mutex);
                    cout << vc_output[1] << endl;
                    pthread_mutex_unlock(&std_output_mutex);
                    sem_wait(&approx_vc_done[1]);
                    pthread_mutex_lock(&std_output_mutex);
                    cout << vc_output[2] << endl;
                    pthread_mutex_unlock(&std_output_mutex);

                }
                    break;
                case 's': {
                    int arg1, arg2;
                    string arg_str;
                    try {
                        size_t pos = readin_string.find("s ");
                        if (pos == string::npos) {
                            std::cerr << "Error: Invalid input " << readin_string << endl;
                            break;
                        }
                        else if (pos != 0){
                            std::cerr << "Error: Invalid input " << '\"' << readin_string << '\"'<< " didn't start from \'s\' "<< endl;
                            break;
                        }

                        cmd_str = readin_string.substr(2, readin_string.length()-2);
                        if ((pos = cmd_str.find(' ')) != std::string::npos) {
                            arg_str = cmd_str.substr(0, pos);
                            cmd_str.erase(0, pos);
                            arg1 = stoi(arg_str);
                            arg_str = cmd_str;
                            arg2 = stoi(arg_str);
                            if (arg1 < 0 || arg2 < 0){
                                std::cerr << "Error: Unexpected Negative Number " << arg1 << "," << arg2 << endl;
                                break;
                            }
                            if ( arg1 >= vertex_num || arg2 >= vertex_num ){
                                std::cerr << "Error: Unexpected Argument " << arg1 << "-" << arg2 << endl;
                                break;
                            }
                            BFS_Visit(arg1, vertex_num);
                            if ( arg1 != arg2){
                                output_shortest_path(arg1, arg2);
                            }
                            else{
                                std::cout << arg1 << "-" << arg2;
                            }
                            std::cout << endl;
                        }
                        else{
                            std::cerr << "Error: Unexpected single Argument " << cmd_str << endl;
                        }
                    }
                    catch (exception& e)
                    {
                        std::cerr << "Error: exception for " << e.what() << endl;
                    };
                }
                    break;
                default:
                    std::cerr << "Error: Invalid input " << readin_string << endl;
                    break;
            }
        }
        else {
            std::cerr << "Error: " << err_msg << endl;
        }
        usleep(1000);
    }
    return 0;
}


void output_cpu_time(long start_time, long end_time, int id) {
    long diff = end_time - start_time;
    int us =  diff / 1000;
    int ns =  diff % 1000;
    char output_line[256];
    pthread_mutex_lock(&std_output_mutex);
    if (id == 0){
        sprintf(output_line,"CNF-SAT-VC Duration: %d.%03d us", us, ns);
        cout << output_line << endl;
    }
    else {
        sprintf(output_line,"APPROX-VC-%d Duration: %d.%03d us", id, us, ns);
        cout << output_line << endl;
    }
    pthread_mutex_unlock(&std_output_mutex);
}

long get_pclock_in_ns(clockid_t cid)
{
    struct timespec ts;
    long ns = 0;

    if (clock_gettime(cid, &ts) == -1) {
        cerr << "ERROR: clock_gettime" << endl;
        return -1;
    }
    ns = ts.tv_sec * 1000000000 + ts.tv_nsec;
    return ns;
}
