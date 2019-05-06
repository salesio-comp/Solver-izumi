#include "Solver.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <memory>
#include <map>
#include <chrono>

// 方向を定義
static const int direction[9][2]{
        {0, 0},
        {-1, -1},
        {0,  -1},
        {1,  -1},
        {1,  0},
        {1,  1},
        {0,  1},
        {-1, 1},
        {-1, 0}
};

Solver::Solver(std::vector<std::vector<int>> points, std::vector<std::pair<int, int>> ally_agent, std::vector<std::pair<int, int>> enemy_agent, int turn)
: m_width(points[0].size()), m_height(points.size()), m_points(std::move(points)), m_agent_num(ally_agent.size()), m_engine(std::random_device()())
{
    // エージェントの初期座標マスにタイルを置く
    std::get<0>(m_node) = Board(m_height, std::vector<int>(m_width));
    for (int i = 0; i < m_agent_num; i++){
        std::get<0>(m_node)[ally_agent[i].second][ally_agent[i].first] = 1;
        std::get<0>(m_node)[enemy_agent[i].second][enemy_agent[i].first] = 2;
    }

    // エージェントを格納
    std::get<1>(m_node) = std::move(ally_agent);
    std::get<2>(m_node) = std::move(enemy_agent);

    // ターンを格納
    std::get<3>(m_node) = turn;
}

std::vector<int> Solver::search(const std::vector<int>& ally_action, const std::vector<int>& enemy_action){
    step(m_node, ally_action, enemy_action);

    return monteCarlo();
}

void Solver::debug(const Node& node){
    const auto& [status, ally, enemy, turn] = node;

    std::cout << "turn:" << turn << std::endl;

    // 盤面状態描画
    for (const auto& line : status){
        for (const auto& e : line){
            if (e == 1)
                std::cout << "🔴";
            else if (e == 2)
                std::cout << "🔵";
            else
                std::cout << "⚪";
        }
        std::cout << std::endl;
    }

    // 味方エージェント座標を表示
    for (const auto& e : ally)
        std::cout << "agent🔴:{" << e.first << "," << e.second << "}" << std::endl;
    // 敵エージェント座標を表示
    for (const auto& e : enemy)
        std::cout << "agent🔵:{" << e.first << "," << e.second << "}" << std::endl;

    // タイルポイントを表示
    auto [tile1, tile2] = tilePoint(status);
    std::cout << "tile point🔴:" << tile1 << std::endl;
    std::cout << "tile point🔵:" << tile2 << std::endl;

    // 領域ポイントを表示
    int area1 = areaPoint(status, 1);
    int area2 = areaPoint(status, 2);
    std::cout << "area point🔴:" << area1 << std::endl;
    std::cout << "area point🔵:" << area2 << std::endl;

    // 合計ポイントを表示
    std::cout << "total point🔴:" << area1 + tile1 << std::endl;
    std::cout << "total point🔵:" << area2 + tile2 << std::endl;
}

std::pair<int, int> Solver::tilePoint(const Board& status){
    std::pair<int, int> ret(0, 0);
    for (int i = 0; i < m_height; i++){
        for (int j = 0; j < m_width; j++){
            if (status[i][j] == 1)
                ret.first += m_points[i][j];
            else if (status[i][j] == 2)
                ret.second += m_points[i][j];
        }
    }
    return ret;
}

