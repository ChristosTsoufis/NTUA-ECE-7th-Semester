#include <assert.h>
#include <iostream>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#define min(a, b) ((a) > (b) ? (b) : (b))

using namespace std;
inline int sumQuery(int **aux, int tli, int tlj, int rbi, int rbj) {

    return aux[rbi][rbj] - aux[tli - 1][rbj] - aux[rbi][tlj - 1] +
           aux[tli - 1][tlj - 1];
}
void partition(int **cost, int N, int K) {
    int n = N;
    int *p = (int *)malloc((N + 1) * sizeof(int));
    int **E = (int **)malloc((N + 1) * sizeof(int *));
    for (int i = 0; i <= N; i++)
        E[i] = (int *)malloc((K + 1) * sizeof(int));

    // p[0] = 0;
    // for(int i=1; i<=n; ++i) p[i] = p[i-1] + cost[1][i];
    E[0][0] = 0;
    for (int i = 1; i < N + 1; ++i) {
        E[i][1] = cost[i][i];
    }
    E[1][1] = 0;
    for (int i = 2; i < K + 1; ++i) {
        E[1][i] = 0;
    }
    for (int i = 2; i <= n; ++i) {
        int start = 0;
        for (int j = 2; j <= K; ++j) {
            E[i][j] = INT_MAX;
            for (int w = start; w < i; ++w) {
                int c = E[w][j - 1] + sumQuery(cost, w + 1, w + 1, i, i);
                if (E[i][j] > c) {
                    E[i][j] = c;
                    start = w;
                }
            }
            //cout << E[i][j] << " ";
        }
        //cout << endl;
    }

    cout << E[N][K] << endl;
}

int preProcess(int **mat, int **aux, int N) {

    for (int i = 0; i < N; i++)
        aux[0][i] = mat[0][i];

    for (int i = 1; i < N; i++)
        for (int j = 0; j < N; j++)
            aux[i][j] = mat[i][j] + aux[i - 1][j];

    for (int i = 0; i < N; i++)
        for (int j = 1; j < N; j++)
            aux[i][j] += aux[i][j - 1];
}

int main(int argc, char const *argv[]) {
    FILE *f = freopen(argv[1], "r", stdin);
    int N, K;
    std::ios_base::sync_with_stdio(false);
    cin >> N >> K;
    int **A = (int **)malloc((N + 1) * sizeof(int *));
    for (int i = 0; i <= N; i++)
        A[i] = (int *)calloc((N + 1), sizeof(int));

    for (int i = 0; i < N; ++i) {
        for (int j = i + 1; j < N; ++j) {
            cin >> A[i + 1][j + 1];
        }
    }
    int **cost = (int **)malloc((N + 1) * sizeof(int *));
    for (int i = 0; i <= N; i++)
        cost[i] = (int *)calloc((N + 1), sizeof(int));
    preProcess(A, cost, N + 1);
    for (int i = 0; i <= N; i++)
        free(A[i]);
    partition(cost, N, K);

    return 0;
}