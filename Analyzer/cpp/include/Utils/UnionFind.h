#ifndef _UNION_FIND_H_
#define _UNION_FIND_H_

#include <unordered_map>

template <typename T>
class UnionFind {
private:
	std::unordered_map<T, T> parent;
	std::unordered_map<T, int> size;

public:
	void add(const T& x) {
		if (parent.count(x)) {
			return;
		}
		parent[x] = x;
		size[x] = 1;
	}

	T find(const T& x) {
		if (!parent.count(x)) {
			add(x);
		}

		if (parent[x] != x) {
			parent[x] = find(parent[x]);
		}

		return parent[x];
	}

	void unite(const T& x, const T& y) {
		T rx = find(x);
		T ry = find(y);

		if (rx == ry) {
			return;
		}

		if (size[rx] < size[ry]) {
			std::swap(rx, ry);
		}

		parent[ry] = rx;
		size[rx] += size[ry];
	}

	bool connected(const T& x, const T& y) {
		return find(x) == find(y);
	}

	int getSize(const T& x) {
		return size[find(x)];
	}
};

#endif
