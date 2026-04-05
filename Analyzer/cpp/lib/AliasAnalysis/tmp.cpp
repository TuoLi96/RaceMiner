#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <set>

void AliasGraph::compact(UnionFind<AGNode *> &uf) {
    using namespace std;

    // =========================================================
    // Step 0. 确保图中所有节点都已加入并查集
    // =========================================================
    for (AGNode *node : agnode_set) {
        uf.add(node);
    }

    // =========================================================
    // Step 1. worklist 初始化
    //
    // 初始所有已有等价类都可能触发传播：
    // 如果同一类中的多个节点在相同 offset 上有出边，
    // 那么这些出边目标节点也要被合并。
    // =========================================================
    queue<AGNode *> worklist;
    unordered_set<AGNode *> in_worklist;

    for (AGNode *rep : uf.getRoots()) {
        rep = uf.find(rep);
        if (!in_worklist.count(rep)) {
            worklist.push(rep);
            in_worklist.insert(rep);
        }
    }

    // =========================================================
    // Step 2. worklist 传播闭包
    //
    // 对于一个类 S：
    //   - 收集该类中所有节点的出边
    //   - 按 offset 分组
    //   - 如果某个 offset 下有多个目标节点，则这些目标节点要合并
    //   - 新形成的类继续入队传播
    // =========================================================
    while (!worklist.empty()) {
        AGNode *rep = worklist.front();
        worklist.pop();

        rep = uf.find(rep);
        in_worklist.erase(rep);

        const auto &members = uf.getMembers(rep);

        // 单节点类通常不会触发“同 offset 多目标合并”，但保留统一逻辑即可
        unordered_map<int, vector<AGNode *>> offset2dsts;

        for (AGNode *node : members) {
            const auto &out_edges = node->getOutEdges();

            for (const auto &[offset, edge] : out_edges) {
                if (!edge) continue;

                AGNode *dst = edge->getDst();
                if (!dst) continue;

                offset2dsts[offset].push_back(dst);
            }
        }

        // 对每个 offset 下的目标节点进行合并
        for (auto &[offset, dsts] : offset2dsts) {
            if (dsts.size() <= 1) {
                continue;
            }

            AGNode *base_rep = uf.find(dsts[0]);

            for (size_t i = 1; i < dsts.size(); ++i) {
                AGNode *cur_rep = uf.find(dsts[i]);
                base_rep = uf.find(base_rep);

                if (base_rep == cur_rep) {
                    continue;
                }

                AGNode *new_rep = uf.unite(base_rep, cur_rep);
                new_rep = uf.find(new_rep);

                // 新合并出来的类可能继续触发传播
                if (!in_worklist.count(new_rep)) {
                    worklist.push(new_rep);
                    in_worklist.insert(new_rep);
                }

                base_rep = new_rep;
            }
        }
    }

    // =========================================================
    // Step 3. 根据最终并查集结果，创建压缩后的新节点
    //
    // 每个最终等价类 -> 一个新 AGNode
    // 并把原来类中的 alias_set 全部合并进去
    // =========================================================
    unordered_map<AGNode *, AGNode *> rep2newnode;

    for (AGNode *old_node : agnode_set) {
        AGNode *rep = uf.find(old_node);

        if (rep2newnode.find(rep) == rep2newnode.end()) {
            rep2newnode[rep] = new AGNode();
        }

        AGNode *new_node = rep2newnode[rep];
        for (llvm::Value *val : old_node->getAliasSet()) {
            new_node->insertVal(val);
        }
    }

    // =========================================================
    // Step 4. 重建压缩后的边
    //
    // 原图中的每条边:
    //   old_src --offset--> old_dst
    //
    // 在压缩图中变成:
    //   new(src_rep) --offset--> new(dst_rep)
    //
    // 注意：
    //   同一条压缩边（相同 src, dst, offset）只保留一条
    // =========================================================
    set<AGNode *> new_agnode_set;
    set<AGEdge *> new_agedge_set;
    llvm::DenseMap<llvm::Value *, AGNode *> new_val2node;

    for (auto &[rep, new_node] : rep2newnode) {
        new_agnode_set.insert(new_node);

        for (llvm::Value *val : new_node->getAliasSet()) {
            new_val2node[val] = new_node;
        }
    }

    struct EdgeKey {
        AGNode *src;
        AGNode *dst;
        int offset;

        bool operator==(const EdgeKey &other) const {
            return src == other.src &&
                   dst == other.dst &&
                   offset == other.offset;
        }
    };

    struct EdgeKeyHash {
        size_t operator()(const EdgeKey &k) const {
            size_t h1 = std::hash<AGNode *>()(k.src);
            size_t h2 = std::hash<AGNode *>()(k.dst);
            size_t h3 = std::hash<int>()(k.offset);
            return h1 ^ (h2 << 1) ^ (h3 << 2);
        }
    };

    unordered_set<EdgeKey, EdgeKeyHash> edge_seen;

    for (AGEdge *old_edge : agedge_set) {
        if (!old_edge) continue;

        AGNode *old_src = old_edge->getSrc();
        AGNode *old_dst = old_edge->getDst();
        int offset = old_edge->getOffset();

        if (!old_src || !old_dst) continue;

        AGNode *new_src = rep2newnode[uf.find(old_src)];
        AGNode *new_dst = rep2newnode[uf.find(old_dst)];

        EdgeKey key{new_src, new_dst, offset};
        if (edge_seen.count(key)) {
            continue;
        }
        edge_seen.insert(key);

        AGEdge *new_edge = new AGEdge(new_src, new_dst, offset);
        new_agedge_set.insert(new_edge);

        new_src->pushOutEdge(offset, new_edge);
        new_dst->pushInEdge(new_edge);
    }

    // =========================================================
    // Step 5. 释放旧图，替换为压缩后的新图
    // =========================================================
    for (AGEdge *edge : agedge_set) {
        delete edge;
    }

    for (AGNode *node : agnode_set) {
        delete node;
    }

    agedge_set = std::move(new_agedge_set);
    agnode_set = std::move(new_agnode_set);
    val2node = std::move(new_val2node);
}
