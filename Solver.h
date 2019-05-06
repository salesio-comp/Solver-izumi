#ifndef SOLVER2_SOLVER_H
#define SOLVER2_SOLVER_H

#include <vector>
#include <random>

class Solver {
public:
    // コンストラクタ
    Solver(std::vector<std::vector<int>> points, std::vector<std::pair<int, int>> ally_agent, std::vector<std::pair<int, int>> enemy_agent, int turn);
    // 探索実行
    std::vector<int> search(const std::vector<int>& ally_action, const std::vector<int>& enemy_action);

//private:
    using Board = std::vector<std::vector<int>>;            // ボード
    using Agent = std::pair<int, int>;                      // エージェントの座標
    using Agents = std::vector<Agent>;                      // エージェントの配列
    using Node = std::tuple<Board, Agents, Agents, int>;    // 1ターンの盤面情報:タイル情報,味方エージェント情報,敵エージェント情報,残りターン
    using Action = int;                                     // 行動種類
    using Actions = std::vector<Action>;                    // エージェント一人一人に対する行動情報

    // 味方と敵のタイルポイントを返す
    std::pair<int, int> tilePoint(const Board& status);
    // 領域ポイントを返す
    int areaPoint(const Board& status, int team);
    // 1ターン進める
    void step(Node& node, const Actions& ally_actions, const Actions& enemy_actions);
    // ランダムに行動を選ぶ
    Actions randomSelect(const Board& status, const Agents& agents, int team);
    // プレイアウトする
    int playOut(Node& node);
    // モンテカルロモドキ
    Actions monteCarlo();
    // デバッグ表示用
    void debug(const Node& node);
    // 領域を表示
    void drawArea(const Board& status, int team);

    const size_t m_width;       // 幅
    const size_t m_height;      // 高さ
    const size_t m_agent_num;   // エージェントの数
    const Board m_points;       // 各マスの得点の情報
    Node m_node;                // 現在の盤面状況
    std::mt19937 m_engine;      // 乱数生成器
};


#endif //SOLVER2_SOLVER_H
