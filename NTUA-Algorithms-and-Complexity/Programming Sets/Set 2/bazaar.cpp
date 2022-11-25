#include <iostream> 
#include <utility>
#include <vector>
#include <algorithm>

using namespace std;

vector<int> knapsack[9];  
vector<int> knapsack2[3];
vector<pair<int,int>> traders[9];
int items[9] = {0};
int items2[3] = {0};
int lim = 2000000000;
int matrix [1501][5001];

int KNAPSACK(int i, int b, vector<pair<int,int>> &v){
    int quantity = v[i-1].first;
    int cost = v[i-1].second; 
    
    if (matrix[i][b] == -1){
        if (b < 1) matrix[i][b] = lim;
        else if (b == 1) matrix[i][b] = min(KNAPSACK(i-1, 1, v), cost);
        else if (i == 1) {
            if (b <= quantity) matrix[i][b] = cost;
            else matrix[i][b] = lim;
        }
        else if (b <= quantity) matrix[i][b] = min(KNAPSACK(i-1, b, v), cost);
        else matrix[i][b] = min(KNAPSACK(i-1, b, v), KNAPSACK(i-1, b-quantity, v) + cost);
    }
    return matrix[i][b];
}

void init(int B, vector<pair<int,int>> &v, vector<int> &solution){
    solution.push_back(0);
    if(B!=0){
        int quantity, cost;
        int size = v.size();
        for (int i = 0; i <= size; i++){
            for (int b = 0; b<=B; b++){
                matrix[i][b] = -1;
            }
        }
        matrix[1][1] = v[0].second; 
        for (int b = 1; b<= B; b++)
            solution.push_back(KNAPSACK(size, b, v));
    }
    return;
}

int main(int argc, char *argv[]) { 
    FILE *input;
	int i, j, c, N, M, trader, amount, cost;
    char type;

	scanf("%d %d", &N, &M);
	for (i = 0; i < M; i++){
		scanf("%d %c %d %d", &trader, &type, &amount, &cost);
        if (type == 'C'){
            traders[3*(trader-1) + 2].push_back(make_pair(amount,cost));
            items[3*(trader-1) + 2] += amount;
        }
        if (type == 'B'){
            traders[3*(trader-1) + 1].push_back(make_pair(amount,cost));
            items[3*(trader-1) + 1] += amount;
        }
        if (type == 'A'){
            traders[3*(trader-1)].push_back(make_pair(amount,cost));
            items[3*(trader-1)] += amount;
        }
	}
    for (i = 0; i < 9; i++){
        items[i] = min(items[i], N);
        init(items[i], traders[i], knapsack[i]);    
    }
    for (i = 0; i < 3; i++){
        items2[i] = min(items[3*i], items[3*i+1]);
        items2[i] = min(items2[i], items[3*i+2]);
        for(j = 0; j <= items2[i]; j++){
            knapsack2[i].push_back(knapsack[3*i][j] + knapsack[3*i+1][j] + knapsack[3*i+2][j]);
        }
    }
    c = lim;
    i = 0;
    while(i <= N && i <= items2[0]){
        if (N <= i + items2[2]) j = 0;
        else j = N - i - items2[2];
        while(i+j <= N && j <= items2[1]){
            c = min(c, knapsack2[0][i] + knapsack2[1][j] + knapsack2[2][N-i-j]);
            j++;
        }
        i++;
    }
    if (c != lim)
        printf("%d\n",c);
    else
        printf("-1\n");
    return 0;
}