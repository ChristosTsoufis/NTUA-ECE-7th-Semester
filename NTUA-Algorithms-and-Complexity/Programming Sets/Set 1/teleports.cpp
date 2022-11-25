#include <iostream>
#include <algorithm>
using namespace std; 

class vertice {
	private:
		int a, b, w;
	
	public: 
		vertice (): a(0), b(0), w(0) {}
		vertice (int v, int u, int w0): a(v), b(u), w(w0) {}
		int u() {
			return a;
		};
		int v() {
			return b;
		};
		int weight() {
			return w;
		};
		void operator = (vertice u){
			a = u.a;
			b = u.b;
			w = u.w;
			return;
		};
		bool operator == (vertice u){
			return w == u.w;
		};
		bool operator < (vertice u){
			return w < u.w;
		};
		bool operator > (vertice u){
			return w > u.w;
		};
};

void swap (vertice *a, vertice *b){
	vertice	*c = new vertice();
	*c = *a;
	*a = *b;
	*b = *c;
}
int partition(vertice A[], int left, int right) {
	vertice pivot;
	pivot = A[left];
	int i = left - 1, j = right + 1;
	while (1) {
		while (A[++i] < pivot);
		while (A[--j] > pivot);
		if (i < j) swap(&A[i], &A[j]);
		else return(j); 
	}
}
void quickSort(vertice A[], int left, int right) {
	if (left >= right) return; // At most 1 element
	int pivot = rand() % (right - left + 1) + left;
	swap(A[left], A[pivot]);
	int q = partition(A, left, right);
	quickSort(A, left, q);
	quickSort(A, q+1, right); 
}

int universe[1000001];
int parent[1000001];
vertice vertices[1000000];

int find(int x) {
	while (x != parent[x]) x = parent[x];
	return(x); 
	}
	
void unionTree(int x, int y) {
	int z = find(y);
	int w;
	while (x != parent[x]) {
		w = x;
		x = parent[x];
		parent[w] = z;
	}
	parent[x] = z;
	while (y != parent[y]) {
		w = y;
		y = parent[y];
		parent[w] = z;
	}
} 

int main(int argc, char *argv[]) { 
	int N, M, a, b, w, i, j;

	scanf("%d %d", &N, &M);
	for (i = 1; i <= N; i++) {
		scanf("%d", &universe[i]);
		parent[i] = i;
	}
	for (i = 0; i < M; i++) {
		scanf("%d %d %d", &a, &b, &w);
		vertices[i] = vertice(a,b,w);
	}

	quickSort(vertices, 0, M-1);
	i = M-1;
	j = 1;
	while (i >= 0){
		unionTree(vertices[i].u(), vertices[i].v());
		while (find(j) == find(universe[j]))
			if (j == N) {
				printf("%d\n", vertices[i].weight());
				return 0;
			}else j++;
		i--;
	}
	return 0;
}