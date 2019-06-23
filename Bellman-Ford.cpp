/*
															// Bellman - Ford //
				
	- Bellman - Ford : �� �������� �ٸ� ��� ������ �ִ� ��θ� ���ϴ� �˰���
							, ���ͽ�Ʈ�� �˰���� �޸� ����ġ�� ������ �����ص� �ִ� ��θ� ���� �� ����
							, ������� �� ��������Ʈ �ʿ� x
							, �� ������ ���ؼ� ������ŭ �ݺ� 
	 
	 	- ���� ����Ŭ : ����ġ�� ������ �����Ͽ� Ư�� ������ �湮�� ������ ������ �����ϴ� ���
		 					, ���� ����Ŭ�� �����Ѵٴ� ���� �ش� �������� �̵��ϴ� �ִ� ��ΰ� ���� x  
	 
		N : ���� ����
		M : ���� ����
		
			->  Dist[1] = 0;
				for (int i = 1; i <= N; i++) // ���� Bellman - Ford �˰����� N - 1 �� �ݺ� [ N ��°�� ���� ����Ŭ Ȯ�� ] 
				{
					for (int j = 0; j < M; j++)
					{
						if (Dist[v[j].start] != INF && Dist[v[j].end] > v[j].weight + Dist[v[j].start])
						{
							Dist[v[j].end] = v[j].weight + Dist[v[j].start];
			
							if (i == N) // �ش� ������ ���� �ִ� ��ΰ� �����Ǿ��ٸ� Bellma - Ford �˰����� �ݺ��ص� ��ȭ x
								Nagative = 1;
						}
					}
				}
	
	- SPFA [ Shortest Path Faster Algorithm ] : A -> B �� ���� �ִ� ��ΰ� ��ȭ���� �ʾҴٸ� ���� �ݺ��� ���ؼ� B �� ���ؼ��� ���� Ȯ�� x 
												, Queue �̿� 
	
		D[] : �� ������ ���� �ִ� ��� ���� ����
		Check[] : �ش� ������ ���� �ִ� ��� ������ ��ȭ�ϸ� �ش� ������ push �� true, pop �� �� false  
		Cnt[] : �� ������ �湮�� Ƚ�� [ �� ������ ��ü ���� ���� �̻� �湮�ߴٸ� ���� ����Ŭ ���縦 �ǹ� ] 
	
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
