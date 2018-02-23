from kivy.uix.floatlayout import FloatLayout
from datetime import timedelta
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.button import Button
from kivy.uix.dropdown import DropDown
from kivy.uix.listview import ListView
from boton import Boton
from boton2 import Boton2
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
    idAula = ''
    nombreAula = ''
    inicio = 0
    aulas = DropDown()
    documento = ''
    lastIdentificador = ''
    numIncidences = 10
    
    def __init__(self, horarioPrincipal, filterTotal, documento, **kwargs):
        ''' Inicialización del horario '''

        timeTable = horarioPrincipal
        filterLoad = filterTotal
        self.documento = documento 
        
        super(Boxes, self).__init__(**kwargs)

        #Fichero de lectura/escritura
        doc = etree.parse(self.documento)

        
        #añado los botones al horario
        for dia in horarioPrincipal.dias:
            bx_m = BoxLayout(orientation='vertical')#mañana
            bx_t = BoxLayout(orientation='vertical')#tarde

            #añado las cabeceras (dias)
            nombre = dia
            btn = Boton(text=nombre, size_hint_y=0.02)
            btn.setIdent(nombre)
            bx_m.add_widget(btn)

            btn = Boton(text=nombre, size_hint_y=0.02)
            btn.setIdent(nombre)
            bx_t.add_widget(btn)
            
            
            #Botones de mañana (Clase/Aula/Hora)
            hour = 9
            for texto, porcentaje, asignaturaID, aulaID in horarioPrincipal.clases_manana(nombre):
                btn = Boton(text = texto, size_hint_y = porcentaje)
                btn.bind(on_release = lambda x:self.intercambia(horario=horarioPrincipal))
                btn.setIdent(hour)
                btn.setAsigID(asignaturaID)
                btn.setAulaID(aulaID)
                btn.text_size = (300, 100)
                bx_m.add_widget(btn)
                hour = hour +1
            self.ids['_morning'].add_widget(bx_m)

            #Botones de tarde (Clase/Aula/Hora)
            hour = 15
            for texto, porcentaje in horarioPrincipal.clases_tarde(nombre):
                btn = Boton(text = texto, size_hint_y = porcentaje)
                btn.bind(on_release = lambda x:self.intercambia(horario=horarioPrincipal))
                btn.setIdent(hour)
                btn.text_size = (300, 100)
                bx_t.add_widget(btn)
                hour = hour +1
            self.ids['_afternoon'].add_widget(bx_t)

        #Añado los desplegables de selección a la primera pantalla   
        profes = DropDown()
        aulas = DropDown()
        asigns = DropDown()
        curs = DropDown()
        button = Button()

        profText = set()
        profId = set()
        aulaText = set()
        aulaId = set()
        cursoText = set()
        cursoId = set()
        asigText = set()
        asigId = set()
        
        timegroups = doc.getroot().find('Instances')
        timegroups = timegroups.find('Instance')
        resources = timegroups.find('Resources')

        #Recupera los datos de la fuente (profesores, aulas y cursos) del fichero
        for child in resources:
            resource = child.find('ResourceType')

            '''Inserto los botones según los datos'''
            if resource is not None:
                resourceType = resource.get('Reference')
            
            if resourceType == 'Teacher':
                #Recupero el nombre y el ID del profesor
                profText.add(child.findtext('Name'))
                profId.add(child.get('Id'))

            if resourceType == 'Room' or resourceType == 'Laboratory':
                #Recupero el nombre y el ID del aula
                aulaText.add(child.findtext('Name'))
                aulaId.add(child.get('Id'))
                

            if resourceType == 'Class':
                #Recupero el nombre y el ID del curso
                cursoText.add(child.findtext('Name'))
                cursoId.add(child.get('Id'))
                

        #Recupera los datos de la fuente (asignaturas)
        events = timegroups.find('Events')
        
        for child in events:
            
            '''Inserto las asignaturas'''
            if child.findtext('Name') is not None:
                #Recupero el nombre y el ID de la asignatura
                asigText.add(child.findtext('Name'))
                asigId.add(child.get('Id'))

        #Creo los botones de los desplegables ordenados
        profText = sorted(profText)
        profId = sorted(profId)
        aulaText = sorted(aulaText)
        aulaId = sorted(aulaId)
        cursoText = sorted(cursoText)
        cursoId = sorted(cursoId)
        asigText = sorted(asigText)
        asigId = sorted(asigId)

        for child in profText:
            #Inserto los botones de los profesores
            btn = Boton(text=child, size_hint_y=None, height=44)
            pos = profText.index(child)
            ident = profId[pos]
            btn.setIdent(ident)

            # Mostrar el menu
            btn.bind(on_release=lambda btn: profes.select(btn.text))
            btn.bind(on_release=lambda btn: self.loadFilter(btn.text,filterLoad,btn.ident))

            # add el boton dentro del dropdown
            profes.add_widget(btn)

        for child in aulaText:
            #Inserto los botones de las aulas
            btn = Boton(text=child, size_hint_y=None, height=44)
            pos = aulaText.index(child)
            ident = aulaId[pos]
            btn.setIdent(ident)
                
            # Mostrar el menu
            btn.bind(on_release=lambda btn: aulas.select(btn.text))
            btn.bind(on_release=lambda btn: self.loadFilter(btn.text,filterLoad,btn.ident))

            # add el boton dentro del dropdown
            aulas.add_widget(btn)

        for child in cursoText:
            #Inserto los botones de los cursos
            btn = Boton(text=child, size_hint_y=None, height=44)
            pos = cursoText.index(child)
            ident = cursoId[pos]
            btn.setIdent(ident)
                
            # Mostrar el menu
            btn.bind(on_release=lambda btn: curs.select(btn.text))
            btn.bind(on_release=lambda btn: self.loadFilter(btn.text,filterLoad,btn.ident))

            # add el boton dentro del dropdown
            curs.add_widget(btn)

        for child in asigText:
            #Inserto los botones de las asignaturas
            btn = Boton(text=child, size_hint_y=None, height=44)
            pos = asigText.index(child)
            ident = asigId[pos]
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

        #Formato para los desplegables
        aulasbutton= Button(text = 'Aulas', size_hint = (None, None), width = 200)
        aulasbutton.bind(on_release=aulas.open)
        profesbutton = Button(text = 'Profesores', size_hint = (None, None), width = 330)
        profesbutton.bind(on_release=profes.open)  
        asignsbutton= Button(text = 'Asignaturas', size_hint = (None, None), width = 400)
        asignsbutton.bind(on_release=asigns.open)
        cursosbutton= Button(text = 'Cursos', size_hint = (None, None), width = 250)
        cursosbutton.bind(on_release=curs.open)

        profes.bind(on_select=lambda instance, x: setattr(profesbutton, 'text', x))
        aulas.bind(on_select=lambda instance, x: setattr(aulasbutton, 'text', x))
        asigns.bind(on_select=lambda instance, x: setattr(asignsbutton, 'text', x))
        curs.bind(on_select=lambda instance, x: setattr(cursosbutton, 'text', x))
        
        #Añado botón de carga y los desplegables
        loadSelection = Button(text = 'Cargar selección', size_hint = (None, None), width = 200)
        loadSelection.bind(on_release=lambda btn: self.loadTimetable(horarioPrincipal))
        self.ids['_main'].add_widget(loadSelection)
        self.ids['_main'].add_widget(profesbutton)
        self.ids['_main'].add_widget(aulasbutton)
        self.ids['_main'].add_widget(asignsbutton)
        self.ids['_main'].add_widget(cursosbutton)
        
        #Cargo las incidencias en su pantalla desde el fichero
        doc = etree.parse(self.documento)
        solutionGroups = doc.getroot().find('SolutionGroups')
        solutionGroup = solutionGroups.find('SolutionGroup')
        solution = solutionGroup.find('Solution')
        report = solution.find('Report')
        resources = report.find('Resources')
        events = report.find('Events')

        nInc = 0

        for child in resources:
            reference = child.get('Reference')
            for nice in child:
                constraint = nice.get('Reference')
                text = ' tiene un conflicto de '
                cursos = timegroups.find('Resources')

                for binice in cursos:
                    resource = binice.find('ResourceType')

                    '''Inserto los botones según los datos'''
                    if resource is not None:
                        resourceType = resource.get('Reference')
                        resourceName = binice.findtext('Name')
                    
                    if resourceType == 'Class' and reference == binice.get('Id'):
                        #Inserto un botón con la incidencia y sus datos
                        constraints = timegroups.find('Constraints')
                        
                        if constraints is not None and nInc < self.numIncidences:
                            for child in constraints:
                                if child.get('Id') == constraint:
                                    nInc = nInc +1
                                    btn = Button(text = resourceName + text + child.findtext('Name'))
                                    self.ids['_incidences'].add_widget(btn)
        
        for child in events:
            reference = child.get('Reference')
            for nice in child:
                constraint = nice.get('Reference')
                text = ' tiene un conflicto de '
                asig = timegroups.find('Events')

                for binice in asig:

                    '''Inserto los botones según los datos'''
                    if binice.findtext('Name') is not None:
                        resourceName = binice.findtext('Name')
                    
                        if reference == binice.get('Id'):
                            #Inserto un botón con la incidencia y sus datos
                            constraints = timegroups.find('Constraints')
                        
                            if constraints is not None and nInc < self.numIncidences:
                                for child in constraints:
                                    if child.get('Id') == constraint:
                                        nInc = nInc +1
                                        btn = Button(text = resourceName + text + child.findtext('Name'))
                                        self.ids['_incidences'].add_widget(btn)
                                                
        self.children[1].children[0].children[2].text = 'Acción: Intercambiar asignaturas'
        self.children[0].children[0].children[4].text = 'Horario de mañana'
        #print(self.filterTotal)
            
    def resetDropdown(self):
        #Añado los desplegables de selección a la primera pantalla   
        profes = DropDown()
        aulas = DropDown()
        asigns = DropDown()
        curs = DropDown()
        button = Button()

        profText = set()
        profId = set()
        aulaText = set()
        aulaId = set()
        cursoText = set()
        cursoId = set()
        asigText = set()
        asigId = set()

        filterLoad = self.filterTotal
        #print("Filtro total:", filterLoad)
        #Fichero de lectura/escritura
        doc = etree.parse(self.documento)
        timegroups = doc.getroot().find('Instances')
        timegroups = timegroups.find('Instance')
        resources = timegroups.find('Resources')

        #Recupera los datos de la fuente (profesores, aulas y cursos) del fichero
        for child in resources:
            resource = child.find('ResourceType')

            '''Inserto los botones según los datos'''
            if resource is not None:
                resourceType = resource.get('Reference')
            
            if resourceType == 'Teacher':
                #Recupero el nombre y el ID del profesor
                profText.add(child.findtext('Name'))
                profId.add(child.get('Id'))

            if resourceType == 'Room' or resourceType == 'Laboratory':
                #Recupero el nombre y el ID del aula
                aulaText.add(child.findtext('Name'))
                aulaId.add(child.get('Id'))

            if resourceType == 'Class':
                #Recupero el nombre y el ID del curso
                cursoText.add(child.findtext('Name'))
                cursoId.add(child.get('Id'))

        #Recupera los datos de la fuente (asignaturas)
        events = timegroups.find('Events')
        
        for child in events:
            
            '''Inserto las asignaturas'''
            if child.findtext('Name') is not None:
                #Recupero el nombre y el ID de la asignatura
                asigText.add(child.findtext('Name'))
                asigId.add(child.get('Id'))

        #Creo los botones de los desplegables ordenados
        profText = sorted(profText)
        profId = sorted(profId)
        aulaText = sorted(aulaText)
        aulaId = sorted(aulaId)
        cursoText = sorted(cursoText)
        cursoId = sorted(cursoId)
        asigText = sorted(asigText)
        asigId = sorted(asigId)

        for child in profText:
            #Inserto los botones de los profesores
            btn = Boton(text=child, size_hint_y=None, height=44)
            pos = profText.index(child)
            ident = profId[pos]
            btn.setIdent(ident)

            # Mostrar el menu
            btn.bind(on_release=lambda btn: profes.select(btn.text))
            btn.bind(on_release=lambda btn: self.loadFilter(btn.text,filterLoad,btn.ident))

            # add el boton dentro del dropdown
            profes.add_widget(btn)

        for child in aulaText:
            #Inserto los botones de las aulas
            btn = Boton(text=child, size_hint_y=None, height=44)
            pos = aulaText.index(child)
            ident = aulaId[pos]
            btn.setIdent(ident)
                
            # Mostrar el menu
            btn.bind(on_release=lambda btn: aulas.select(btn.text))
            btn.bind(on_release=lambda btn: self.loadFilter(btn.text,filterLoad,btn.ident))

            # add el boton dentro del dropdown
            aulas.add_widget(btn)

        for child in cursoText:
            #Inserto los botones de los cursos
            btn = Boton(text=child, size_hint_y=None, height=44)
            pos = cursoText.index(child)
            ident = cursoId[pos]
            btn.setIdent(ident)
                
            # Mostrar el menu
            btn.bind(on_release=lambda btn: curs.select(btn.text))
            btn.bind(on_release=lambda btn: self.loadFilter(btn.text,filterLoad,btn.ident))

            # add el boton dentro del dropdown
            curs.add_widget(btn)

        for child in asigText:
            #Inserto los botones de las asignaturas
            btn = Boton(text=child, size_hint_y=None, height=44)
            pos = asigText.index(child)
            ident = asigId[pos]
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

        #Formato para los desplegables
        #profesbutton = Button(text = 'Profesores', size_hint = (None, None), width = 330)
        self.ids['_main'].children[3].bind(on_release=profes.open)
        self.ids['_main'].children[3].text = 'Profesores' 
        #aulasbutton= Button(text = 'Aulas', size_hint = (None, None), width = 200)
        self.ids['_main'].children[2].bind(on_release=aulas.open)
        self.ids['_main'].children[2].text = 'Aulas'
        #asignsbutton= Button(text = 'Asignaturas', size_hint = (None, None), width = 400)
        self.ids['_main'].children[1].bind(on_release=asigns.open)
        self.ids['_main'].children[1].text = 'Asignaturas'
        #cursosbutton= Button(text = 'Cursos', size_hint = (None, None), width = 250)
        self.ids['_main'].children[0].bind(on_release=curs.open)
        self.ids['_main'].children[0].text = 'Cursos'

        profes.bind(on_select=lambda instance, x: setattr(self.ids['_main'].children[3], 'text', x))
        aulas.bind(on_select=lambda instance, x: setattr(self.ids['_main'].children[2], 'text', x))
        asigns.bind(on_select=lambda instance, x: setattr(self.ids['_main'].children[1], 'text', x))
        curs.bind(on_select=lambda instance, x: setattr(self.ids['_main'].children[0], 'text', x))

        #Reseteo los valores del horario a vacío, tanto los textos como el estado del botón
        #print(self.filterTotal)
        #Borro los datos de la mañana para pintar de nuevo
        for i in range(len(self.ids['_morning'].children)):
            #Busco el día
            for boton in range(len(self.ids['_morning'].children[0].children)):
                #Busco la hora
                for j in range(len(self.ids['_morning'].children[0].children)):
                    if self.ids['_morning'].children[i].children[j].getIdent() not in ['Lunes','Martes','Miercoles','Jueves','Viernes']:
                        self.ids['_morning'].children[i].children[j].setAsigID('')
                        self.ids['_morning'].children[i].children[j].setAulaID('')
                        texto = self.ids['_morning'].children[i].children[j].getText()
                        salto = '\n'
                        cortado = texto.split(salto)
                        cortado[0] = 'Libre'
                        cortado[1] = 'Sin Aula'
                        texto = ('%s\n%s\n%s--%s')%('Libre', 'Sin Aula', timedelta(hours=8+j), timedelta(hours=9+j))
                        self.ids['_morning'].children[i].children[j].setText(salto.join(cortado))
                        self.ids['_morning'].children[i].children[j].background_color = [1,1,1,1]

        #Borro los datos de la tarde para pintar de nuevo
        for i in range(len(self.ids['_afternoon'].children)):
            #Busco el día
            for boton in range(len(self.ids['_afternoon'].children[0].children)):
                #Busco la hora
                for j in range(len(self.ids['_afternoon'].children[0].children)):
                    if self.ids['_afternoon'].children[i].children[j].getIdent() not in ['Lunes','Martes','Miercoles','Jueves','Viernes']:
                        self.ids['_afternoon'].children[i].children[j].setAsigID('')
                        self.ids['_afternoon'].children[i].children[j].setAulaID('')
                        texto = self.ids['_afternoon'].children[i].children[j].getText()
                        salto = '\n'
                        cortado = texto.split(salto)
                        cortado[0] = 'Libre'
                        cortado[1] = 'Sin Aula'
                        texto = ('%s\n%s\n%s--%s')%('Libre', 'Sin Aula', timedelta(hours=8+j), timedelta(hours=9+j))
                        self.ids['_afternoon'].children[i].children[j].setText(salto.join(cortado))
                        self.ids['_afternoon'].children[i].children[j].background_color = [1,1,1,1]
        
        self.ids['_main'].children[4].disabled = False
        self.ids['_main'].children[3].disabled = False
        self.ids['_main'].children[2].disabled = False
        self.ids['_main'].children[1].disabled = False
        self.ids['_main'].children[0].disabled = False

        #Pongo el texto en el botón que sirve como indicador
        self.children[1].children[0].children[0].text = self.children[1].children[0].children[0].text.split(':')[0] + ': Ninguno'
        
        self.lastIdentificador = ''
        self.filterTotal = set()
        self.filterLoad = set()        

    def loadFilter(self,text,filterLoad,ident):

        '''Método que recupera el filtro seleccionado en los desplegables y que se usa para cargar'''    
        filterLoad = set()
        filterLoad.add(ident)
        print("Al principio",filterLoad)
        #print("llamando a filterLoad",filterLoad)
        
        #Pongo el texto en el botón que sirve como indicador
        self.children[1].children[0].children[0].text = self.children[1].children[0].children[0].text.split(':')[0] + ': ' + text
 
        #filterLoad = filterLoad.union(self.filterTotal)
            
        listaParcial = set()
        listaPadres = set()
        listaHermanos = set()
        hermanoEncontrado = False
            
        # Recupero los valores seleccionados y cargo sus referencias xml
        doc = etree.parse(self.documento)
        timegroups = doc.getroot().find('Instances')
        timegroup = timegroups.find('Instance')
        events = timegroup.find('Events')

        #Si no filtro por aula, busco las asignaturas
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
                                    pass
                                    #else:
                                        #listaParcial.add(nice.get('Reference'))
         
                else:
                    resources = child.find('Resources')
                    listaHermanos = set()
                    if resources is not None:
                        for nice in resources:
                            if nice.get('Reference') is not None:
                                if nice.get('Reference') in filterLoad:
                                    listaPadres.add(child.get('Id'))
                                    hermanoEncontrado = True
                                    #else:
                                        #print('')
                                        #listaHermanos.add(nice.get('Reference'))
                                            
                        if hermanoEncontrado == True:
                            listaParcial = listaParcial.union(listaPadres)
                            listaParcial = listaParcial.union(listaHermanos)
                            hermanoEncontrado = False
        print("listaPadr",listaParcial,ident)    
        listaAsignaturas = set()

        #Salvo el filtro        
        self.filterLoad = listaParcial

        #Si no se ha encontrado (aula) lo añado directamente
        if self.filterLoad == set():
            self.filterLoad.add(ident)
            
        #Busco la referencia a las aulas en la solucion (guardo las asignaturas)