int Solver::areaPoint(const Board& status, int team){
    // 1回り大きいボードを用意して輪郭を1とする
    Board cache;
    for (int i = 0; i < m_height+2; i++){
        if (i == 0 || i == m_height+1)
            cache.emplace_back(std::vector<int>(m_width+2, 1));
        else{
            std::vector<int> tmp(m_width+2);
            tmp.front() = tmp.back() = 1;
            cache.emplace_back(std::move(tmp));
        }
    }

    // 4方から隣接するマスが1か調べ、タイルが置かれておらず、1だったらそのマスも1になる
    // タイルが置かれていたら2になる
    // 上から
    for (int i = 0; i < m_height; i++){
        for (int j = 0; j < m_width; j++){
            if (cache[i+1][j+1] != 0)
                continue;
            int tile = status[i][j];
            if (tile != team && (cache[i][j+1] == 1 || cache[i+1][j+2] == 1 || cache[i+2][j+1] == 1 || cache[i+1][j] == 1))
                cache[i+1][j+1] = 1;
            else if (tile == team)
                cache[i+1][j+1] = 2;
        }
    }

    // 右から
    for (int j = static_cast<int>(m_width-1); j >= 0; j--){
        for (int i = 0; i < m_height; i++){
            if (cache[i+1][j+1] != 0)
                continue;
            int tile = status[i][j];
            if (tile != team && (cache[i][j+1] == 1 || cache[i+1][j+2] == 1 || cache[i+2][j+1] == 1 || cache[i+1][j] == 1))
                cache[i+1][j+1] = 1;
            else if (tile == team)
                cache[i+1][j+1] = 2;
        }
    }

    // 下から
    for (int i = static_cast<int>(m_height-1); i >= 0; i--){
        for (int j = static_cast<int>(m_width-1); j >= 0; j--){
            if (cache[i+1][j+1] != 0)
                continue;
            int tile = status[i][j];
            if (tile != team && (cache[i][j+1] == 1 || cache[i+1][j+2] == 1 || cache[i+2][j+1] == 1 || cache[i+1][j] == 1))
                cache[i+1][j+1] = 1;
            else if (tile == team)
                cache[i+1][j+1] = 2;
        }
    }

    // 左から
    for (int j = 0; j < m_width; j++){
        for (int i = static_cast<int>(m_height-1); i >= 0; i--){
            if (cache[i+1][j+1] != 0)
                continue;
            int tile = status[i][j];
            if (tile != team && (cache[i][j+1] == 1 || cache[i+1][j+2] == 1 || cache[i+2][j+1] == 1 || cache[i+1][j] == 1))
                cache[i+1][j+1] = 1;
            else if (tile == team)
                cache[i+1][j+1] = 2;
        }
    }

    int sum = 0;
    for (int i = 0; i < m_height; i++){
        for (int j = 0; j < m_width; j++){
            if (cache[i + 1][j + 1] == 0){
                sum += std::abs(m_points[i][j]);
            }
        }
    }

    return sum;
}

void Solver::step(Node& node, const Actions& ally_actions, const Actions& enemy_actions){
    auto& [status, ally, enemy, turn] = node;
    // 1ターン経過
    turn--;

    // 準備
    Agents next_ally(ally);
    Agents next_enemy(enemy);

    // エージェントの行動対象マスを設定
    for (int i = 0; i < m_agent_num; i++){
        next_ally[i].first += direction[(ally_actions[i] - 1) % 8 + 1][0];
        next_ally[i].second += direction[(ally_actions[i] - 1) % 8 + 1][1];

        next_enemy[i].first += direction[(enemy_actions[i] - 1) % 8 + 1][0];
        next_enemy[i].second += direction[(enemy_actions[i] - 1) % 8 + 1][1];
    }

    // コンフリクトを調べる
    for (int i = 0; i < m_agent_num*2; i++){
        Agent& work_i = (i < m_agent_num) ? next_ally[i] : next_enemy[i-m_agent_num];
        for (int j = 0; j < m_agent_num*2; j++){
            if (i == j)
                continue;
            Agent& work_j = (j < m_agent_num) ? next_ally[j] : next_enemy[j-m_agent_num];
            const Agent& before_i = (i < m_agent_num) ? ally[i] : enemy[i-m_agent_num];
            const Agent& before_j = (j < m_agent_num) ? ally[j] : enemy[j-m_agent_num];
            if (work_i == work_j){
                work_i = before_i;
                work_j = before_j;
            }
            else if (Action action_j = (j < m_agent_num) ? ally_actions[j] : enemy_actions[j-m_agent_num]; action_j > 8 && work_i == before_j){
                work_i = before_i;
            }
        }
    }

    // タイルを置く・エージェントを進める・タイル除去
    for (int i = 0; i < m_agent_num; i++){
        if (ally_actions[i] <= 8){
            status[next_ally[i].second][next_ally[i].first] = 1;
            ally[i] = next_ally[i];
        }
        else
            status[next_ally[i].second][next_ally[i].first] = 0;
        if (enemy_actions[i] <= 8) {
            status[next_enemy[i].second][next_enemy[i].first] = 2;
            enemy[i] = next_enemy[i];
        }
        else
            status[next_enemy[i].second][next_enemy[i].first] = 0;
    }
}

