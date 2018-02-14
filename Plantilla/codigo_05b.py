c = True
i = 0
while c and i != N:
    d = True
    j = 0
    while d and j != N:
        d = X[i][j] == 0
        j += 1
    c = not d
    i += 1
if not c:
    print("La primera fila nula es %d." % (i - 1))
