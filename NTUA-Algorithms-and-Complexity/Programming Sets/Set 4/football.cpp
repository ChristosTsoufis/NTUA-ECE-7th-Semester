#include <cstdio>
#include <vector>
#include <iostream>
#include <list> 
#include <stack> 
#include <map>

using namespace std;

//used code from geeksforgeeks
bool flag=true;
map<int, int> gvs;
int iter;
int mf = 1;

class Graph { 
    int V;    // No. of vertices 
    list<int> *adj;    // An array of adjacency lists 
  
    void fillOrder(int v, bool visited[], stack<int> &Stack); 

    void resultconnectswithall(int v, bool visited[], stack<int> &Stack); 
  
    void makemap(int v, bool visited[]); 

public: 
    Graph(int V); 
    void addEdge(int v, int w); 
  
    void gimmedaresult(); 
  
    Graph getTranspose(); 
}; 
  
Graph::Graph(int V) { 
    this->V = V; 
    adj = new list<int>[V]; 
} 
  
void Graph::makemap(int v, bool visited[]) { 

    visited[v] = true; 
    gvs.insert(pair<int, int>(v,iter));


  
    list<int>::iterator i; 
    for (i = adj[v].begin(); i != adj[v].end(); ++i) {
        if (!visited[*i]) {
            makemap(*i, visited); 
        }
    }
} 
  
Graph Graph::getTranspose() { 
    Graph g(V); 
    for (int v = 0; v < V; v++) { 
        // Recur for all the vertices adjacent to this vertex 
        list<int>::iterator i; 
        for(i = adj[v].begin(); i != adj[v].end(); ++i) 
        { 
            g.adj[*i].push_back(v); 
        } 
    } 
    return g; 
} 
  
void Graph::addEdge(int v, int w) { 
    adj[v].push_back(w); // Add w to v’s list. 
} 
  
void Graph::fillOrder(int v, bool visited[], stack<int> &Stack) { 
    // Mark the current node as visited and print it 
    visited[v] = true; 
  
    // Recur for all the vertices adjacent to this vertex 
    list<int>::iterator i; 
    for(i = adj[v].begin(); i != adj[v].end(); ++i) 
        if(!visited[*i]) 
            fillOrder(*i, visited, Stack); 
  
    // All vertices reachable from v are processed by now, push v  
    Stack.push(v); 
} 

void Graph::resultconnectswithall(int v, bool visited[], stack<int> &Stack) { 
    // Mark the current node as visited and print it 
    visited[v] = true; 
  
    // Recur for all the vertices adjacent to this vertex 
    list<int>::iterator i; 
    for(i = adj[v].begin(); i != adj[v].end(); ++i) {
    	//if (*i==Stack.top()) break;
        if(!visited[*i]){  
        	mf++;
            resultconnectswithall(*i, visited, Stack); 
        }
        
    }
  
    
} 
  

void Graph::gimmedaresult() { 
    stack<int> Stack;
  
    // Mark all the vertices as not visited (For first DFS) 
    bool *visited = new bool[V]; 
    for(int i = 0; i < V; i++) 
        visited[i] = false; 
  
    // Fill vertices in stack according to their finishing times 
    for(int i = 0; i < V; i++) 
        if(visited[i] == false) 
            fillOrder(i, visited, Stack); 
  
    // Create a reversed graph 
    Graph gr = getTranspose(); 
  
    // Mark all the vertices as not visited (For second DFS) 
    for(int i = 0; i < V; i++) 
        visited[i] = false; 

    // iter is the number of nodes in the dag that will be created by the strongly connected components
    iter=0;

    // Now process all vertices in order defined by Stack 
    while (Stack.empty() == false) { 
        // Pop a vertex from stack 
        int v = Stack.top();
        Stack.pop(); 
  
        // make map: nodes from initial graph to nodes to supergraph (dag from strongly connected components)
        if (visited[v] == false) {   
            gr.makemap(v, visited); 
            iter++;
        } 
    }
    // dag by the strongly connected components
    Graph super(iter);


    for (int v=0 ; v < V ; v++){
        list<int>::iterator i; 
        for (i = adj[v].begin(); i != adj[v].end(); ++i) {
            if((gvs.find(v))->second != (gvs.find(*i))->second) {
                super.addEdge((gvs.find(v))->second, (gvs.find(*i))->second);  
                //cout << "ADDED " << (gvs.find(v))->second << " " << (gvs.find(*i))->second << endl;
            }
        }
    }

    // Mark all the vertices as not visited for DFS in supergraph 
    bool *visited2 = new bool[iter]; 
    for(int i = 0; i < iter; i++) 
        visited2[i] = false;

    for(int i = 0; i < iter; i++) {
        if(visited2[i] == false) 
            super.fillOrder(i, visited2, Stack); 
    }

    int wanted= Stack.top();
    Stack.pop();

    for(int i = 0; i < iter; i++) 
        visited2[i] = false;

    //check if the first two teams are 
    super.resultconnectswithall(wanted, visited2, Stack); 

    
    //cout << "iter is " << iter << endl;
    //cout << "mf " << mf << endl;
    //solution does not connect with all nodes so no specific winner
    if (mf!=iter) cout << 0 << endl;

    else{
	    int result=0;

	    map<int, int>::iterator itr; 

	    for (itr = gvs.begin(); itr != gvs.end(); ++itr) { 
	        if (itr->second == wanted) result++;
	        //cout << '\t' << itr->first << '\t' << itr->second << '\n'; 
	    } 

	    cout << result << endl;
	}
} 

//----------------------------------------------------------------------------------


int main(){
	ios_base::sync_with_stdio(false);
	int N;
	cin >> N;

	Graph g(N); 

    int i = 0;

	for (i=0;i<N;i++){
		int k,l;
        cin >> k;
		for (int j=0; j<k; j++){
            cin >> l;
			g.addEdge(l-1,i);
		}
		
	}
	g.gimmedaresult(); 


    return 0;
}

