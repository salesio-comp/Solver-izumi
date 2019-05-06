#include <iostream>
#include <vector>
#include <future>
#include <thread>
#include "Solver.h"
using namespace std;

int main() {
    vector<vector<int>> points = {
            {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1},
            {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2},
            {3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3},
            {4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4},
            {5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5},
            {6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6},
            {7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7},
            {8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8},
            {9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9},
            {10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10},
            {10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10},
            {9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9},
            {8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8},
            {7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7},
            {6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6},
            {5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5},
            {4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4},
            {3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3},
            {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2},
            {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1}
    };

    vector<pair<int, int>> ally = {
            {2, 2},
            //{7, 2},
            //{2, 7},
            //{7, 7},
            //{12, 12},
            //{17, 12},
            //{12, 17},
            {17, 17}
    };

    vector<pair<int, int>> enemy = {
            //{12, 2},
            {17, 2},
            //{12, 7},
            //{17, 7},
            //{2, 12},
            //{7, 12},
            {2, 17},
            //{7, 17}
    };

    int turn = 40;

    Solver s1(points, ally, enemy, turn);
    Solver s2(points, enemy, ally, turn);
    s1.debug(s1.m_node);

    Solver::Actions ally_actions(8);
    Solver::Actions enemy_actions(8);

    while (turn--){
        auto result1 = async(launch::async, [&s1, &ally_actions, &enemy_actions]{return s1.search(ally_actions, enemy_actions);});

        auto result2 = async(launch::async, [&s2, &enemy_actions, &ally_actions]{return s2.search(enemy_actions, ally_actions);});
        /*this_thread::sleep_for(chrono::seconds(5));
        cout << "5 seconds passed" << endl;*/

        ally_actions = result1.get();
        enemy_actions = result2.get();

        s1.debug(s1.m_node);
    }

    s1.drawArea(get<0>(s1.m_node), 1);
    s1.drawArea(get<0>(s1.m_node), 2);
    //s.monteCarlo();

    return 0;
}