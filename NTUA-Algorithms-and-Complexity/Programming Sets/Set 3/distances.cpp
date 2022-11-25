#include <iostream>
#include <algorithm>
#include <utility>
#include <vector>
#include <list>
#include <tuple>
#include <vector>
#include <bitset>
#include <climits>

#define INT_BITS sizeof(int)*8
#define MAX_N 100000

using namespace std;

int N, M;

struct Edge{
	int node1, node2, weight;
};

struct Node{
	list<pair<int, int>> adjacent;
};

struct set{
	int parent, rank;
} subsets[MAX_N];

int find(int node){
	while (node != subsets[node].parent) node = find(subsets[node].parent);
	return node;
}

void Union(int x, int y){
	int setx, sety;
	setx = find(x);
	sety = find(y);
	if (setx == sety) return;
	if (subsets[setx].rank > subsets[sety].rank) subsets[sety].parent = setx;
	else{
		subsets[setx].parent = sety;
		if (subsets[setx].rank == subsets[sety].rank) ++(subsets[sety].rank);
	}
}

bool sameset(Edge &e){
	int setx, sety;
	setx = find(e.node1);
	sety = find(e.node2);
	if (setx == sety) return true;
	else Union(e.node1, e.node2);
	return false;
}

int dfs(const vector<Node> & nodes, int current, int parent, vector<unsigned long long> & times){
	const Node& cur = nodes.at(current);
	int count = 1;
	for (auto p : cur.adjacent){
		int child, index;
		unsigned long long set_size;
		tie(child, index) = p;
		if (child == parent) continue;
		set_size = dfs(nodes, child, current, times);
		times.at(index) += set_size*(N-set_size);
		while (times[index] > 1){
			if (index == (int) times.size()-1){
				times.push_back(times[index]/2);
				times[index] %= 2;
			}
			else {
				times[index+1] += times[index]/2;
				times[index] %= 2;
			}
			++index;
		}
		count += set_size;
	}
	return count;
}


int main(){
	int last;
	string result;
	ios::sync_with_stdio(false);
	cin.tie(NULL);
	cin >> N >> M;
	vector<unsigned long long> times(M, 0);
	vector<Edge> edges(M);
	vector<Node> treeNodes(N+1);
	for (int i = 0; i<M; i++){
		int n1, n2, w;
		cin >> n1 >> n2 >> w;
		edges[w].weight = w;
		edges[w].node1 = n1;
		edges[w].node2 = n2;
	}
	for (int i=0; i<N; ++i){
		subsets[i].parent = i;
		subsets[i].rank = 0;
	}
	edges.erase(remove_if(edges.begin(), edges.end(), sameset), edges.end());
	for (unsigned int i=0; i<edges.size(); i++){
		int first, second, weight;
		first = edges[i].node1;
		second = edges[i].node2;
		weight = edges[i].weight;
		treeNodes[first].adjacent.push_back(make_pair(second, weight));
		treeNodes[second].adjacent.push_back(make_pair(first, weight));
	}
	dfs(treeNodes, N/2, 0, times);
	int pos = 0;
	result.resize(times.size());
	for (last= times.size()-1; last>=0; last--){
		result[pos] = (times[last]) ? '1' : '0';
		pos++;
	}
	result = result.substr(result.find_first_of('1'), result.length());
	cout << result << endl;
}

