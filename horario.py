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
        self.empieza_tarde = timedelta(hours = 15)
        self.choosed = False
        self.listaAulas = ''
        self.listaAsignaturas = ''

    def incluye_hora(self, dia, asignatura, asignaturaID, aula, aulaID, inicio, fin):
        pos = self.dias.index(dia)
        nueva_lista = []
        encontrado = False
        if not self.h_dia[pos]:
            self.h_dia[pos].append(tuple((asignatura,asignaturaID,aula,aulaID,fin)))
        else:
            for l in self.h_dia[pos]:
                nueva_lista.append(tuple(l))
                print(l)
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
        return (manana[-1][4]-self.inicio).seconds

    def segundos_clase_tarde(self):
        '''
        Metodo que devuelve los segundos totales que hay
        que estar cada dia dando clase por la tarde.
        Recordad que todos los dias tienen que tener las mismas horas,
        por lo que hay que rellenar con huecos libres.
        '''
        manana = [l for l in self.h_dia[0] if l[4]>=self.empieza_tarde]
        return (self.h_dia[0][-1][4]-self.empieza_tarde).seconds

    def clases_manana(self, dia):
        '''
        simple iterador para dibujar los botones
        '''
        pos = self.dias.index(dia)

        principio = self.inicio
        total = float(self.segundos_clase_manana())
        manana = [l for l in self.h_dia[pos] if l[4]<=self.empieza_tarde]
        for asignatura, asignaturaID, aula, aulaID, fin in manana:
            porcentaje = (fin-principio).seconds/total
            porcentaje *= 0.98
            texto = ('%s\n%s\n%s--%s')%(asignatura, aula, principio, fin)
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
            porcentaje = (fin-principio).seconds/total
            porcentaje *= 0.98
            texto = ('%s\n%s\n%s--%s')%(asignatura, aula, principio, fin)
            yield texto, porcentaje
            principio = fin
            
            
##    def save_timetableXML(self):
##        print('GUARDANDO XML')
##
##        #solo guardo si se aplica algún filtro(y modifico lo cargado)
##        if self.listaAulas != '' or self.listaAsignaturas != '':
##            #Inicializaciones
##            doc = etree.parse('datos/outfile_nuevo_solucion.xml')
##            root = doc.getroot()
##
##            #Primero recorro el archivo borrando lo cargado anteriormente
##
##
##
##            #Añado al archivo la nueva disposición del horario
##            
##            #Soluciones
##            etree.SubElement(root, "SolutionGroups")
##            solucion = etree.SubElement(root[1], "SolutionGroup", id="fromHorario")
##
##            #Metadata
##            metadata = etree.SubElement(solucion, "Metadata")
##            name = etree.SubElement(metadata, "Name")
##            name.text = "Horarios de Informática"
##            contributor = etree.SubElement(metadata, "Contributor")
##            contributor.text = "Jose Ramon Vejo"
##            date = etree.SubElement(metadata, "Date")
##            date.text = "05/03/2017"
##            country = etree.SubElement(metadata, "Country")
##            country.text = "Spain"
##
##
##            #Anado la solucion
##            solve = etree.SubElement(solucion, "Solution")
##            reference = etree.SubElement(solve, "Reference")
##            reference.text = "Horarios de Informática"
##            events = etree.SubElement(solve, "Events")
##
##            #Recorro el arbol guardando el horario
##            event = etree.SubElement(events, "Event", Reference="Horarios de Informática")
##
##            duration = etree.SubElement(event, "Duration")
##            duration.text = "1"
##            time  = etree.SubElement(event, "Time", Reference="Reference Time")
##
##            #Guardo la solucion en el fichero
##            outFile = open('datos/outfile_nuevo_solucion.xml', 'wb')
##            doc.write(outFile)
##            print('XML GUARDADO')
##            print('Estos son los datos guardados')
##            #print(etree.tostring(root))

        



