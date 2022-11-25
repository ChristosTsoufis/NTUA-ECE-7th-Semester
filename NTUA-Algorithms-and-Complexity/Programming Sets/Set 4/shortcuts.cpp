#include <cstdio>
#include <vector>
#include <set>

#define MAXN 1005
#define MAXM 10005
#define ll long long
#define X first
#define Y second

using namespace std;

ll N, M, B, s, t;
vector<pair<ll,ll> > adj[MAXN];
ll dp[MAXN][MAXN];

const ll INF = 123456789123LL;

int main() {
  scanf("%lld %lld %lld %lld %lld", &N, &M, &s, &t, &B);

  for(int i = 0; i < M; i++){
    ll u,v,w;
    scanf("%lld %lld %lld", &u, &v, &w);
    adj[u].push_back({v,w});
  }

  for(int i = 1; i <= N; i++){
    for(int j = 0; j < N; j++){
      dp[i][j] = INF;
    }
  }

  set<pair<ll,pair<ll,ll>>> q;

  dp[s][0] = 0;
  q.insert({0,{s,0}});

  while(!q.empty()){
    ll u = q.begin()->Y.X;
    ll k = q.begin()->Y.Y;
    q.erase(q.begin());

    for(auto edge: adj[u]){
      ll w = edge.Y;
      ll v = edge.X;

      if(dp[v][k] > dp[u][k] + w){
        q.erase({dp[v][k], {v,k}});
        dp[v][k] = dp[u][k] + w;
        q.insert({dp[v][k], {v,k}});
      }

      if(k+1 < N && dp[v][k+1] > dp[u][k]){
        q.erase({dp[v][k+1], {v,k+1}});
        dp[v][k+1] = dp[u][k];
        q.insert({dp[v][k+1], {v, k+1}});
      }
    }

  }

  int answer = N;

  for(int k = 0; k < N; k++){
    if(dp[t][k] <= B){
      answer = k;
      break;
    }
  }

  printf("%d\n", answer);
}
