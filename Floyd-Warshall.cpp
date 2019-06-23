/*
						// Floyd - Warshall //
			
		Flyod - Warshall : �׷������� ��� ������ ���� �ִ� ��θ� ã�� �˰���
		
		
			-> i -> j ��κ��� i -> k + k -> j ��ΰ� �� �ִ��̸� i -> j ��ſ� i -> k + k -> j �� �̿�  
			
			for(int k = 1; k <= N; k++)
				for(int i = 1; i <= N; i++)
					for(int j = 1; j <= N; j++)
						if(D[i][j] > D[i][k] + D[k][j])
							D[i][j] = D[i][k] + D[k][j];

		
		- �� ������ ���� Flyod - Warshall ��� ã��
				
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
		
		
		- Flyod - Warshall ������ �̿��Ͽ� ������ �׷��� ���� [ �׷����� ���������� ���� �� ������ ������ ������ �ּ��� �׷����� ���� ]		
		
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
								C[i][j] = 1; // i -> j ��δ� �ʿ� x 
						}
					}
				}
		
		
		- �� ������ ���� ����Ŭ ã��
				
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
