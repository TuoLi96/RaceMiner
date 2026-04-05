#ifndef _UNION_FIND_H_
#define _UNION_FIND_H_

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <stdexcept>
#include <utility>

template <typename T>
class UnionFind {
private:
	std::unordered_map<T, T> parent;
	std::unordered_map<T, int> sz;

	std::unordered_map<T, std::unordered_set<T>> members;

	int num_sets = 0;

public:
	UnionFind() = default;
	~UnionFind() = default;

public:
	bool contains(const T &x) const {
		return parent.count(x) != 0;
	}

	void add(const T &x) {
		if (contains(x)) {
			return;
		}

		parent[x] = x;
		sz[x] = 1;
		members[x].insert(x);
		num_sets++;
	}

	T find(const T &x) {
		if (!contains(x)) {
			add(x);
		}

		if (parent[x] != x) {
			parent[x] = find(parent[x]);
		}

		return parent[x];
	}

	T unite(const T &x, const T &y) {
		T rx = find(x);
		T ry = find(y);

		if (rx == ry) {
			return rx;
		}

		if (sz[rx] < sz[ry]) {
			std::swap(rx, ry);
		}

		parent[ry] = rx;
		sz[rx] += sz[ry];

		members[rx].insert(members[ry].begin(), members[ry].end());
		members.erase(ry);

		num_sets--;
		return rx;
	}

	bool same(const T &x, const T &y) {
		return find(x) == find(y);
	}

	bool connected(const T &x, const T &y) {
		return same(x, y);
	}

	int sizeOf(const T &x) {
		return sz[find(x)];
	}

	int getSize(const T &x) {
		return sizeOf(x);
	}

	int numSets() const {
		return num_sets;
	}

	const std::unordered_set<T> &getMembers(const T &x) {
		T r = find(x);
		return members.at(r);
	}

	std::vector<T> getRoots() {
		std::vector<T> roots;
		roots.reserve(members.size());
		for (auto &[root, _] : members) {
			roots.push_back(root);
		}
		return roots;
	}

	std::vector<T> getAllElements() const {
		std::vector<T> elems;
		elems.reserve(parent.size());
		for (const auto &[x, _] : parent) {
			elems.push_back(x);
		}
		return elems;
	}
};

#endif
