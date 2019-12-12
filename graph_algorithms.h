//
// Created by uw_tliu on 2019-11-22.
//

#include "min_vertex_cover.h"
#ifndef ECE650_PROJECT_GRAPH_FUNC_H
#define ECE650_PROJECT_GRAPH_FUNC_H
#define NIL (-1)
enum eCOLOR{
    WHITE = -2,
    GRAY = -3,
    BLACK = -4
};

bool build_graph(uint32_t vertexNum, const vector<struct_edge>& edge_lst);
void BFS_Visit(int s, int vertexNum);
void output_shortest_path(int s, int v);
bool parsing_edge_list(string &edge_string, vector<struct_edge> &edge_lst, string &edge_err_msg);
bool parse_command(std::string &input_str, char &command, int &arg, std::string &err_msg);
void destroy_graph();
string approx_vc_1(int vertexNum, vector<struct_edge>& edge_lst);
string approx_vc_2(int vertexNum, vector<struct_edge>& edge_lst);

#endif //ECE650_PROJECT_GRAPH_FUNC_H
