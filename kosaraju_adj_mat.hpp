#include <bits/stdc++.h>
using namespace std;

class GFG_adj_mat {
public:
    // dfs Function to reach destination
    bool dfs(int curr, int des, vector<vector<int> >& adjmat,
             vector<int>& visited)
    {

        // If curr node is destination return true
        if (curr == des) {
            return true;
        }
        visited[curr] = 1;
        for (int x=0;x<adjmat.size();x++) {
            if (!visited[x] && adjmat[curr][x]) {
                if (dfs(x, des, adjmat, visited)) {
                    return true;
                }
            }
        }
        return false;
    }

    // To tell whether there is path from source to
    // destination
    bool isPath(int src, int des, vector<vector<int> >& adjmat)
    {
        vector<int> visited(adjmat.size() + 1, 0);
        return dfs(src, des, adjmat, visited);
    }

    // Function to return all the strongly connected
    // component of a graph.
    vector<vector<int>> findSCC(int n,
                                vector<vector<int> >& a)
    {
        // Stores all the strongly connected components.
        vector<vector<int> > ans;

        // Stores whether a vertex is a part of any Strongly
        // Connected Component
        vector<int> is_scc(n + 1, 0);

        vector<vector<int> > adjmat(n + 1,vector<int>(n+1,0));

        //adj[i] contains all the vertices that vertex i connected to
        for (int i = 0; i < a.size(); i++) {
            adjmat[a[i][0]][a[i][1]] = 1;
        }
        // Traversing all the vertices
        for (int i = 1; i <= n; i++) {
            if (!is_scc[i]) {

                // If a vertex i is not a part of any SCC
                // insert it into a new SCC list and check
                // for other vertices whether they can be
                // a part of this list.
                vector<int> scc;
                scc.push_back(i);

                for (int j = i + 1; j <= n; j++) {

                    // If there is a path from vertex i to
                    // vertex j and vice versa put vertex j
                    // into the current SCC list.
                    if (!is_scc[j] && isPath(i, j, adjmat)
                        && isPath(j, i, adjmat)) {
                        is_scc[j] = 1;
                        scc.push_back(j);
                    }
                }

                // Insert the SCC containing vertex i into
                // the final list.
                ans.push_back(scc);
            }
        }
        return ans;
    }
};
