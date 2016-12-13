from datetime import timedelta

class horario:
    '''
    Clase que implementa un horario. Ahora supongo que todos los
    dias empiezan y terminan a la misma hora. Eso quiere decir
    que si hay huecos libres entonces que se pongan explicitamente.
    Y que todos los dias tiene que darse clase, aunque sea un hueco libre.
    '''

    def __init__(self, dias, inicio):
        self.dias = dias
        self.h_dia = [list() for i in dias]
        self.inicio = inicio
        self.empieza_tarde = timedelta(hours = 15)

    def incluye_hora(self, dia, asignatura, inicio, fin):
        pos = self.dias.index(dia)
        nueva_lista = []
        encontrado = False
        if not self.h_dia[pos]:
            self.h_dia[pos].append(tuple((asignatura,fin)))
        else:
            for l in self.h_dia[pos]:
                nueva_lista.append(tuple(l))
                if not encontrado and l[1]>=inicio:
                    print str(l[1]), str(inicio)
                    nueva_lista.append(tuple((asignatura,fin)))
                    encontrado = True
                self.h_dia[pos] = nueva_lista

    def segundos_clase_manana(self):
        '''
        Metodo que devuelve los segundos totales que hay
        que estar cada dia dando clase por la manana.
        Recordad que todos los dias tienen que tener las mismas horas,
        por lo que hay que rellenar con huecos libres.
        '''
        manana = [l for l in self.h_dia[0] if l[1]<=self.empieza_tarde]
        return (manana[-1][1]-self.inicio).seconds

    def segundos_clase_tarde(self):
        '''
        Metodo que devuelve los segundos totales que hay
        que estar cada dia dando clase por la tarde.
        Recordad que todos los dias tienen que tener las mismas horas,
        por lo que hay que rellenar con huecos libres.
        '''
        manana = [l for l in self.h_dia[0] if l[1]>=self.empieza_tarde]
        return (self.h_dia[0][-1][1]-self.empieza_tarde).seconds

    def clases_manana(self, dia):
        '''
        simple iterador para dibujar los botones
        '''
        pos = self.dias.index(dia)

        principio = self.inicio
        total = float(self.segundos_clase_manana())
        manana = [l for l in self.h_dia[pos] if l[1]<=self.empieza_tarde]
        for asignatura, fin in manana:
            porcentaje = (fin-principio).seconds/total
            porcentaje *= 0.98
            texto = ('%s\n%s--%s')%(asignatura, principio, fin)
            yield texto, porcentaje
            principio = fin


    def clases_tarde(self, dia):
        '''
        simple iterador para dibujar los botones
        '''
        pos = self.dias.index(dia)
        principio = self.empieza_tarde
        total = float(self.segundos_clase_tarde())
        tarde = [l for l in self.h_dia[pos] if l[1]>self.empieza_tarde]
        for asignatura, fin in tarde:
            porcentaje = (fin-principio).seconds/total
            porcentaje *= 0.98
            texto = ('%s\n%s--%s')%(asignatura, principio, fin)
            yield texto, porcentaje
            principio = fin
            
    def change_buttons(self):
        sys.stdout.write('1')
