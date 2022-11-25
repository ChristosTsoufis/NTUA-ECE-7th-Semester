#include <iostream>
#include <algorithm>
#include <fstream>
#include <unordered_map>

using namespace std;
long long int solve(long long int Cs,long long int Cf,long long int Ts,long long int Tf,long long int c,long long int d){
  if(c-d*Cs<0){
    return -1;
  }
  long long int temp = (c-d*Cs)/(Cf-Cs);
  while( (d-temp)< 0){
      temp = d;
  }
  return temp*Tf + (d-temp)*Ts;
}

int main(){
  long long int N,K,D,T,time,fast,slow,Ts,Cs,Tf,Cf,currentN;
  int j;
  unordered_map<long long int, long long int> Costs;
  
  cin >>  N >> K >> D >> T ;
  long long int * dist = new long long int[K+1];
  long long int cost;
  long long int *cap = new long long int[N];
  for(int i=0; i<N; i++){
    cin >> cost >> cap[i];
    
    if (Costs.find(cap[i]) == Costs.end()){
        Costs[cap[i]] = cost;
    }
    else if (Costs[cap[i]] > cost){
        Costs[cap[i]] = cost;
    }
  }
  sort(cap,cap + N);
  for(long long int i=0; i<K; i++){
      cin >> dist[i];
  }
  dist[K] = D;
  sort(dist,dist+K+1);
  long long int maxDistance=dist[0];
  for(long long int l=1; l<K+1; l++){
    maxDistance = max(maxDistance,dist[l]-dist[l-1]);
  }
  if(cap[N-1]<maxDistance){
    cout << -1 << endl;
    return 0;
  }
  cin >> Ts >> Cs >> Tf >> Cf;

  bool Acceptable = false;
  bool Faulty = false;
  long long int temp, minseen, maxseen, distance, step;
  minseen = N;
  maxseen = 0;
  step = N/2;
  currentN = N/2;
  while(step!=0){
    step = step / 2;
    time = 0;
    distance = 0;
        for(j=0; j<K; j++){
          if(cap[currentN]<maxDistance){
            temp=-1;
            break;
          }
          distance = dist[j+1] - dist[j];
          long long int what = j;
          if(distance < cap[currentN]/Cf){
              while(distance < cap[currentN]/Cf && j<K){
                distance = dist[++j] - dist[what];
              }
              if (j!=what){
                  j--;
                  distance = dist[j] - dist[what];
              }
          }
          temp = solve(Cs,Cf,Ts,Tf,cap[currentN],distance);
          if(temp==-1 || time > T){
              break;
            }
            time += temp;
        }
        if(temp == -1 || time > T){
          Faulty = true;
            if(step!=0){
                maxseen = max(maxseen,currentN);
                currentN += step;
            }
            else{
                maxseen = max(maxseen,currentN);
                currentN += 1;
                break;
            }
            continue;
        }
        minseen = min(minseen,currentN);
        currentN -= step;
        Acceptable = true;
    }
    if (currentN >= N){
        cout << -1 <<endl;
        delete dist;
        delete cap;
        return 0;
    }
    if(maxseen == 0){
        maxseen--;
    }
        for(int i = maxseen+1; i<minseen; i++){
            time = 0;
            distance = 0;
                for(j=0; j<K; j++){
                  if(cap[i]<maxDistance){
                    temp=-1;
                    break;
                  }
                  distance = dist[j+1] - dist[j];
                  long long int what = j;
                  if(distance < cap[i]/Cf){
                      while(distance < cap[i]/Cf && j<K){
                        distance = dist[++j] - dist[what];
                      }
                      if (j!=what){
                          j--;
                          distance = dist[j] - dist[what];
                      }
                  }
                  temp = solve(Cs,Cf,Ts,Tf,cap[i],distance);
                  if(temp==-1 || time > T){
                      break;
                    }
                    time += temp;
                }
                if(temp != -1 && time <= T){
                    currentN = i;
                    break;
                }
                if(i==N-1){
                    cout << -1 << endl;
                    delete dist;
                    delete cap;
                    return 0;
                }
        }
    if (currentN >= N){
        cout << -1 <<endl;
        delete dist;
        delete cap;
        return 0;
    }
    else{
      long long int temp1 = Costs[cap[currentN]];
        for(int i=currentN; i<N; i++){
          temp1 = min(Costs[cap[i]],temp1);
    }
    cout << temp1 << endl;
    delete dist;
    delete cap;
    return 0;
  }
  return 0;
}