Solver::Actions Solver::randomSelect(const Board& status, const Agents& agents, int team){
    std::vector<std::pair<Agent, Action>> agents_actions;
    for (int i = 0; i < m_agent_num; i++) {
        Actions candidate = {1, 2, 3, 4, 5, 6, 7, 8};
        Actions second_candidate;
        bool continue_flag = false;

        // 自分のタイルが置かれていないマスへ行動をする
        while (!candidate.empty()) {
            std::shuffle(candidate.begin(), candidate.end(), m_engine);
            auto x = agents[i].first + direction[candidate.back()][0];
            auto y = agents[i].second + direction[candidate.back()][1];
            // ボード外だったら無し
            if (x < 0 || m_width <= x || y < 0 || m_height <= y)
                candidate.pop_back();
            // 味方同士で同じマスへ行動しようとしていたら無し
            else if (std::any_of(agents_actions.begin(), agents_actions.end(),[x, y](const std::pair<Agent, Action>& pair){return x == pair.first.first + direction[pair.second][0] && y == pair.first.second + direction[pair.second][1];}))
                candidate.pop_back();
            // 行動の対象マスに味方のタイルが置かれていたら保留
            else if (status[y][x] == team) {
                second_candidate.push_back(candidate.back());
                candidate.pop_back();
            }
            // それ以外ならOK
            else {
                agents_actions.emplace_back(agents[i], candidate.back() + ((status[y][x] == 3 - team) ? 8 : 0));
                continue_flag = true;
                break;
            }
        }
        if (continue_flag)
            continue;

        // 仕方ないから自分のタイルが置かれているマスへ行動する
        while (!second_candidate.empty()) {
            std::shuffle(second_candidate.begin(), second_candidate.end(), m_engine);
            auto x = agents[i].first + direction[second_candidate.back()][0];
            auto y = agents[i].second + direction[second_candidate.back()][1];
            // ボード外だったら無し
            if (x < 0 || m_width <= x || y < 0 || m_height <= y)
                second_candidate.pop_back();
            // 味方同士で同じマスへ行動しようとしていたら無し
            else if (std::any_of(agents_actions.begin(), agents_actions.end(),[x, y](const std::pair<Agent, Action>& pair){return x == pair.first.first + direction[pair.second][0] && y == pair.first.second + direction[pair.second][1];}))
                second_candidate.pop_back();
            // それ以外ならOK
            else{
                agents_actions.emplace_back(agents[i], second_candidate.back());
                continue_flag = true;
                break;
            }
        }
        if (continue_flag)
            continue;

        // どこにも移動できない場合は動かない
        agents_actions.emplace_back(agents[i], 0);
    }
    Actions actions;
    for (const auto& e : agents_actions)
        actions.push_back(e.second);
    return actions;
}

int Solver::playOut(Node& node){
    auto& [status, ally, enemy, turn] = node;
    while (turn > 0) {
        step(node, randomSelect(status, ally, 1), randomSelect(status, enemy, 2));

        //debug();
    }

    auto [tile1, tile2] = tilePoint(status);
    auto area1 = areaPoint(status, 1);
    auto area2 = areaPoint(status, 2);

    return (tile1 + area1 > tile2 + area2);
}

