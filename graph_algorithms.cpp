//
// Created by uw_tliu on 2019-11-22.
//
#include <vector>
#include <list>
#include <queue>
#include <iostream>
#include <fstream>
#include <sstream>
#include "min_vertex_cover.h"
#include "graph_algorithms.h"

vector<vector<int>> AdjacencyList;                                              // Adjacency list
int *color= nullptr;                                                            // see Color enum below
int *predecessor = nullptr;                                                     // Parent vertex of current vertex
int *dist = nullptr;                                                            // Distance from root
extern int vertex_num;

bool build_graph(uint32_t vertexNum, const vector<struct_edge>& edge_lst)
{
    try{
        if (edge_lst.empty())
            return false;
        AdjacencyList.resize(0);
        AdjacencyList.shrink_to_fit();
        AdjacencyList.resize(vertexNum);
        AdjacencyList.shrink_to_fit();

        for (auto & e_num : edge_lst) {
            //Store graph information
            if ( (vertexNum-1) < e_num.p1  || (vertexNum-1) < e_num.p2 ){
                std::cerr<<"Error: edge point beyond limitation " <<  e_num.p1 << " -> " << e_num.p2 << endl;
                return false;
            }
            else {
                AdjacencyList[e_num.p1].push_back(e_num.p2);
                AdjacencyList[e_num.p2].push_back(e_num.p1);
            }
        }
        for(uint32_t i = 0; i < vertexNum; ++i)
        {
            sort(AdjacencyList[i].begin(), AdjacencyList[i].end());
        }
    }
    catch( exception &e){
        std::cout << "Error: " << e.what() << endl;
        return false;
    }
    return true;
}

string approx_vc_1(int vertexNum, vector<struct_edge>& edge_lst){
    uint32_t idx, highest_degree, highest_degree_idx;
    string vertex_cover = "APPROX-VC-1: ";
    list<int> vertex_list;

    while (!edge_lst.empty()) {
        //clog << "APPROX-VC1: edge numbers" << edge_lst.size() << endl;
        highest_degree = 0;
        highest_degree_idx = 0;
        if (!build_graph(vertexNum, edge_lst)) {
            break;
        }
        for (uint32_t idx = 0; idx < AdjacencyList.size(); idx++) {
            if (highest_degree < AdjacencyList.at(idx).size()) {
                highest_degree_idx = idx;
                highest_degree = AdjacencyList.at(idx).size();
            }
        }
        //clog << "APPROX-VC1:  Found Highest degree vertex" << highest_degree_idx << endl;
        vertex_list.push_back(highest_degree_idx);
        //vertex_cover += std::to_string(highest_degree_idx) + ",";
        // Remove all edges incident on highest degree vertex
        while(1) {
            for ( idx = 0; idx < edge_lst.size(); idx++) {
                if (edge_lst.at(idx).p1 == highest_degree_idx || edge_lst.at(idx).p2 == highest_degree_idx) {
                    //clog << "APPROX-VC1: Delete Edge " << edge_lst.at(idx).p1 << "->" << edge_lst.at(idx).p2 << endl;
                    edge_lst.erase(edge_lst.begin() + idx);
                    break;
                }
            }
            if (idx == edge_lst.size()){
                //Try deleted all highest degree vertex incident edges,
                // if not, repeat to delete highest degree vertex incident edges
                break;
            }
        }
    }
    AdjacencyList.clear();
    vertex_list.sort();
    for( auto vertex : vertex_list){
        vertex_cover += to_string(vertex) + ",";
    }
    vertex_cover.erase(vertex_cover.size()-1);
    return vertex_cover;


}

