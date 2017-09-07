from datetime import timedelta
from lxml import etree

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
        self.empieza_tarde = 7
        self.choosed = False
        self.listaAulas = ''
        self.listaAsignaturas = ''

    def incluye_hora(self, dia, asignatura, asignaturaID, aula, aulaID, inicio, fin):
        '''
        Metodo que devuelve incluye los datos de una asignatura en el horario.
        '''
        pos = self.dias.index(dia)
        nueva_lista = []
        encontrado = False
        if not self.h_dia[pos]:
            self.h_dia[pos].append(tuple((asignatura,asignaturaID,aula,aulaID,fin)))
        else:
            for l in self.h_dia[pos]:
                nueva_lista.append(tuple(l))
                if not encontrado and l[4]>=inicio:
                    #print str(l[1]), str(inicio)
                    nueva_lista.append(tuple((asignatura,asignaturaID,aula,aulaID,fin)))
                    encontrado = True
                self.h_dia[pos] = nueva_lista

    def segundos_clase_manana(self):
        '''
        Metodo que devuelve los segundos totales que hay
        que estar cada dia dando clase por la manana.
        Recordad que todos los dias tienen que tener las mismas horas,
        por lo que hay que rellenar con huecos libres.
        '''
        manana = [l for l in self.h_dia[0] if l[4]<=self.empieza_tarde]
        return (manana[-1][4]-self.inicio)

    def segundos_clase_tarde(self):
        '''
        Metodo que devuelve los segundos totales que hay
        que estar cada dia dando clase por la tarde.
        Recordad que todos los dias tienen que tener las mismas horas,
        por lo que hay que rellenar con huecos libres.
        '''
        manana = [l for l in self.h_dia[0] if l[4]>=self.empieza_tarde]
        return (self.h_dia[0][-1][4]-self.empieza_tarde)

    def clases_manana(self, dia):
        '''
        simple iterador para dibujar los botones
        '''
        pos = self.dias.index(dia)

        principio = self.inicio
        total = float(self.segundos_clase_manana())
        manana = [l for l in self.h_dia[pos] if l[4]<=self.empieza_tarde]
        for asignatura, asignaturaID, aula, aulaID, fin in manana:
            porcentaje = (fin-principio)/total
            porcentaje *= 0.98
            texto = ('%s\n%s\n%s -- %s')%(asignatura, aula, principio, fin)
            yield texto, porcentaje, asignaturaID, aulaID
            principio = fin


    def clases_tarde(self, dia):
        '''
        simple iterador para dibujar los botones
        '''
        pos = self.dias.index(dia)
        principio = self.empieza_tarde
        total = float(self.segundos_clase_tarde())
        tarde = [l for l in self.h_dia[pos] if l[4]>self.empieza_tarde]
        for asignatura, asignaturaID, aula, aulaID, fin in tarde:
            porcentaje = (fin-principio)/total
            porcentaje *= 0.98
            texto = ('%s\n%s\n%s -- %s')%(asignatura, aula, principio, fin)
            yield texto, porcentaje
            principio = fin
        
