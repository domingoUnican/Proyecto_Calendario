\textbf{for} (i = 0; i < n; i++)\{
  \textbf{for} (j = 0; j < n; j++)
    \textbf{if} (x[i][j] != 0) \textbf{goto} {\color{red}no_nula};
  \textit{printf}({\color{blue}"La primera fila nula es "}
         {\color{blue}"%d.\textbackslash{}n}", i);
  \textbf{break};
 {\color{red}no_nula:};
 \}
