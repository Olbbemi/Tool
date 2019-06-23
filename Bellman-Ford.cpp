/*
															// Bellman - Ford //
				
	- Bellman - Ford : 한 정점에서 다른 모든 정점의 최단 경로를 구하는 알고리즘
							, 다익스트라 알고리즘과 달리 가중치에 음수가 존재해도 최단 경로를 구할 수 있음
							, 인접행렬 및 인접리스트 필요 x
							, 각 정점에 대해서 간선만큼 반복 
	 
	 	- 음수 사이클 : 가중치에 음수가 존재하여 특정 정점에 방문할 때마다 음수로 수렴하는 경우
		 					, 음수 사이클이 존재한다는 뜻은 해당 정점으로 이동하는 최단 경로가 존재 x  
	 
		N : 정점 개수
		M : 간선 개수
		
			->  Dist[1] = 0;
				for (int i = 1; i <= N; i++) // 실제 Bellman - Ford 알고리즘은 N - 1 번 반복 [ N 번째는 음수 사이클 확인 ] 
				{
					for (int j = 0; j < M; j++)
					{
						if (Dist[v[j].start] != INF && Dist[v[j].end] > v[j].weight + Dist[v[j].start])
						{
							Dist[v[j].end] = v[j].weight + Dist[v[j].start];
			
							if (i == N) // 해당 정점에 대한 최단 경로가 결정되었다면 Bellma - Ford 알고리즘을 반복해도 변화 x
								Nagative = 1;
						}
					}
				}
	
	- SPFA [ Shortest Path Faster Algorithm ] : A -> B 에 대한 최단 경로가 변화하지 않았다면 다음 반복에 대해서 B 에 대해서는 굳이 확인 x 
												, Queue 이용 
	
		D[] : 각 정점에 대한 최단 경로 정보 저장
		Check[] : 해당 정점에 대한 최단 경로 정보가 변화하면 해당 정점을 push 및 true, pop 할 때 false  
		Cnt[] : 각 정점을 방문한 횟수 [ 각 정점을 전체 정점 개수 이상 방문했다면 음수 사이클 존재를 의미 ] 
	
			->  D[1] = 0;
				q.push(1);
				Check[1] = 1;
			
				while (q.empty() != 1) 
				{
					int temp = q.front();	q.pop();
					Check[temp] = 0;
					
					for (int j = 0; j < Graph[temp].size(); j++) 
					{
						if (D[Graph[temp][j].end] > D[temp] + Graph[temp][j].weight) 
						{
							D[Graph[temp][j].end] = D[temp] + Graph[temp][j].weight;
					
							if (Check[Graph[temp][j].end] == 0) 
							{
								q.push(Graph[temp][j].end);
								Check[Graph[temp][j].end] = 1;
								Cnt[Graph[temp][j].end]++;
								
								if (Cnt[Graph[temp][j].end] >= N) 
								{
									printf("-1\n"); // Negative Cycle
									return 0;
								}
							}
						}
					}
				}	
*/