string approx_vc_2(int vertexNum, vector<struct_edge>& edge_lst){
    uint32_t idx, edge_lst_size, random_pick;
    string vertex_cover = "APPROX-VC-2: ";
    uint32_t cur_edge_p1, cur_edge_p2;
    list<int> vertex_list;
    std::ifstream urandom("/dev/urandom");

    while (!edge_lst.empty()) {
        #ifdef PROJ_DEBUG
        clog << "APPROX-VC2: edge numbers" << edge_lst.size() << endl;
        #endif
        edge_lst_size = edge_lst.size();
        urandom.read((char *) &random_pick, sizeof(char));

        random_pick %= edge_lst_size;
        cur_edge_p1 = edge_lst.at(random_pick).p1;
        cur_edge_p2 = edge_lst.at(random_pick).p2;
        vertex_list.push_back(cur_edge_p1);
        vertex_list.push_back(cur_edge_p2);
        while (1) {
            for (idx = 0; idx < edge_lst.size(); idx++) {
                if (edge_lst.at(idx).p1 == cur_edge_p1 || edge_lst.at(idx).p2 == cur_edge_p1 ||
                    edge_lst.at(idx).p1 == cur_edge_p2 || edge_lst.at(idx).p2 == cur_edge_p2) {
                    #ifdef PROJ_DEBUG
                    clog << "APPROX-VC2: Delete Edge " << edge_lst.at(idx).p1 << "->" << edge_lst.at(idx).p2 << endl;
                    #endif
                    edge_lst.erase(edge_lst.begin() + idx);
                    break;
                }
            }
            if (idx == edge_lst.size()) {
                break;
            }
        }
    }

    vertex_list.sort();
    for ( auto vertex : vertex_list){
        vertex_cover += to_string(vertex) + ",";
    }
    vertex_cover.erase(vertex_cover.size()-1);
    return vertex_cover;
}



void BFS_Visit(int s, int vertexNum)
{
    if (color){
        delete[] color;
        color = nullptr;
    }
    color = new int[vertexNum];
    if (predecessor){
        delete[] predecessor;
        predecessor = nullptr;
    }
    predecessor = new int[vertexNum];
    if (dist){
        delete[] dist;
        dist = nullptr;
    }
    dist = new int[vertexNum];

    for(int i = 0; i < vertexNum; ++i)
    {
        color[i] = WHITE;
        predecessor[i] = WHITE;
        dist[i] = NIL;
    }

    try{
        queue<int> q;                           //Create Q
        color[s] = GRAY;                        //discovered this vertex
        dist[s] = 0;                         //discovered this vertex
        q.push(s);                              //ENQUEUE(Q,s)
        while(!q.empty())                       //while Q!= 0
        {
            int u = q.front();                  // do u <- DEQUEUE(Q)
            int count = AdjacencyList[u].size();
            for(int i = 0; i < count; ++i)      //for each v in Adj[u]
            {
                int v = AdjacencyList[u][i];
                if(color[v] == WHITE)         //if color[v] == white
                {                             //then
                    color[v] = GRAY;          //color[v] <- GRAY
                    predecessor[v] = u;       //Pi[v] <- u
                    dist[v] = dist[u] + 1;    //d[v] <- d[u] + 1
                    q.push(v);                //ENQUEUE(Q,v)
                }
            }
            color[u] = BLACK;                 //color[u] <- BLACK
            q.pop();
        }
    }
    catch (exception &e){
        std::cout << "Error: " << e.what() << endl;
    }
#ifdef DEBUG
    for(int j = 0; j < vertexNum; ++j)
    {
        std::cout << "dist" << j << "==" << dist[j] << endl;
    }
#endif
}

void output_shortest_path(int s, int v)
{
    if (predecessor == nullptr){
        std::cerr<<"Error: Path not exist!";
        return;
    }

    if( s == v ){
        cout<< s ;
    }
    else if (predecessor[v] == WHITE)
    {
        std::cerr<<"Error: Path not exist between " <<  s << " and " << v;
        return;
    }
    else{
        output_shortest_path(s, predecessor[v]);
        std::cout<< "-" << v;
    }
}

bool parse_command(std::string &input_str, char &command, int &arg, std::string &err_msg){
    char ch = 0;
    std::istringstream input(input_str);

    ch = input.peek();

    if ( ch == EOF ) {
        err_msg = "Empty command";
        return false;
    }
    else{

        input >> ch;

        if (input.fail()) {
            err_msg = "Failed to read input";
            return false;
        }

        if (ch == 'V'){
#ifdef ANALYSIS
            std::cout << input_str << std::endl;
#endif
            if ( input.peek() != ' '){
                err_msg = "bad argument";
                return false;
            }
            int num;
            input >> num;
            if (input.fail()) {
                err_msg = "Missing or bad argument";
                return false;
            }
            ws(input);
            if (!input.eof()) {
                err_msg = "Unexpected argument";
                return false;
            }
            command = ch;
            arg = num;
            return true;
        }
        else if (ch == 'E' ) {
#ifdef ANALYSIS
            std::cout << input_str << std::endl;
#endif
            command = ch;
            return true;
        }
        else if (ch == 's') {
            command = ch;
            return true;
        }
        else{
            err_msg = "Unexpected argument";
            return false;
        }
    }

}

