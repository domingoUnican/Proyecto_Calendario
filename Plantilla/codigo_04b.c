for (i = 0; i < n; i++){
  for (j = 0; j < n; j++)
    if (x[i][j] != 0) goto no_nula;
  printf("La primera fila nula es "
	 "%d.\n", i);
  break;
 no_nula:;
 }
