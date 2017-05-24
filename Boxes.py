from kivy.uix.floatlayout import FloatLayout
from datetime import timedelta
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.button import Button
from kivy.uix.dropdown import DropDown
from kivy.uix.listview import ListView
from boton import Boton
from excel import excel
from lxml import etree

class Boxes(FloatLayout):
    '''Se crea el entorno visual del horario con Kivi'''
    numPulsaciones = 0
    i = 0
    j = 0
    k = 0
    l = 0
    primero = []
    segundo = []
    timeTable = ''
    filterLoad = set()
    filterTotal = set()
    
    def __init__(self, horarioPrincipal, filterTotal, **kwargs):
        ''' Inicialización del horario '''

        timeTable = horarioPrincipal
        filterLoad = filterTotal
        
        super(Boxes, self).__init__(**kwargs)

        #Fichero de lectura
        doc = etree.parse('datos/outfile_nuevo_solucion.xml')

        
        #añado los botones al horario
        for dia in horarioPrincipal.dias:
            bx_m = BoxLayout(orientation='vertical')#mañana
            bx_t = BoxLayout(orientation='vertical')#tarde

            #añado dias
            nombre = dia
            btn = Boton(text=nombre, size_hint_y=0.02)
            btn.setIdent(nombre)
            bx_m.add_widget(btn)

            btn = Boton(text=nombre, size_hint_y=0.02)
            btn.setIdent(nombre)
            bx_t.add_widget(btn)
            
            
            #Botones de mañana
            hour = 9
            for texto, porcentaje, asignaturaID, aulaID in horarioPrincipal.clases_manana(nombre):
                btn = Boton(text = texto, size_hint_y = porcentaje)
                btn.bind(on_release = lambda x:self.intercambia(horario=horarioPrincipal))
                btn.setIdent(hour)
                btn.setAsigID(asignaturaID)
                btn.setAulaID(aulaID)
                bx_m.add_widget(btn)
                hour = hour +1
            self.ids['_morning'].add_widget(bx_m)

            #Botones de tarde
            hour = 15
            for texto, porcentaje in horarioPrincipal.clases_tarde(nombre):
                btn = Boton(text = texto, size_hint_y = porcentaje)
                btn.bind(on_release = lambda x:self.intercambia(horario=horarioPrincipal))
                btn.setIdent(hour)
                bx_t.add_widget(btn)
                hour = hour +1
            self.ids['_afternoon'].add_widget(bx_t)

        #Añado los desplegables de la primera pantalla   
        profes = DropDown()
        aulas = DropDown()
        asigns = DropDown()
        curs = DropDown()
        button = Button()

        print('LEYENDO DATOS DESPLEGABLES')
        timegroups = doc.getroot().find('Instances')
        timegroups = timegroups.find('Instance')
        resources = timegroups.find('Resources')

        #Recupera profesores, aulas y cursos
        for child in resources:
            resource = child.find('ResourceType')

            '''Inserto los botones según los datos'''
            if resource is not None:
                resourceType = resource.get('Reference')
            
            if resourceType == 'Teacher':
                #Inserto los botones de los profesores
                #print(child.findtext('Name'))
                btn = Boton(text=child.findtext('Name'), size_hint_y=None, height=44)
                #Asigno el ID
                ident = child.get('Id')
                btn.setIdent(ident)
                
                # Mostrar el menu
                btn.bind(on_release=lambda btn: profes.select(btn.text))
                btn.bind(on_release=lambda btn: self.loadFilter(btn.text,filterLoad,btn.ident))

                # add el boton dentro del dropdown
                profes.add_widget(btn)

            if resourceType == 'Room':
                #Inserto los botones de las aulas
                #print(child.findtext('Name'))
                btn = Boton(text=child.findtext('Name'), size_hint_y=None, height=44)
                #Asigno el ID
                ident = child.get('Id')
                btn.setIdent(ident)
                
                # Mostrar el menu
                btn.bind(on_release=lambda btn: aulas.select(btn.text))
                btn.bind(on_release=lambda btn: self.loadFilter(btn.text,filterLoad,btn.ident))

                # add el boton dentro del dropdown
                aulas.add_widget(btn)

            if resourceType == 'Class':
                #Inserto los botones de los cursos
                #print(child.findtext('Name'))
                btn = Boton(text=child.findtext('Name'), size_hint_y=None, height=44)
                #Asigno el ID
                ident = child.get('Id')
                btn.setIdent(ident)
                
                # Mostrar el menu
                btn.bind(on_release=lambda btn: curs.select(btn.text))
                btn.bind(on_release=lambda btn: self.loadFilter(btn.text,filterLoad,btn.ident))

                # add el boton dentro del dropdown
                curs.add_widget(btn)

        #Añado las asignaturas
        events = timegroups.find('Events')
        
        for child in events:
            '''event = child.find('Event')'''

            '''Inserto las asignaturas'''
            if child.findtext('Name') is not None:
                #print(child.findtext('Name'))
                btn = Boton(text=child.findtext('Name'), size_hint_y=None, height=44)
                #Asigno el ID
                ident = child.get('Id')
                btn.setIdent(ident)
                
                # Mostrar el menu
                btn.bind(on_release=lambda btn: asigns.select(btn.text))
                btn.bind(on_release=lambda btn: self.loadFilter(btn.text,filterLoad,btn.ident))

                # add el boton dentro del dropdown
                asigns.add_widget(btn)

        #Añado un boton vacio a cada desplegable      
        # Profesores
        btn = Boton(text='Ninguno', size_hint_y=None, height=44)
        btn.bind(on_release=lambda btn: profes.select(btn.text))
        profes.add_widget(btn)
        # aulas
        btn = Boton(text='Ninguna', size_hint_y=None, height=44) 
        btn.bind(on_release=lambda btn: aulas.select(btn.text))
        aulas.add_widget(btn)
        #Cursos
        btn = Boton(text='Ninguno', size_hint_y=None, height=44) 
        btn.bind(on_release=lambda btn: curs.select(btn.text))
        curs.add_widget(btn)
        # asignaturas
        btn = Boton(text='Ninguna', size_hint_y=None, height=44) 
        btn.bind(on_release=lambda btn: asigns.select(btn.text))
        asigns.add_widget(btn)

        #añado los desplegables a la pantalla
        profesbutton = Button(text = 'Profesores', size_hint = (None, None), width = 330)
        profesbutton.bind(on_release=profes.open)
        aulasbutton= Button(text = 'Aulas', size_hint = (None, None), width = 200)
        aulasbutton.bind(on_release=aulas.open)
        asignsbutton= Button(text = 'Asignaturas', size_hint = (None, None), width = 400)
        asignsbutton.bind(on_release=asigns.open)
        cursosbutton= Button(text = 'Cursos', size_hint = (None, None), width = 250)
        cursosbutton.bind(on_release=curs.open)
        
        aulas.bind(on_select=lambda instance, x: setattr(aulasbutton, 'text', x))
        profes.bind(on_select=lambda instance, x: setattr(profesbutton, 'text', x))
        asigns.bind(on_select=lambda instance, x: setattr(asignsbutton, 'text', x))
        curs.bind(on_select=lambda instance, x: setattr(cursosbutton, 'text', x))

        #Añado botón de carga
        loadSelection = Button(text = 'Cargar selección', size_hint = (None, None), width = 200)
        loadSelection.bind(on_release=lambda btn: self.loadTimetable(horarioPrincipal))
        self.ids['_main'].add_widget(loadSelection)

        
        self.ids['_main'].add_widget(profesbutton)
        self.ids['_main'].add_widget(aulasbutton)
        self.ids['_main'].add_widget(asignsbutton)
        self.ids['_main'].add_widget(cursosbutton)

        #Separación y boton de exportacion
