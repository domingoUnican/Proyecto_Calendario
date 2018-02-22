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
    days = ['Lunes','Martes','Miercoles','Jueves','Viernes']
    timeTable = ''
    filterLoad = set()
    filterTotal = set()
    idAula = ''
    nombreAula = ''
    inicio = 0
    aulas = DropDown()
    lastIdentificador = ''
    numIncidences = 10
    bd = None
    
    def __init__(self, horarioPrincipal, filterTotal,  bd,  **kwargs):
        ''' Inicialización del horario '''

        timeTable = horarioPrincipal
        filterLoad = filterTotal
        self.bd = bd
        super(Boxes, self).__init__(**kwargs)        
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

        profText = bd.all_prof_text()
        profId = bd.all_prof_id()
        aulaText = bd.all_aula_text()
        aulaId = bd.all_aula_id()
        cursoText = bd.all_curso_text()
        cursoId = bd.all_curso_id()
        asigText = bd.all_asignatura_text()
        asigId = bd.all_asignatura_id()

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
        

        nInc = 0
        for incidencia in bd.colisiones():
            btn = Button(text = resourceName + text + child.findtext('Name'))
            self.ids['_incidences'].add_widget(btn)
            nInc += 1
            if nInc >= self.numIncidences:
                break                                         
        self.children[1].children[0].children[2].text = 'Acción: Intercambiar asignaturas'
        self.children[0].children[0].children[4].text = 'Horario de mañana'
        
            
    def resetDropdown(self):
        #Añado los desplegables de selección a la primera pantalla   
        profes = DropDown()
        aulas = DropDown()
        asigns = DropDown()
        curs = DropDown()
        button = Button()
        profText = bd.all_prof_text()
        profId = bd.all_prof_id()
        aulaText = bd.all_aula_text()
        aulaId = bd.all_aula_id()
        cursoText = bd.all_curso_text()
        cursoId = bd.all_curso_id()
        asigText = bd.all_asignatura_text()
        asigId = bd.all_asignatura_id()
        filterLoad = self.filterTotal

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

        #Borro los datos de la mañana para pintar de nuevo
        for i in range(len(self.ids['_morning'].children)):
            #Busco el día
            for boton in range(len(self.ids['_morning'].children[0].children)):
                #Busco la hora
                for j in range(len(self.ids['_morning'].children[0].children)):
                    if self.ids['_morning'].children[i].children[j].getIdent() not in self.days:
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
                    if self.ids['_afternoon'].children[i].children[j].getIdent() not in self.days:
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
        
        #Pongo el texto en el botón que sirve como indicador
        self.children[1].children[0].children[0].text = self.children[1].children[0].children[0].text.split(':')[0] + ': ' + text
 
        #filterLoad = filterLoad.union(self.filterTotal)
            
        listaParcial = set()
        listaPadres = set()
        listaHermanos = set()
        hermanoEncontrado = False
            
        # Recupero los valores seleccionados y cargo sus referencias xml
        # doc = etree.parse(self.documento)
        # timegroups = doc.getroot().find('Instances')
        # timegroup = timegroups.find('Instance')
        # events = timegroup.find('Events')

        # #Si no filtro por aula, busco las asignaturas
        # #Busco los profesores, clases y asignaturas que coinciden con el filtro
        # for child in events:
        #     if child is not None:
        #         name = child.find('Name')
        #         key = child.get('Id')
        #         if key in filterLoad:
        #             resources = child.find('Resources')
        #             if resources is not None:
        #                 for nice in resources:
        #                     if nice.get('Reference') is not None:
        #                         if nice.get('Reference') in filterLoad:
        #                             print('')
        #                             #else:
        #                                 #listaParcial.add(nice.get('Reference'))
         
        #         else:
        #             resources = child.find('Resources')
        #             listaHermanos = set()
        #             if resources is not None:
        #                 for nice in resources:
        #                     if nice.get('Reference') is not None:
        #                         if nice.get('Reference') in filterLoad:
        #                             listaPadres.add(child.get('Id'))
        #                             hermanoEncontrado = True
        #                             #else:
        #                                 #print('')
        #                                 #listaHermanos.add(nice.get('Reference'))
                                            
        #                 if hermanoEncontrado == True:
        #                     listaParcial = listaParcial.union(listaPadres)
        #                     listaParcial = listaParcial.union(listaHermanos)
        #                     hermanoEncontrado = False
            
        listaAsignaturas = set()

        #Salvo el filtro        
        self.filterLoad = listaParcial

        #Si no se ha encontrado (aula) lo añado directamente
        if self.filterLoad == set():
            self.filterLoad.add(ident)
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
            
        # timegroups = doc.getroot().find('Instances')
        # timegroups = timegroups.find('Instance')
        # resources = timegroups.find('Resources')

        # #Recupera profesores, aulas y cursos
        # for child in resources:
        #     resource = child.find('ResourceType')

        #     '''Inserto los botones según los datos'''
        #     if resource is not None:
        #         resourceType = resource.get('Reference')

        #     #Actualizo el desplegable de los profesores
        #     if resourceType == 'Teacher':
        #         if child.get('Id') in self.filterLoad:
        #             btn = Boton(text=child.findtext('Name'), size_hint_y=None, height=44)
        #             ident = child.get('Id')
        #             btn.setIdent(ident)
        #             btn.bind(on_release=lambda btn: profes.select(btn.text))
        #             btn.bind(on_release=lambda btn: self.loadFilter(btn.text,filterLoad,btn.ident))
        #             profes.add_widget(btn)
        #             if child.get('Id') in self.filterTotal:
        #                 texto = child.findtext('Name')
        #                 identificador = 'Prof'
                        
        #     #Actualizo el desplegable de las aulas
        #     if resourceType == 'Room':
        #         if child.get('Id') in self.filterLoad:
        #             btn = Boton(text=child.findtext('Name'), size_hint_y=None, height=44)
        #             ident = child.get('Id')
        #             btn.setIdent(ident)
        #             btn.bind(on_release=lambda btn: aulas.select(btn.text))
        #             btn.bind(on_release=lambda btn: self.loadFilter(btn.text,filterLoad,btn.ident))
        #             aulas.add_widget(btn)
        #             if child.get('Id') in self.filterTotal:
        #                 texto = child.findtext('Name')
        #                 identificador = 'Aula'

        #     #Actualizo el desplegable de los cursos
        #     if resourceType == 'Class':
        #         if child.get('Id') in self.filterLoad:
        #             btn = Boton(text=child.findtext('Name'), size_hint_y=None, height=44)
        #             ident = child.get('Id')
        #             btn.setIdent(ident)
        #             btn.bind(on_release=lambda btn: curs.select(btn.text))
        #             btn.bind(on_release=lambda btn: self.loadFilter(btn.text,filterLoad,btn.ident))
        #             curs.add_widget(btn)
        #             if child.get('Id') in self.filterTotal:
        #                 texto = child.findtext('Name')
        #                 identificador = 'Curso'

        # #Añado las asignaturas
        # events = timegroups.find('Events')
            
        # for child in events:
        #     if child.findtext('Name') is not None:
        #         if child.get('Id') in self.filterLoad:
        #             btn = Boton(text=child.findtext('Name'), size_hint_y=None, height=44)
        #             ident = child.get('Id')
        #             btn.setIdent(ident)
        #             btn.bind(on_release=lambda btn: asigns.select(btn.text))
        #             btn.bind(on_release=lambda btn: self.loadFilter(btn.text,filterLoad,btn.ident))
        #             asigns.add_widget(btn)
        #             if child.get('Id') in self.filterTotal:
        #                 texto = child.findtext('Name')
        #                 identificador = 'Asig'

        # #Añado un boton vacio a cada desplegable      
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
        
        self.filterTotal = self.filterLoad
                
    
    def loadTimetable(self,horarioPrincipal):

        '''Método que realiza la carga del horario según lo introducido en el filtro'''
        
        #Inicializaciones
        filterProf = ''
        filterAula = ''
        filterAsig = ''
        filterCurs = ''
        dat_temp = self.bd.contain(self.filterTotal)
        listaParcial = [i[0] for i in dat_temp]
        timeParcial = [i[2] for i in dat_temp]
        asigParcial = [i[1] for i in dat_temp]
        nombresAsig = [i[3] for i in dat_temp]
        nombresAulas = [i[4] for i in dat_temp]
        self.ids['_main'].children[4].disabled = True

        #Borro los datos de la mañana para pintar de nuevo
        for i in range(len(self.ids['_morning'].children)):
            #Busco el día
            for boton in range(len(self.ids['_morning'].children[0].children)):
                #Busco la hora
                for j in range(len(self.ids['_morning'].children[0].children)):
                    if self.ids['_morning'].children[i].children[j].getIdent() not in self.days:
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
                    if self.ids['_afternoon'].children[i].children[j].getIdent() not in self.days:
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
        

        #inserto según los tiempos recuperados
        pos = 0
        
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
        
        # Recupero los valores seleccionados y cargo sus referencias xml
        self.bd.delete(self.filterTotal)
        #recorro el horario y inserto sus datos
        guardado_dict = {i:False for i in self.days}
        
        #Guardo los datos de la mañana
        for i in range(len(self.ids['_morning'].children)):
            #Recorro los días
            for boton in range(len(self.ids['_morning'].children[0].children)):
                #Recorro las horas
                for j in range(len(self.ids['_morning'].children[0].children)):
                    if not guardado_dict[self.ids['_morning'].children[i].children[j].getIdent()]:
                        dia_id = self.ids['_morning'].children[i].children[j].getIdent()
                        for j in range(len(self.ids['_morning'].children[0].children)):
                            diaHora = self.ids['_morning'].children[i].children[j].getIdent()
                            asignatura = self.ids['_morning'].children[i].children[j].getAsigID()
                            aula = self.ids['_morning'].children[i].children[j].getAulaID()
                            self.ids['_morning'].children[i].children[j].background_color = [1,1,1,1]
                            #Compruebo que no sean horas libres
                            if len(asignatura) > 1:
                                self.bd.add(asignatura,
                                            dia_id  + str(diaHora-8),
                                            aula
                                )
                    guardado_dict[self.ids['_morning'].children[i].children[j].getIdent()] = True
                guardado_dict = {i:False for i in self.days}
        guardado_dict = {i:False for i in self.days}
        #Guardo los datos de la mañana
        for i in range(len(self.ids['_afternoon'].children)):
            #Recorro los días
            for boton in range(len(self.ids['_afternoon'].children[0].children)):
                #Recorro las horas
                for j in range(len(self.ids['_afternoon'].children[0].children)):
                    if not guardado_dict[self.ids['_afternoon'].children[i].children[j].getIdent()]:
                        dia_id = self.ids['_afternoon'].children[i].children[j].getIdent()
                        for j in range(len(self.ids['_afternoon'].children[0].children)):
                            diaHora = self.ids['_afternoon'].children[i].children[j].getIdent()
                            asignatura = self.ids['_afternoon'].children[i].children[j].getAsigID()
                            aula = self.ids['_afternoon'].children[i].children[j].getAulaID()
                            self.ids['_afternoon'].children[i].children[j].background_color = [1,1,1,1]
                            #Compruebo que no sean horas libres
                            if len(asignatura) > 1:
                                self.bd.add(asignatura,
                                            dia_id  + str(diaHora-8),
                                            aula
                                )
                    guardado_dict[self.ids['_afternoon'].children[i].children[j].getIdent()] = True
    
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
