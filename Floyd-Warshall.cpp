/*
						// Floyd - Warshall //
			
		Flyod - Warshall : 그래프에서 모든 정점에 대한 최단 경로를 찾는 알고리즘
		
		
			-> i -> j 경로보다 i -> k + k -> j 경로가 더 최단이면 i -> j 대신에 i -> k + k -> j 를 이용  
			
			for(int k = 1; k <= N; k++)
				for(int i = 1; i <= N; i++)
					for(int j = 1; j <= N; j++)
						if(D[i][j] > D[i][k] + D[k][j])
							D[i][j] = D[i][k] + D[k][j];

		
		- 두 정점에 대한 Flyod - Warshall 경로 찾기
				
			void Path(int i, int j)
			{
				if(P[i][j] == 0)
				{
					v.push_back(i);
					v.push_back(j);
					return;
				}
			
				int temp = P[i][j];
				
				Path(i, temp);
				v.pop_back();
				Path(temp, j);
			}
		
		
		- Flyod - Warshall 정보를 이용하여 원래의 그래프 복구 [ 그래프는 여러가지가 나올 수 있으나 간선의 개수가 최소인 그래프로 복구 ]		
		
				for (int k = 1; k <= N; k++)
				{
					for (int i = 1; i <= N; i++)
					{
						for (int j = 1; j <= N; j++)
						{
							if (i == j || j == k || i == k)
								continue;
			
							if (D[i][j] > D[i][k] + D[k][j])
							{
								printf("-1");
								return 0;
							}
			
							if (D[i][j] == D[i][k] + D[k][j])
								C[i][j] = 1; // i -> j 경로는 필요 x 
						}
					}
				}
		
		
		- 각 정점에 대한 사이클 찾기
				
			result = -1;
			for (int k = 1; k <= N; k++) 
				for (int i = 1; i <= N; i++)
					for (int j = 1; j <= N; j++) 
						if (D[i][j] > D[i][k] + D[k][j])
							D[i][j] = D[i][k] + D[k][j];
		
			for (int i = 1; i <= N; i++)
				if (D[i][i] != INF) 
					if (result == -1 || result > D[i][i]) 
						result = D[i][i];	
*/