##        empty = Button(text = '', size_hint = (None, None))
##        self.ids['_main'].add_widget(empty)

        
    def loadFilter(self,text,filterLoad,ident):

        #Compruebo si es el primer filtrado
        if len(self.filterTotal) == 0:
            self.filterTotal.add(ident)
            
            filterLoad = filterLoad.union(self.filterTotal)
            
            listaParcial = set()
            listaPadres = set()
            listaHermanos = set()
            hermanoEncontrado = False
            
            # Recupero los valores seleccionados y cargo sus referencias xml
            doc = etree.parse('datos/outfile_nuevo_solucion.xml')
            timegroups = doc.getroot().find('Instances')
            timegroup = timegroups.find('Instance')
            events = timegroup.find('Events')

            print('FILTRO AL EMPEZAR')
            print(filterLoad)

            #Busco los profesores, clases y asignaturas que coinciden con el filtro
            for child in events:
                if child is not None:
                    name = child.find('Name')
                    key = child.get('Id')
                    if key in filterLoad:
                        resources = child.find('Resources')
                        if resources is not None:
                            for nice in resources:
                                if nice.get('Reference') is not None:
                                    if nice.get('Reference') in filterLoad:
                                        print('YA ESTA')
                                    else:
                                        listaParcial.add(nice.get('Reference'))
     
                    else:
                        resources = child.find('Resources')
                        listaHermanos = set()
                        if resources is not None:
                            for nice in resources:
                                if nice.get('Reference') is not None:
                                    if nice.get('Reference') in filterLoad:
                                        print('ENCONTRADO')
                                        listaPadres.add(child.get('Id'))
                                        hermanoEncontrado = True
                                    else:
                                        listaHermanos.add(nice.get('Reference'))
                                        
                            if hermanoEncontrado == True:
                                print('GUARDO PADRE E HIJOS')
                                print('listaPadres')
                                print(listaPadres)
                                print('listaHermanos')
                                print(listaHermanos)
                                listaParcial = listaParcial.union(listaPadres)
                                listaParcial = listaParcial.union(listaHermanos)
                                hermanoEncontrado = False

            listaAsignaturas = set()

            #Actualizo el filtro
            filterLoad = filterLoad.union(listaParcial)
            print('FILTRO EN EL MEDIO')
            print(filterLoad)        
            
            #Busco la referencia a las aulas en la solucion
            solutionGroups = doc.getroot().find('SolutionGroups')
            solutionGroup = solutionGroups.find('SolutionGroup')
            solution = solutionGroup.find('Solution')
            events = solution.find('Events')

            for child in events:
                resources = child.find('Resources')
                if child.get('Reference') in filterLoad:
                    for nice in resources:
                        listaParcial.add(nice.get('Reference'))
                else:
                    for nice in resources:
                        if nice.get('Reference') in filterLoad:
                            listaAsignaturas.add(child.get('Reference'))

                            #Con la lista de asignatutas, recupero los profesores y cursos
                            timegroups = doc.getroot().find('Instances')
                            timegroup = timegroups.find('Instance')
                            events = timegroup.find('Events')

                            for child in events:
                                if child is not None:
                                    name = child.find('Name')
                                    key = child.get('Id')
                                    if key in listaAsignaturas:
                                        resources = child.find('Resources')
                                        for nice in resources:
                                            listaParcial.add(nice.get('Reference'))
                                    
            print('filterTotal AL FINAL')
            print(self.filterTotal)
            #Salvo el filtro         
            self.filterLoad = filterLoad.union(listaParcial)
            print('FILTRO AL FINAL')
            print(self.filterLoad)
            
            #Elimino los desplegables antiguos
            self.ids['_main'].remove_widget(self.ids['_main'].children[3])
            self.ids['_main'].remove_widget(self.ids['_main'].children[2])
            self.ids['_main'].remove_widget(self.ids['_main'].children[1])
            self.ids['_main'].remove_widget(self.ids['_main'].children[0])

            #Los actualizo con el filtro
            profes = DropDown()
            aulas = DropDown()
            asigns = DropDown()
            curs = DropDown()
            button = Button()
            texto = ''
            identificador = ''
            
            timegroups = doc.getroot().find('Instances')
            timegroups = timegroups.find('Instance')
            resources = timegroups.find('Resources')

            #Recupera profesores, aulas y cursos
            for child in resources:
                resource = child.find('ResourceType')

                '''Inserto los botones según los datos'''
                if resource is not None:
                    resourceType = resource.get('Reference')

                #Actualizo el desplegable de los profesores
                if resourceType == 'Teacher':
                    if child.get('Id') in self.filterLoad:
                        btn = Boton(text=child.findtext('Name'), size_hint_y=None, height=44)
                        ident = child.get('Id')
                        btn.setIdent(ident)
                        btn.bind(on_release=lambda btn: profes.select(btn.text))
                        btn.bind(on_release=lambda btn: self.loadFilter(btn.text,filterLoad,btn.ident))
                        profes.add_widget(btn)
                        if child.get('Id') in self.filterTotal:
                            texto = child.findtext('Name')
                            identificador = 'Prof'
                        
                #Actualizo el desplegable de las aulas
                if resourceType == 'Room':
                    if child.get('Id') in self.filterLoad:
                        btn = Boton(text=child.findtext('Name'), size_hint_y=None, height=44)
                        ident = child.get('Id')
                        btn.setIdent(ident)
                        btn.bind(on_release=lambda btn: aulas.select(btn.text))
                        btn.bind(on_release=lambda btn: self.loadFilter(btn.text,filterLoad,btn.ident))
                        aulas.add_widget(btn)
                        if child.get('Id') in self.filterTotal:
                            texto = child.findtext('Name')
                            identificador = 'Aula'

                #Actualizo el desplegable de los cursos
                if resourceType == 'Class':
                    if child.get('Id') in self.filterLoad:
                        btn = Boton(text=child.findtext('Name'), size_hint_y=None, height=44)
                        ident = child.get('Id')
                        btn.setIdent(ident)
                        btn.bind(on_release=lambda btn: curs.select(btn.text))
                        btn.bind(on_release=lambda btn: self.loadFilter(btn.text,filterLoad,btn.ident))
                        curs.add_widget(btn)
                        if child.get('Id') in self.filterTotal:
                            texto = child.findtext('Name')
                            identificador = 'Curso'

            #Añado las asignaturas
            events = timegroups.find('Events')
            
            for child in events:
                if child.findtext('Name') is not None:
                    if child.get('Id') in self.filterLoad:
                        btn = Boton(text=child.findtext('Name'), size_hint_y=None, height=44)
                        ident = child.get('Id')
                        btn.setIdent(ident)
                        btn.bind(on_release=lambda btn: asigns.select(btn.text))
                        btn.bind(on_release=lambda btn: self.loadFilter(btn.text,filterLoad,btn.ident))
                        asigns.add_widget(btn)
                        if child.get('Id') in self.filterTotal:
                            texto = child.findtext('Name')
                            identificador = 'Asig'

            #Añado un boton vacio a cada desplegable      
            # Profesores
            btn = Boton(text='Ninguno', size_hint_y=None, height=44)
            btn.bind(on_release=lambda btn: profes.select(btn.text))
            profes.add_widget(btn)
            # aulas
            btn = Boton(text='Ninguna', size_hint_y=None, height=44) 
            btn.bind(on_release=lambda btn: aulas.select(btn.text))
            aulas.add_widget(btn)
            #Cursos
            btn = Boton(text='Ninguno', size_hint_y=None, height=44) 
            btn.bind(on_release=lambda btn: curs.select(btn.text))
            curs.add_widget(btn)
            # asignaturas
            btn = Boton(text='Ninguna', size_hint_y=None, height=44) 
            btn.bind(on_release=lambda btn: asigns.select(btn.text))
            asigns.add_widget(btn)

            #añado los desplegables a la pantalla, seteando el texto
            if identificador == 'Prof':
                profesbutton = Button(text = texto, size_hint = (None, None), width = 330)
                profesbutton.bind(on_release=profes.open)
            else:
                profesbutton = Button(text = 'Profesores', size_hint = (None, None), width = 330)
                profesbutton.bind(on_release=profes.open)

            if identificador == 'Aula':
                aulasbutton= Button(text = texto, size_hint = (None, None), width = 200)
                aulasbutton.bind(on_release=aulas.open)
            else:
                aulasbutton= Button(text = 'Aulas', size_hint = (None, None), width = 200)
                aulasbutton.bind(on_release=aulas.open)

            if identificador == 'Asig':
                asignsbutton= Button(text = texto, size_hint = (None, None), width = 400)
                asignsbutton.bind(on_release=asigns.open)
            else:
                asignsbutton= Button(text = 'Asignaturas', size_hint = (None, None), width = 400)
                asignsbutton.bind(on_release=asigns.open)

            if identificador == 'Curso':
                cursosbutton= Button(text = texto, size_hint = (None, None), width = 250)
                cursosbutton.bind(on_release=curs.open)
            else:
                cursosbutton= Button(text = 'Cursos', size_hint = (None, None), width = 250)
                cursosbutton.bind(on_release=curs.open)
            
            aulas.bind(on_select=lambda instance, x: setattr(aulasbutton, 'text', x))
            profes.bind(on_select=lambda instance, x: setattr(profesbutton, 'text', x))
            asigns.bind(on_select=lambda instance, x: setattr(asignsbutton, 'text', x))
            curs.bind(on_select=lambda instance, x: setattr(cursosbutton, 'text', x))
            
            self.ids['_main'].add_widget(profesbutton)
            self.ids['_main'].add_widget(aulasbutton)
            self.ids['_main'].add_widget(asignsbutton)
            self.ids['_main'].add_widget(cursosbutton)

        else:
            #Ya se ha filtrado una vez, se añade el nuevo filtro
            self.filterLoad.add(ident)
            self.filterTotal.add(ident)

        print('filterTotal AL FINAL')
        print(self.filterTotal)
        print('FILTRO AL FINAL')
        print(self.filterLoad)
        self.filterTotal = self.filterLoad
                
    
    def loadTimetable(self,horarioPrincipal):

        print('Filtro seleccionado para la carga')
        print(self.filterTotal)

        filterProf = ''
        filterAula = ''
        filterAsig = ''
        filterCurs = ''
        listaParcial = []
        timeParcial = []
        asigParcial = []

        # Recupero los valores seleccionados y cargo sus referencias xml
        doc = etree.parse('datos/outfile_nuevo_solucion.xml')
        solutionGroups = doc.getroot().find('SolutionGroups')
        solutionGroup = solutionGroups.find('SolutionGroup')
        solution = solutionGroup.find('Solution')
        events = solution.find('Events')

        #Rocorro la solución y pinto el horario según lo seleccionado
        for child in events:
            time = child.find('Time')
            print(time)
            if time is not None:
                timeReference = time.get('Reference')
                resources = child.find('Resources')
                if child.get('Reference') in self.filterTotal:
                    for nice in resources:
                        listaParcial.append(nice.get('Reference'))
                        timeParcial.append(timeReference)
                        asigParcial.append(child.get('Reference'))
                        
        print('listaParcial')
        print(listaParcial)
        print('timeParcial')
        print(timeParcial)
        print('asigParcial')
        print(asigParcial)
        
        print('self.filterTotal')
        print(self.filterTotal)

        print('Cargo los datos de las listas')
        print(horarioPrincipal)
        print('horarioPrincipal.dias')
        print(horarioPrincipal.dias)

        print('horarioPrincipal.h_dia')
        print(horarioPrincipal.h_dia)

        print('horarioPrincipal.inicio')
        print(horarioPrincipal.inicio)

        #Crear copia de las listas con los nombres

        #inserto según los tiempos
        pos = 0
        
        for element in timeParcial:
            print(element)
            dia = element[:len(element)-1]
            hora = int(element[len(element)-1:len(element)]) + 8
            
            print('dia')
            print(dia)
            print('hora')
            print(hora)
            print('pos')
            print(pos)
            print('asigParcial[pos]')
            print(asigParcial[pos])
            print('listaParcial[pos]')
            print(listaParcial[pos])

            if hora < 15:
                #Inserto los datos por la mañana
                for i in range(len(self.ids['_morning'].children)):
                    print('self.ids[_morning].children[i]')
                    print(self.ids['_morning'].children[i])

                    #Busco el día
                    for boton in range(len(self.ids['_morning'].children[0].children)):
                        print('self.ids[_morning].children[0].children[0].getIdent()')
                        print(self.ids['_morning'].children[i].children[boton].getIdent())
                        #Dia encontrado
                        if self.ids['_morning'].children[i].children[boton].getIdent() == dia:
                            #Busco la hora
                            for j in range(len(self.ids['_morning'].children[0].children)):
                                if self.ids['_morning'].children[i].children[j].getIdent() == hora:
                                    #Hora encontrada, pongo los datos
                                    self.ids['_morning'].children[i].children[j].setAsigID(asigParcial[pos])
                                    self.ids['_morning'].children[i].children[j].setAulaID(listaParcial[pos])
                                    texto = ('%s\n%s\n%s--%s')%(asigParcial[pos], listaParcial[pos], timedelta(hours=hora), timedelta(hours=hora+1))
                                    self.ids['_morning'].children[i].children[j].setText(texto)
                            

            else:
                #inserto los datos por la tarde
                for i in range(len(self.ids['_morning'].children)):
                    print('self.ids[_afternoon].children[i]')
                    print(self.ids['_afternoon'].children[i])

                    #Busco el día
                    for boton in range(len(self.ids['_morning'].children[0].children)):
                        print('self.ids[_afternoon].children[0].children[0].getIdent()')
                        print(self.ids['_afternoon'].children[i].children[boton].getIdent())
                        #Dia encontrado
                        if self.ids['_afternoon'].children[i].children[boton].getIdent() == dia:
                            #Busco la hora
                            for j in range(len(self.ids['_afternoon'].children[0].children)):
                                if self.ids['_afternoon'].children[i].children[j].getIdent() == hora:
                                    #Hora encontrada, pongo los datos
                                    self.ids['_afternoon'].children[i].children[j].setAsigID(asigParcial[pos])
                                    self.ids['_afternoon'].children[i].children[j].setAulaID(listaParcial[pos])
                                    texto = ('%s\n%s\n%s--%s')%(asigParcial[pos], listaParcial[pos], timedelta(hours=hora), timedelta(hours=hora+1))
                                    self.ids['_afternoon'].children[i].children[j].setText(texto)

            
            #horarioPrincipal.incluye_hora(dia,asigParcial[pos],asigParcial[pos],listaParcial[pos],listaParcial[pos],timedelta(hours=hora), timedelta(hours=hora+1))
            pos = pos +1
            
        #len(timeParcial) int('42')

        #for dia in ['Lunes','Martes','Miercoles','Jueves','Viernes']:

        #añado los botones al horario