##        solutionGroups = doc.getroot().find('SolutionGroups')
##        solutionGroup = solutionGroups.find('SolutionGroup')
##        solution = solutionGroup.find('Solution')
##        events = solution.find('Events')
##
##        for child in events:
##            resources = child.find('Resources')
##            if child.get('Reference') in filterLoad:
##                if resources is not None:
##                    for nice in resources:
##                        listaParcial.add(nice.get('Reference'))
##            else:
##                if resources is not None:
##                    for nice in resources:
##                        if nice.get('Reference') in filterLoad:
##                            listaAsignaturas.add(child.get('Reference'))
##
##                            #Con la lista de asignatutas, recupero los profesores y cursos
##                            timegroups = doc.getroot().find('Instances')
##                            timegroup = timegroups.find('Instance')
##                            events = timegroup.find('Events')
##
##                            for child in events:
##                                if child is not None:
##                                    name = child.find('Name')
##                                    key = child.get('Id')
##                                    if key in listaAsignaturas:
##                                        resources = child.find('Resources')
##                                        for nice in resources:
##                                            listaParcial.add(nice.get('Reference'))
                                    
        #self.filterLoad = filterLoad.union(listaParcial)
            
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
                    print("Ident_dentro",ident,child.findtext('Name'))
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
                    print("Ident_dentro",ident)
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
                    print("Ident_dentro",ident)
                    btn.bind(on_release=lambda btn: curs.select(btn.text))
                    btn.bind(on_release=lambda btn: self.loadFilter(btn.text,filterLoad,btn.ident))
                    curs.add_widget(btn)
                    if child.get('Id') in self.filterTotal:
                        texto = child.findtext('Name')
                        identificador = 'Curso'
        print("texto:", texto,"identificador:",identificador)
        #Añado las asignaturas
        events = timegroups.find('Events')
        print("filterLoad", self.filterLoad, "filterTotal",self.filterTotal)    
        for child in events:
            if child.findtext('Name') is not None:
                if child.get('Id') in self.filterLoad:
                    btn = Boton(text=child.findtext('Name'), size_hint_y=None, height=44)
                    ident = child.get('Id')
                    btn.setIdent(ident)
                    print("Ident_fuera",ident)
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

        #if identificador == 'Prof' or self.lastIdentificador == 'Prof':
        self.ids['_main'].children[3].disabled = True
        #if identificador == 'Aula' or self.lastIdentificador == 'Aula':
        self.ids['_main'].children[2].disabled = True
        #if identificador == 'Asig' or self.lastIdentificador == 'Asig':
        self.ids['_main'].children[1].disabled = True
        #if identificador == 'Curso' or self.lastIdentificador == 'Curso':
        self.ids['_main'].children[0].disabled = True

        self.lastIdentificador = identificador

        self.filterLoad = self.filterLoad.union(listaAsignaturas)
        print("Lista total de asignaturas", self.filterLoad)
        self.filterTotal = self.filterLoad
                
    
    def loadTimetable(self,horarioPrincipal):

        '''Método que realiza la carga del horario según lo introducido en el filtro'''
        
        #Inicializaciones
        filterProf = ''
        filterAula = ''
        filterAsig = ''
        filterCurs = ''
        listaParcial = []
        timeParcial = []
        asigParcial = []
        self.ids['_main'].children[4].disabled = True

        # Recupero los valores seleccionados y cargo sus referencias xml
        doc = etree.parse(self.documento)
        solutionGroups = doc.getroot().find('SolutionGroups')
        solutionGroup = solutionGroups.find('SolutionGroup')
        solution = solutionGroup.find('Solution')
        events = solution.find('Events')

        #Recorro la solución y pinto el horario según lo seleccionado
        for child in events:
            time = child.find('Time')
            if time is not None:
                timeReference = time.get('Reference')
                resources = child.find('Resources')
                if child.get('Reference') in self.filterTotal:
                    if resources is not None:
                        for nice in resources:
                            if '' == nice.get('Reference'):
                                listaParcial.append('Sin aula')
                            else:
                                listaParcial.append(nice.get('Reference'))
                            timeParcial.append(timeReference)
                            asigParcial.append(child.get('Reference'))
                else:
                    if resources is not None:
                        for nice in resources:
                            if nice.get('Reference') in self.filterTotal:
                                listaParcial.append(nice.get('Reference'))
                                timeParcial.append(timeReference)
                                asigParcial.append(child.get('Reference'))
                        

        #Reseteo los valores del horario a vacío, tanto los textos como el estado del botón

        #Borro los datos de la mañana para pintar de nuevo
        for i in range(len(self.ids['_morning'].children)):
            #Busco el día
            for boton in range(len(self.ids['_morning'].children[0].children)):
                #Busco la hora
                for j in range(len(self.ids['_morning'].children[0].children)):
                    if self.ids['_morning'].children[i].children[j].getIdent() not in ['Lunes','Martes','Miercoles','Jueves','Viernes']:
                        self.ids['_morning'].children[i].children[j].setAsigID('')
                        self.ids['_morning'].children[i].children[j].setAulaID('')
                        texto = self.ids['_morning'].children[i].children[j].getText()
                        salto = '\n'
                        cortado = texto.split(salto)
                        cortado[0] = 'Libre'
                        cortado[1] = 'Sin Aula'
                        texto = ('%s\n%s\n%s--%s')%('Libre', 'Sin Aula', j, 1+j)
                        self.ids['_morning'].children[i].children[j].setText(salto.join(cortado))
                        self.ids['_morning'].children[i].children[j].background_color = [1,1,1,1]

        #Borro los datos de la tarde para pintar de nuevo
        for i in range(len(self.ids['_afternoon'].children)):
            #Busco el día
            for boton in range(len(self.ids['_afternoon'].children[0].children)):
                #Busco la hora
                for j in range(len(self.ids['_afternoon'].children[0].children)):
                    if self.ids['_afternoon'].children[i].children[j].getIdent() not in ['Lunes','Martes','Miercoles','Jueves','Viernes']:
                        self.ids['_afternoon'].children[i].children[j].setAsigID('')
                        self.ids['_afternoon'].children[i].children[j].setAulaID('')
                        texto = self.ids['_afternoon'].children[i].children[j].getText()
                        salto = '\n'
                        cortado = texto.split(salto)
                        cortado[0] = 'Libre'
                        cortado[1] = 'Sin Aula'
                        texto = ('%s\n%s\n%s--%s')%('Libre', 'Sin Aula', j, 1+j)
                        self.ids['_afternoon'].children[i].children[j].setText(salto.join(cortado))
                        self.ids['_afternoon'].children[i].children[j].background_color = [1,1,1,1]

                                        
        #Crear copia de las listas con los nombres
        nombresAsig = []
        nombresAulas = []
        timegroups = doc.getroot().find('Instances')
        timegroups = timegroups.find('Instance')
        resources = timegroups.find('Resources')
        events = timegroups.find('Events')

        #Nombres de las asignaturas
        for element in asigParcial:
            for child in events:
                '''Recupero el nombre de la asignatura'''
                if child.findtext('Name') is not None:
                    if element == child.get('Id'):
                        nombresAsig.append(child.findtext('Name'))

        #Nombres de las aulas
        for element in listaParcial:
            if 'Sin aula' == element:
                nombresAulas.append('Sin aula')
                
            for child in resources:
                resource = child.find('ResourceType')
                '''Inserto los botones según los datos'''
                if resource is not None:
                    resourceType = resource.get('Reference')
            
                if resourceType == 'Room' or resourceType == 'Laboratory':
                    if element == child.get('Id'):
                        nombresAulas.append(child.findtext('Name'))

        #inserto según los tiempos recuperados
        pos = 0
        #print("Lista parcial", listaParcial)
        #print("Asig parcial", asigParcial)
        #print("Time parcial", timeParcial)
        for element in timeParcial:
            dia = element[:len(element)-1]
            hora = int(element[len(element)-1:len(element)]) + 8

            if hora < 15:
                #Inserto los datos por la mañana
                for i in range(len(self.ids['_morning'].children)):
                    #Busco el día
                    for boton in range(len(self.ids['_morning'].children[0].children)):
                        #Dia encontrado
                        if self.ids['_morning'].children[i].children[boton].getIdent() == dia:
                            #Busco la hora
                            for j in range(len(self.ids['_morning'].children[0].children)):
                                if self.ids['_morning'].children[i].children[j].getIdent() == hora:
                                    #Hora encontrada, pongo los datos
                                    self.ids['_morning'].children[i].children[j].setAsigID(asigParcial[pos])
                                    self.ids['_morning'].children[i].children[j].setAulaID(listaParcial[pos])                                    
                                    texto = ('%s\n%s\n%s--%s')%(nombresAsig[pos], nombresAulas[pos], hora-8, hora-7)
                                    self.ids['_morning'].children[i].children[j].setText(texto)
            else:
                #inserto los datos por la tarde
                for i in range(len(self.ids['_morning'].children)):
                    #Busco el día
                    for boton in range(len(self.ids['_morning'].children[0].children)):
                        #Dia encontrado
                        if self.ids['_afternoon'].children[i].children[boton].getIdent() == dia:
                            #Busco la hora
                            for j in range(len(self.ids['_afternoon'].children[0].children)):
                                if self.ids['_afternoon'].children[i].children[j].getIdent() == hora:
                                    #Hora encontrada, pongo los datos
                                    self.ids['_afternoon'].children[i].children[j].setAsigID(asigParcial[pos])
                                    self.ids['_afternoon'].children[i].children[j].setAulaID(listaParcial[pos])
                                    texto = ('%s\n%s\n%s--%s')%(nombresAsig[pos], nombresAulas[pos], hora-8, hora-7)
                                    self.ids['_afternoon'].children[i].children[j].setText(texto)

            #Avanzo la posición de las listas
            pos = pos +1

    def saveTimetable(self):
        '''Método que guarda los datos del horario cargado con el filtro en el fichero'''
        print("QUE tengo en filter total", self.filterTotal)
        # Recupero los valores seleccionados y cargo sus referencias xml
        doc = etree.parse(self.documento)
        solutionGroups = doc.getroot().find('SolutionGroups')
        solutionGroup = solutionGroups.find('SolutionGroup')
        solution = solutionGroup.find('Solution')
        events = solution.find('Events')

        #Recorro la solución y borro los datos antiguos
        for child in events:
            resources = child.find('Resources')
            if child.get('Reference') in self.filterTotal:
                #Elimino el nodo
                child.getparent().remove(child)
            else:
                if resources is not None:
                    for nice in resources:
                        if nice.get('Reference') in self.filterTotal:
                            #Elimino el nodo
                            child.getparent().remove(child)

        #recorro el horario y inserto sus datos
        guardadoLunes = False
        guardadoMartes = False
        guardadoMiercoles = False
        guardadoJueves = False
        guardadoViernes = False

        #Guardo los datos de la mañana
        for i in range(len(self.ids['_morning'].children)):
            #Recorro los días
            for boton in range(len(self.ids['_morning'].children[0].children)):
                #Recorro las horas
                for j in range(len(self.ids['_morning'].children[0].children)):
                    if self.ids['_morning'].children[i].children[j].getIdent() == 'Lunes' and guardadoLunes == False:
                        for j in range(len(self.ids['_morning'].children[0].children)):
                            diaHora = self.ids['_morning'].children[i].children[j].getIdent()
                            asignatura = self.ids['_morning'].children[i].children[j].getAsigID()
                            aula = self.ids['_morning'].children[i].children[j].getAulaID()
                            self.ids['_morning'].children[i].children[j].background_color = [1,1,1,1]
                            #Compruebo que no sean horas libres
                            if len(asignatura) > 1:
                                #Añado la asignatura con su horario y aula
                                event = etree.SubElement(events, "Event")
                                event.set("Reference", asignatura)
                                duration = etree.SubElement(event, "Duration")
                                duration.text = "1"
                                time = etree.SubElement(event, "Time")
                                time.set("Reference", 'Lunes' + str(diaHora-8))
                                resources = etree.SubElement(event, "Resources")
                                resource = etree.SubElement(resources, "Resource")
                                resource.set("Reference", aula)
                                role = etree.SubElement(resource, "Role")
                                role.text = "Room"
                                guardadoLunes = True
                                
                    if self.ids['_morning'].children[i].children[j].getIdent() == 'Martes'and guardadoMartes == False:
                        for j in range(len(self.ids['_morning'].children[0].children)):
                            diaHora = self.ids['_morning'].children[i].children[j].getIdent()
                            asignatura = self.ids['_morning'].children[i].children[j].getAsigID()
                            aula = self.ids['_morning'].children[i].children[j].getAulaID()
                            self.ids['_morning'].children[i].children[j].background_color = [1,1,1,1]
                            #Compruebo que no sean horas libres
                            if len(asignatura) > 1:
                                #Añado la asignatura con su horario y aula
                                event = etree.SubElement(events, "Event")
                                event.set("Reference", asignatura)
                                duration = etree.SubElement(event, "Duration")
                                duration.text = "1"
                                time = etree.SubElement(event, "Time")
                                time.set("Reference", 'Martes' + str(diaHora-8))
                                resources = etree.SubElement(event, "Resources")
                                resource = etree.SubElement(resources, "Resource")
                                resource.set("Reference", aula)
                                role = etree.SubElement(resource, "Role")
                                role.text = "Room"
                                guardadoMartes = True
                                
                    if self.ids['_morning'].children[i].children[j].getIdent() == 'Miercoles' and guardadoMiercoles == False:
                        for j in range(len(self.ids['_morning'].children[0].children)):
                            diaHora = self.ids['_morning'].children[i].children[j].getIdent()
                            asignatura = self.ids['_morning'].children[i].children[j].getAsigID()
                            aula = self.ids['_morning'].children[i].children[j].getAulaID()
                            self.ids['_morning'].children[i].children[j].background_color = [1,1,1,1]
                            #Compruebo que no sean horas libres
                            if len(asignatura) > 1:
                                #Añado la asignatura con su horario y aula
                                event = etree.SubElement(events, "Event")
                                event.set("Reference", asignatura)
                                duration = etree.SubElement(event, "Duration")
                                duration.text = "1"
                                time = etree.SubElement(event, "Time")
                                time.set("Reference", 'Miercoles' + str(diaHora-8))
                                resources = etree.SubElement(event, "Resources")
                                resource = etree.SubElement(resources, "Resource")
                                resource.set("Reference", aula)
                                role = etree.SubElement(resource, "Role")
                                role.text = "Room"
                                guardadoMiercoles = True
                                
                    if self.ids['_morning'].children[i].children[j].getIdent() == 'Jueves' and guardadoJueves == False:
                        for j in range(len(self.ids['_morning'].children[0].children)):
                            diaHora = self.ids['_morning'].children[i].children[j].getIdent()
                            asignatura = self.ids['_morning'].children[i].children[j].getAsigID()
                            aula = self.ids['_morning'].children[i].children[j].getAulaID()
                            self.ids['_morning'].children[i].children[j].background_color = [1,1,1,1]
                            #Compruebo que no sean horas libres
                            if len(asignatura) > 1:
                                #Añado la asignatura con su horario y aula
                                event = etree.SubElement(events, "Event")
                                event.set("Reference", asignatura)
                                duration = etree.SubElement(event, "Duration")
                                duration.text = "1"
                                time = etree.SubElement(event, "Time")
                                time.set("Reference", 'Jueves' + str(diaHora-8))
                                resources = etree.SubElement(event, "Resources")
                                resource = etree.SubElement(resources, "Resource")
                                resource.set("Reference", aula)
                                role = etree.SubElement(resource, "Role")
                                role.text = "Room"
                                guardadoJueves = True
                                
                    if self.ids['_morning'].children[i].children[j].getIdent() == 'Viernes' and guardadoViernes == False:
                        for j in range(len(self.ids['_morning'].children[0].children)):
                            diaHora = self.ids['_morning'].children[i].children[j].getIdent()
                            asignatura = self.ids['_morning'].children[i].children[j].getAsigID()
                            aula = self.ids['_morning'].children[i].children[j].getAulaID()
                            self.ids['_morning'].children[i].children[j].background_color = [1,1,1,1]
                            #Compruebo que no sean horas libres
                            if len(asignatura) > 1:
                                #Añado la asignatura con su horario y aula
                                event = etree.SubElement(events, "Event")
                                event.set("Reference", asignatura)
                                duration = etree.SubElement(event, "Duration")
                                duration.text = "1"
                                time = etree.SubElement(event, "Time")
                                time.set("Reference", 'Viernes' + str(diaHora-8))
                                resources = etree.SubElement(event, "Resources")
                                resource = etree.SubElement(resources, "Resource")
                                resource.set("Reference", aula)
                                role = etree.SubElement(resource, "Role")
                                role.text = "Room"
                                guardadoViernes = True
                                
        guardadoLunes = False
        guardadoMartes = False
        guardadoMiercoles = False
        guardadoJueves = False
        guardadoViernes = False

        #guardo los datos por de la tarde
        for i in range(len(self.ids['_afternoon'].children)):
            #Recorro los días
            for boton in range(len(self.ids['_afternoon'].children[0].children)):
                #Recorro las horas
                for j in range(len(self.ids['_afternoon'].children[0].children)):
                    if self.ids['_afternoon'].children[i].children[j].getIdent() == 'Lunes' and guardadoLunes == False:
                        for j in range(len(self.ids['_afternoon'].children[0].children)):
                            diaHora = self.ids['_afternoon'].children[i].children[j].getIdent()
                            asignatura = self.ids['_afternoon'].children[i].children[j].getAsigID()
                            aula = self.ids['_afternoon'].children[i].children[j].getAulaID()
                            self.ids['_afternoon'].children[i].children[j].background_color = [1,1,1,1]
                            #Compruebo que no sean horas libres
                            if len(asignatura) > 1:
                                #Añado la asignatura con su horario y aula
                                event = etree.SubElement(events, "Event")
                                event.set("Reference", asignatura)
                                duration = etree.SubElement(event, "Duration")
                                duration.text = "1"
                                time = etree.SubElement(event, "Time")
                                time.set("Reference", 'Lunes' + str(diaHora-8))
                                resources = etree.SubElement(event, "Resources")
                                resource = etree.SubElement(resources, "Resource")
                                resource.set("Reference", aula)
                                role = etree.SubElement(resource, "Role")
                                role.text = "Room"
                                guardadoLunes = True
                                
                    if self.ids['_afternoon'].children[i].children[j].getIdent() == 'Martes'and guardadoMartes == False:
                        for j in range(len(self.ids['_afternoon'].children[0].children)):
                            diaHora = self.ids['_afternoon'].children[i].children[j].getIdent()
                            asignatura = self.ids['_afternoon'].children[i].children[j].getAsigID()
                            aula = self.ids['_afternoon'].children[i].children[j].getAulaID()
                            self.ids['_afternoon'].children[i].children[j].background_color = [1,1,1,1]
                            #Compruebo que no sean horas libres
                            if len(asignatura) > 1:
                                #Añado la asignatura con su horario y aula
                                event = etree.SubElement(events, "Event")
                                event.set("Reference", asignatura)
                                duration = etree.SubElement(event, "Duration")
                                duration.text = "1"
                                time = etree.SubElement(event, "Time")
                                time.set("Reference", 'Martes' + str(diaHora-8))
                                resources = etree.SubElement(event, "Resources")
                                resource = etree.SubElement(resources, "Resource")
                                resource.set("Reference", aula)
                                role = etree.SubElement(resource, "Role")
                                role.text = "Room"
                                guardadoMartes = True
                                
                    if self.ids['_afternoon'].children[i].children[j].getIdent() == 'Miercoles' and guardadoMiercoles == False:
                        for j in range(len(self.ids['_afternoon'].children[0].children)):
                            diaHora = self.ids['_afternoon'].children[i].children[j].getIdent()
                            asignatura = self.ids['_afternoon'].children[i].children[j].getAsigID()
                            aula = self.ids['_afternoon'].children[i].children[j].getAulaID()
                            self.ids['_afternoon'].children[i].children[j].background_color = [1,1,1,1]
                            #Compruebo que no sean horas libres
                            if len(asignatura) > 1:
                                #Añado la asignatura con su horario y aula
                                event = etree.SubElement(events, "Event")
                                event.set("Reference", asignatura)
                                duration = etree.SubElement(event, "Duration")
                                duration.text = "1"
                                time = etree.SubElement(event, "Time")
                                time.set("Reference", 'Miercoles' + str(diaHora-8))
                                resources = etree.SubElement(event, "Resources")
                                resource = etree.SubElement(resources, "Resource")
                                resource.set("Reference", aula)
                                role = etree.SubElement(resource, "Role")
                                role.text = "Room"
                                guardadoMiercoles = True
                                
                    if self.ids['_afternoon'].children[i].children[j].getIdent() == 'Jueves' and guardadoJueves == False:
                        for j in range(len(self.ids['_afternoon'].children[0].children)):
                            diaHora = self.ids['_afternoon'].children[i].children[j].getIdent()
                            asignatura = self.ids['_afternoon'].children[i].children[j].getAsigID()
                            aula = self.ids['_afternoon'].children[i].children[j].getAulaID()
                            self.ids['_afternoon'].children[i].children[j].background_color = [1,1,1,1]
                            #Compruebo que no sean horas libres
                            if len(asignatura) > 1:
                                #Añado la asignatura con su horario y aula
                                event = etree.SubElement(events, "Event")
                                event.set("Reference", asignatura)
                                duration = etree.SubElement(event, "Duration")
                                duration.text = "1"
                                time = etree.SubElement(event, "Time")
                                time.set("Reference", 'Jueves' + str(diaHora-8))
                                resources = etree.SubElement(event, "Resources")
                                resource = etree.SubElement(resources, "Resource")
                                resource.set("Reference", aula)
                                role = etree.SubElement(resource, "Role")
                                role.text = "Room"
                                guardadoJueves = True
                                
                    if self.ids['_afternoon'].children[i].children[j].getIdent() == 'Viernes' and guardadoViernes == False:
                        for j in range(len(self.ids['_afternoon'].children[0].children)):
                            diaHora = self.ids['_afternoon'].children[i].children[j].getIdent()
                            asignatura = self.ids['_afternoon'].children[i].children[j].getAsigID()
                            aula = self.ids['_afternoon'].children[i].children[j].getAulaID()
                            self.ids['_afternoon'].children[i].children[j].background_color = [1,1,1,1]
                            #Compruebo que no sean horas libres
                            if len(asignatura) > 1:
                                #Añado la asignatura con su horario y aula
                                event = etree.SubElement(events, "Event")
                                event.set("Reference", asignatura)
                                duration = etree.SubElement(event, "Duration")
                                duration.text = "1"
                                time = etree.SubElement(event, "Time")
                                time.set("Reference", 'Viernes' + str(diaHora-8))
                                resources = etree.SubElement(event, "Resources")
                                resource = etree.SubElement(resources, "Resource")
                                resource.set("Reference", aula)
                                role = etree.SubElement(resource, "Role")
                                role.text = "Room"
                                guardadoViernes = True
                                                                
        
        #Guardo la solucion en el fichero
        outFile = open(self.documento, 'wb')
        doc.write(outFile)            
    
    def intercambia(self, horario):

        '''Método para intercambiar los pares asignaturas/aulas de hora'''

        '''Si no he seleccionado una aula para asignar o estoy en medio de un intercambio'''
        if self.children[1].children[0].children[2].getText() == 'Acción: Intercambiar asignaturas' or self.numPulsaciones != 0:

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
                                texto = self.primero.text.splitlines()
                                grandchild.text = ('%s\n%s\n%s')%(texto[0], texto[1], a[2])
                                grandchild.select = 0
                                grandchild.disabled = False
                                grandchild.background_color = [1,0,0,1]
                                
                                auxAsig = grandchild.getAsigID()
                                auxAula = grandchild.getAulaID()
                                grandchild.setAsigID(self.primero.getAsigID())
                                grandchild.setAulaID(self.primero.getAulaID())
                                self.primero.setAsigID(auxAsig)
                                self.primero.setAulaID(auxAula)
                                

            if self.numPulsaciones == 2:
                for child in self.ids[ids].children:
                    self.k = self.k + 1
                    self.l = 0
                    for grandchild in child.children:
                        self.l = self.l + 1
                        if grandchild.select == 2:
                            if grandchild != self.segundo:
                                #Intercambio el segundo
                                grandchild.text = ('%s\n%s\n%s')%(a[0], a[1], texto[2])                            
                                grandchild.select = 0
                                grandchild.disabled = False
                                grandchild.background_color = [1,0,0,1]

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
                                grandchild.background_color = [1,0,0,1]

                #Reinicio para volver a intercambiar
                self.numPulsaciones = 0
        else:

            #Asigno el aula seleccionada. No permito cambiar el aula si se está en medio de un intercambio de horas
            doc = etree.parse('datos/outfile_nuevo_solucion.xml')
            timegroups = doc.getroot().find('Instances')
            timegroups = timegroups.find('Instance')
            resources = timegroups.find('Resources')

            if self.children[1].children[0].children[2].getText() == 'Acción: Quitar aula':
                self.nombreAula = 'Sin Aula'
                self.idAula = ''
            else:
                #Recupera las aulas
                for child in resources:
                    resource = child.find('ResourceType')
                    if resource is not None:
                        resourceType = resource.get('Reference')

                    if resourceType == 'Room' or resourceType == 'Laboratory':
                        if 'Asignar: ' + child.findtext('Name') == self.children[1].children[0].children[2].getText():
                            #Datos del aula seleccionada
                            self.idAula = child.get('Id')
                            self.nombreAula = child.findtext('Name')

            # Recupero el id de la ventana activa
            ids = self.ids['_screen_manager'].current
                
            if ids == 'dia':
                ids = '_morning'
            else:
                ids = '_afternoon'

            # Recorro la ventana y cambio el texto y el ID
            if self.numPulsaciones == 0:
                for child in self.ids[ids].children:
                    self.i = self.i + 1
                    self.j = 0
                    for grandchild in child.children:
                        self.j = self.j + 1
                        if grandchild.select == 2:
                            grandchild.setAulaID(self.idAula)
                            texto = grandchild.text.splitlines()
                            grandchild.text = ('%s\n%s\n%s')%(texto[0], self.nombreAula, texto[2])
                            grandchild.select = 0
                            grandchild.disabled = False
                            grandchild.background_color = [1,0,0,1]


    def changeAsignaturaAula(self):
        '''Método que crea el desplegable para cambiar el aula de la asignatura/hora seleccionada'''

        if self.inicio == 0:
            #Fichero de lectura
            doc = etree.parse(self.documento)
            timegroups = doc.getroot().find('Instances')
            timegroups = timegroups.find('Instance')
            resources = timegroups.find('Resources')

            #Inserto un botón para quitar el aula y otro para volver a intercambiar asignaturas
            btn = Boton2(text='Acción: Intercambiar asignaturas', size_hint_y=None, height=44) 
            btn.bind(on_release=lambda btn: self.aulas.select(btn.text))
            self.aulas.add_widget(btn)
            btn = Boton2(text='Acción: Quitar aula', size_hint_y=None, height=44) 
            btn.bind(on_release=lambda btn: self.aulas.select(btn.text))
            self.aulas.add_widget(btn)
                        
            #Recupera todas las aulas disponibles
            for child in resources:
                resource = child.find('ResourceType')
                if resource is not None:
                    resourceType = resource.get('Reference')

                if resourceType == 'Room' or resourceType == 'Laboratory':
                    #Inserto los botones de las aulas
                    texto = 'Asignar: ' + child.findtext('Name')
                    btn = Boton2(text=texto, size_hint_y=None, height=44)
                    btn.setAulaID(child.get('Id'))
                    
                    # Mostrar el menu
                    btn.bind(on_release=lambda btn: self.aulas.select(btn.text))

                    # add el boton dentro del dropdown
                    self.aulas.add_widget(btn)

            #añado los desplegables a la pantalla
            self.children[1].children[0].children[2].bind(on_release=self.aulas.open)
            self.inicio = 1

        self.children[1].children[0].children[2].bind(on_release=self.aulas.open)
        self.aulas.bind(on_select=lambda instance, x: setattr(self.children[1].children[0].children[2], 'text', x))        