bool parsing_edge_list(string &edge_string, vector<struct_edge> &edge_lst, string &edge_err_msg){
    bool ret_val = true;
    string data_process_str;
    istringstream input(edge_string);
    char ch;

    // Check Edge list format {..}
    input >> ch;
    if ( ch != '{'){
        edge_err_msg = "Unexpected argument";
        return false;
    }
    while ( ( ch = input.peek() ) != '}'){
        input >> ch;
        if ( ch == '\n' || ch == EOF){
            edge_err_msg = "Unexpected argument, missing }";
            ret_val = false;
            break;
        }
        else{
            data_process_str += ch;
        }
    }

    // check string inside of { } is not empty
    if ( data_process_str.empty()){ // empty string within { }
        edge_err_msg = "Unexpected argument, empty between { }";
        ret_val = false;
    }

    // Non-empty string within { }
    if ( data_process_str[0] != '<' ){
        edge_err_msg = "Unexpected argument, missing < at head of string";
        ret_val = false;
    }
    else{               // parsing edge coordinate
        size_t pos = 0;
        std::string delimiter = "<";
        vector<std::string> token_lst;
        int token_list_len;
        string token, comma, p1, p2;
        try {
            while ((pos = data_process_str.find(delimiter)) != std::string::npos) {
                token = data_process_str.substr(0, pos);
                token_lst.push_back(token);
#ifdef DEBUG
                std::cout << token << std::endl;
#endif
                data_process_str.erase(0, pos + delimiter.length());
#ifdef DEBUG
                std::cout << "data_process_str =" << data_process_str << std::endl;
#endif
            }
            if (!data_process_str.empty()){ // push remanding part to to back of list
                token_lst.push_back(data_process_str);
            }
            token_list_len = token_lst.size();
            delimiter = ">";
#ifdef DEBUG
            std::cout << "Total " << token_list_len << " tokens" << std::endl;
#endif
            for (int idx =0; idx < token_list_len; idx++){
                token = token_lst[idx];
                pos = token.find('>');
                if ( pos != string::npos ){
                    token[pos] = ',';
#ifdef DEBUG
                    std::cout << "token =" << token << std::endl;
#endif
                    comma = ',';
                    pos = token.find(comma);
#ifdef DEBUG
                    std::cout << "P1 comma at " << pos  << std::endl;
#endif
                    p1 = token.substr(0, pos);
                    token.erase(0, pos + comma.length());
                    pos = token.find(comma);
#ifdef DEBUG
                    std::cout << "P2 comma at " << pos  << std::endl;
#endif
                    p2 = token.substr(0, pos);
#ifdef DEBUG
                    std::cout << "P1 =" << p1 << ", P2 = " << p2  << std::endl;
#endif
                    auto *edge = new struct_edge;

                    int a1 = std::stoi(p1,nullptr);
                    int a2 = std::stoi(p2,nullptr);

                    if ( a1 < 0 || a1 >= vertex_num ){
                        edge_err_msg = "P1 out of range ";
                        edge_err_msg += p1;
                        edge_err_msg += ",";
                        edge_err_msg += p2;
                        ret_val = false;
                        break;
                    }
                    if ( a2 < 0 || a2 >= vertex_num ){
                        edge_err_msg = "P2 out of range ";
                        edge_err_msg += p1;
                        edge_err_msg += ",";
                        edge_err_msg += p2;
                        ret_val = false;
                        break;
                    }
                    if ( ret_val ){
                        edge->p1 = a1;
                        edge->p2 = a2;
                        edge_lst.push_back(*edge);
                    }
                }
            }
        }
        catch (exception& e)
        {
            edge_err_msg = e.what();
            ret_val = false;
        };

    }
    return ret_val;
}


void destroy_graph(){
    //AdjacencyList.resize(0);
    //AdjacencyList.shrink_to_fit();
    if (predecessor) {
        delete[] predecessor;
    }
    if (color) {
        delete[] color;
    }
    if ( dist ) {
        delete[] dist;
    }
}