Solver::Actions Solver::monteCarlo(){
    auto start = std::chrono::system_clock::now();
    std::map<Actions, int> map;
    Actions ally_actions(m_agent_num);
    Actions enemy_actions(m_agent_num);

    for (int i = 0; i < 2000; i++){
        Node base_node = m_node;

        ally_actions = randomSelect(std::get<0>(base_node), std::get<1>(base_node), 1);
        enemy_actions = randomSelect(std::get<0>(base_node), std::get<2>(base_node), 2);
        step(base_node, ally_actions, enemy_actions);

        map[ally_actions] += playOut(base_node);
        //std::cout << "win:" << map[ally_actions] << std::endl;
        //debug(base_node);
    }

    auto itr = std::max_element(map.begin(), map.end(), [](const std::pair<Actions, int>& a, const std::pair<Actions, int>& b){
        return a.second < b.second;
    });
    std::cout << "best_win:" << itr->second << std::endl;
    auto end = std::chrono::system_clock::now();
    std::cout << "finished:" << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;
    return itr->first;
}

void Solver::drawArea(const Board& status, int team){
    std::cout << "team" << ((team==1) ? "🔴:" : "🔵:") << std::endl;
    // 1回り大きいボードを用意して輪郭を1とする
    Board cache;
    for (int i = 0; i < m_height+2; i++){
        if (i == 0 || i == m_height+1)
            cache.emplace_back(std::vector<int>(m_width+2, 1));
        else{
            std::vector<int> tmp(m_width+2);
            tmp.front() = tmp.back() = 1;
            cache.emplace_back(std::move(tmp));
        }
    }

    // 4方から隣接するマスが1か調べ、タイルが置かれておらず、1だったらそのマスも1になる
    // タイルが置かれていたら2になる
    // 上から
    for (int i = 0; i < m_height; i++){
        for (int j = 0; j < m_width; j++){
            if (cache[i+1][j+1] != 0)
                continue;
            int tile = status[i][j];
            if (tile != team && (cache[i][j+1] == 1 || cache[i+1][j+2] == 1 || cache[i+2][j+1] == 1 || cache[i+1][j] == 1))
                cache[i+1][j+1] = 1;
            else if (tile == team)
                cache[i+1][j+1] = 2;
        }
    }

    // 右から
    for (int j = static_cast<int>(m_width-1); j >= 0; j--){
        for (int i = 0; i < m_height; i++){
            if (cache[i+1][j+1] != 0)
                continue;
            int tile = status[i][j];
            if (tile != team && (cache[i][j+1] == 1 || cache[i+1][j+2] == 1 || cache[i+2][j+1] == 1 || cache[i+1][j] == 1))
                cache[i+1][j+1] = 1;
            else if (tile == team)
                cache[i+1][j+1] = 2;
        }
    }

    // 下から
    for (int i = static_cast<int>(m_height-1); i >= 0; i--){
        for (int j = static_cast<int>(m_width-1); j >= 0; j--){
            if (cache[i+1][j+1] != 0)
                continue;
            int tile = status[i][j];
            if (tile != team && (cache[i][j+1] == 1 || cache[i+1][j+2] == 1 || cache[i+2][j+1] == 1 || cache[i+1][j] == 1))
                cache[i+1][j+1] = 1;
            else if (tile == team)
                cache[i+1][j+1] = 2;
        }
    }

    // 左から
    for (int j = 0; j < m_width; j++){
        for (int i = static_cast<int>(m_height-1); i >= 0; i--){
            if (cache[i+1][j+1] != 0)
                continue;
            int tile = status[i][j];
            if (tile != team && (cache[i][j+1] == 1 || cache[i+1][j+2] == 1 || cache[i+2][j+1] == 1 || cache[i+1][j] == 1))
                cache[i+1][j+1] = 1;
            else if (tile == team)
                cache[i+1][j+1] = 2;
        }
    }

    for (int i = 0; i < m_height; i++){
        for (int j = 0; j < m_width; j++){
            if (cache[i + 1][j + 1] == 0){
                if (team == 1)
                    std::cout << "🔴";
                else if (team == 2)
                    std::cout << "🔵";
            }
            else
                std::cout << "⚪";
        }
        std::cout << std::endl;
    }
}