##        for dia in horarioPrincipal.dias:
##            nombre = dia
##
##            print('self.ids[_morning]')
##            print(self.ids['_morning'])
##            print('self.ids[_morning].children[0].children[0].text')
##            print(self.ids['_morning'].children[0].children[0].text)
##            print('self.ids[_morning].children[0].children[0].getAsigID()')
##            print(self.ids['_morning'].children[0].children[0].getAsigID())
##            print('self.ids[_morning].children[0].children[0].getAulaID()')
##            print(self.ids['_morning'].children[0].children[0].getAulaID())            
##            
##            #Botones de mañana
##            hour = 14
##            for texto, porcentaje in horarioPrincipal.clases_manana(nombre):


    
    def intercambia(self, horario):

        # Recupero el id de la ventana activa
        ids = self.ids['_screen_manager'].current
        
        if ids == 'dia':
            ids = '_morning'
        else:
            ids = '_afternoon'

        # Recorro la ventana guardando los pulsados
        # Busco el primero, busco el segundo, y los intercambio
        if self.numPulsaciones == 0:
            for child in self.ids[ids].children:
                self.i = self.i + 1
                self.j = 0
                for grandchild in child.children:
                    self.j = self.j + 1
                    if grandchild.select == 2:
                        self.primero = grandchild
                        self.numPulsaciones = self.numPulsaciones + 1
                        print(grandchild)
                        print(self.primero)
                        print(grandchild.text)
                        print(grandchild.ident)
                        print(grandchild.select)
                        print('i:'+str(self.i))
                        print('j:'+str(self.j))
                        
        elif self.numPulsaciones == 1:
            a = ''
            for child in self.ids[ids].children:
                self.k = self.k + 1
                self.l = 0
                for grandchild in child.children:
                    self.l = self.l + 1
                    if grandchild.select == 2:
                        if grandchild != self.primero:
                            self.segundo = grandchild
                            a = grandchild.text.splitlines()
                            self.numPulsaciones = self.numPulsaciones + 1

                            #Intercambio el primero
                            print('Texto 1:')
                            print(self.primero.text)
                            texto = self.primero.text.splitlines()
                            grandchild.text = ('%s\n%s\n%s')%(texto[0], texto[1], a[2])
                            grandchild.select = 0
                            grandchild.disabled = False
                            grandchild.background_color = [.5,.5,.5,1]
                            

        if self.numPulsaciones == 2:
            for child in self.ids[ids].children:
                self.k = self.k + 1
                self.l = 0
                for grandchild in child.children:
                    self.l = self.l + 1
                    if grandchild.select == 2:
                        if grandchild != self.segundo:
                            #Intercambio el segundo
                            print('Texto 2:')
                            print(a)
                            grandchild.text = ('%s\n%s\n%s')%(a[0], a[1], texto[2])                            
                            grandchild.select = 0
                            grandchild.disabled = False
                            grandchild.background_color = [.5,.5,.5,1]
                            self.numPulsaciones = self.numPulsaciones + 1

            # Recorro la ventana contraria por si provenía de allí
            ids = self.ids['_screen_manager'].current

            if ids == 'dia':
                ids = '_afternoon'
            else:
                ids = '_morning'

            #Compruebo que si se han intercambiado entre mañana y tarde
            for child in self.ids[ids].children:
                for grandchild in child.children:
                    if grandchild.select == 2:
                        if grandchild != self.segundo:
                            #Intercambio el segundo
                            grandchild.text = ('%s\n%s\n%s')%(a[0], a[1], texto[2])  
                            grandchild.select = 0
                            grandchild.disabled = False
                            grandchild.background_color = [.5,.5,.5,1]

            self.numPulsaciones = 0
