i = 0;
do{
  j = 0;
  while (j < n && x[i][j] == 0) j++;
  i++;
 } while (i < n && j < n);
if (j == n)
  printf("La primera fila nula es "
	 "%d.\n", i - 1);
