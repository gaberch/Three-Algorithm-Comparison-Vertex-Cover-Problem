//
// Created by tliu on 2019-11-15.
//

#ifndef ECE650_A4_MIN_VERTEX_COVER_H
#define ECE650_A4_MIN_VERTEX_COVER_H

#include <iostream>
#include <vector>
#include <minisat/core/SolverTypes.h>
#include <minisat/core/Solver.h>
#include "minisat/utils/System.h"
#include <algorithm>
#include <chrono>
#include <memory>
//#define DEBUG_V
using namespace std;
using namespace Minisat;
typedef struct STRUCT_EDGE{
    uint32_t p1;
    uint32_t p2;

} struct_edge;

//0 is UNSAT, 1 is SAT
enum eResult{
    UNDEF = -1,
    UNSAT= 0,
    SAT = 1
};

class min_vertex_cover {
    int vertices_num;
    vector<struct_edge> &edge_list;
    std::unique_ptr<Minisat::Solver> solver;
public:
    // constructor
    min_vertex_cover(vector<struct_edge> &edges, uint32_t v_num = 0) : vertices_num(v_num), edge_list(edges) {
        solver.reset(new Minisat::Solver());
    }
    vector<vector<Lit>> Literal_list;
    string out_str = "CNF-SAT-VC: ";

    string get_output(){return out_str;}

    /*
     *  At least one vertex is the ith vertex in the vertex cover
     */
    void at_least_one_vertex_in_cover(Solver &solver, int k) {
        Lit lit;
        vec<Lit> clause;
        for (int i = 0; i < k; i++) {
            for (int n = 0; n < vertices_num; n++) {
                lit = Literal_list[n][i];
                clause.push(lit);
            }
            solver.addClause(clause);
            clause.clear();
        }
    }


    /*
    * No one vertex can appear twice in a vertex cover
    */
    void no_vertex_appear_twice(Solver &solver, int k) {
        Lit Xmp, Xmq;
        for (int m = 0; m < vertices_num; m++) {
            for (int q = 0; q < k; q++) {
                for (int p = 0; p < q; p++) {
                    Xmp = Literal_list[m][p];
                    Xmq = Literal_list[m][q];
                    solver.addClause(~Xmp, ~Xmq);
                }
            }
        }
    }


    /*
     * No more than one vertex appears in the mth position of the vertex cover
     */
    void no_morethan_one_vertex_in_m_pos(Solver &solver, int k) {
        Lit Xpm, Xqm;
        for (int m = 0; m < k; m++) {
            for (int q = 0; q < vertices_num; q++) {
                for (int p = 0; p < q; p++) {
                    Xpm = Literal_list[p][m];
                    Xqm = Literal_list[q][m];
                    solver.addClause(~Xpm, ~Xqm);
                }
            }
        }
    }


    /*
     * Every edge is incident to at least one vertex
     */
    void every_edge_atleast_one_vertex(Solver &solver, int k) {
        Lit X_ik, X_jk;
        for (auto &edge : edge_list) {
            vec<Lit> literals;
            for (int i = 0; i < k; i++) {
                X_ik = Literal_list[edge.p1][i];
                literals.push(X_ik);
            }
            for (int j = 0; j < k; j++) {
                X_jk = Literal_list[edge.p2][j];
                literals.push(X_jk);
            }
            solver.addClause(literals);
        }
    }


    bool solve(Solver &solver, int k) {
        // reduction consists following clauses
        Literal_list.clear();
        Literal_list.resize(vertices_num * k);

        for (int n = 0; n < vertices_num; n++) {
            for (int i = 0; i < k; i++) {
                auto var = solver.newVar();
                Literal_list[n].push_back(mkLit(var, false));
            }
        }

        at_least_one_vertex_in_cover(solver, k);
        no_vertex_appear_twice(solver, k);
        no_morethan_one_vertex_in_m_pos(solver, k);
        every_edge_atleast_one_vertex(solver, k);
#ifdef DEBUG_V
        std::clog << " Number of Clauses = " << solver.nClauses() << endl;
#endif


        //auto sat = solver.solve();
        solver.clearInterrupt();
        auto sat = solver.solve();
        return sat;
    }

