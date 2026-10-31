import java.util.*;

public class BFS {
    static void bfs(List<List<Integer>> adj, int start) {
        boolean[] visited = new boolean[adj.size()];
        Queue<Integer> q = new LinkedList<>();
        q.add(start);
        visited[start] = true;
        while (!q.isEmpty()) {
            int node = q.poll();
            System.out.print(node + " ");
            for (int nb : adj.get(node)) {
                if (!visited[nb]) {
                    visited[nb] = true;
                    q.add(nb);
                }
            }
        }
    }

    public static void main(String[] args) {
        List<List<Integer>> adj = new ArrayList<>();
        for (int i = 0; i < 5; i++) adj.add(new ArrayList<>());
        adj.get(0).add(1); adj.get(0).add(2);
        adj.get(1).add(3);
        adj.get(2).add(4);
        adj.get(3).add(4);
        bfs(adj, 0);
    }
}