    vector<int> get_cover(Solver &solver, int k) {
        vector<int> path;
        for (int n = 0; n < vertices_num; n++) {
            for (int idx = 0; idx < k; idx++) {
                if (solver.modelValue(Literal_list[n][idx]) == l_True) {
                    path.push_back(n);
                }
            }
        }
        sort(path.begin(), path.end());
        return path;
    }

    void print_output(std::vector<int> vertex_cover_path) {
        for (auto vertex : vertex_cover_path){
            out_str += std::to_string(vertex);
            out_str += ",";
            //cout << vertex << " ";
        }
        out_str.erase(out_str.size()-1);
        //cout << endl;
    }

    void stop_solver(){
        solver->interrupt();
    }

    //void SIGINT_interrupt(int) { solver->interrupt(); }

    string find_min_vertex_cover() {
        string ret_str = "";
        int *solver_return_val = nullptr;
        solver_return_val = new int[vertices_num];
        vector<int> vertex_cover_paths[vertices_num];
        fill_n(solver_return_val, vertices_num, UNDEF);
#ifdef BINARY_SEARCH
        int low = 0;
        int high = vertices_num;
        int cur;

        std::fill_n(solver_return_val, vertices_num, UNDEF);
#ifdef DEBUG_V
       solver->verbosity = 1;
#endif
        try {
            while (low <= high) {
                cur = (high + low) / 2;
                std::cout << "Trying K = " << cur << endl;
                auto start = std::chrono::system_clock::now();
                solver_return_val[cur] = solve(*solver, cur);
                auto end = std::chrono::system_clock::now();
                std::chrono::duration<double> diff = end - start;
                std::cout << " Result: " << solver_return_val[cur] << " Duration=" << diff.count() << std::endl;

                if (solver_return_val[cur]) {
                    vertex_cover_paths[cur] = get_cover(*solver, cur);
                }

                if (solver_return_val[cur] == SAT &&
                    solver_return_val[cur - 1] == 0 &&
                    cur != UNSAT)
                {
                    print_output(vertex_cover_paths[cur]);
                    break;
                }

                if (solver_return_val[cur] == UNSAT &&
                    solver_return_val[cur + 1] == SAT &&
                    cur != vertices_num)
                {
                    print_output(vertex_cover_paths[cur + 1]);
                    break;
                }

                if (solver_return_val[cur]) {
                    high = cur - 1;
                } else {
                    low = cur + 1;
                }
                solver.reset(new Minisat::Solver());
            }
        }
        catch (exception e){
            cerr << strerror(errno);
        }

#else
        for (int i = 0; i < vertices_num; i++) {
            pthread_testcancel();
#ifdef DEBUG_V
            std::cout << "Trying K= " << i << endl;
#endif
#ifdef DEBUG_V
            solver->verbosity = 1;
#endif
            solver_return_val[i] = solve(*solver, i);
            if (solver_return_val[i] == SAT) {
#ifdef DEBUG_V
                cout << "\nFound cover " << i << endl;
#endif
                vertex_cover_paths[i] = get_cover(*solver, i);
                print_output(vertex_cover_paths[i]);
                break;
            } else if (solver_return_val[i] == UNSAT) {
#ifdef DEBUG_V
                std::cout << " result = " << solver_return_val[i] << endl;
#endif
            } else {
#ifdef DEBUG_V
                std::cout << " result = " << solver_return_val[i] << endl;
#endif
            }
#ifdef DEBUG_V
            cout << "Duration: " << period.count() << " seconds "<< endl;
#endif
            solver.reset(new Minisat::Solver());
        }
#endif
        delete[] solver_return_val;
        return ret_str;
    }

};


#endif //ECE650_A4_MIN_VERTEX_COVER_H