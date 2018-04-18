from kivy.uix.floatlayout import FloatLayout
from datetime import timedelta
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.button import Button
from kivy.uix.dropdown import DropDown
from kivy.uix.listview import ListView
from boton import Boton
from boton2 import Boton2
from collections import defaultdict
from time import time
from helpers import dedupe
import re

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
    log = open('salida.txt','w')
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
            relleno = [ [9,bx_m,'_morning',horarioPrincipal.clases_manana],
                        [15,bx_t,'_afternoon', horarioPrincipal.clases_tarde]]
            for hour, bx_iter,turno, asignacion in relleno:

                #añado las cabeceras (dias)
                nombre = dia
                btn = Boton(text=nombre, size_hint_y=0.02)
                btn.setIdent(nombre)
                bx_iter.add_widget(btn)

                #Botones  (Clase/Aula/Hora)
                for texto, porcentaje, asignaturaID, aulaID in asignacion(nombre):
                    btn = Boton(text = texto, size_hint_y = porcentaje)
                    btn.bind(on_release = lambda x:self.intercambia(horario=horarioPrincipal))
                    btn.setIdent(hour)
                    btn.setAsigID(asignaturaID)
                    btn.setAulaID(aulaID)
                    btn.text_size = (300, 100)
                    bx_iter.add_widget(btn)
                    hour = hour +1
                self.ids[turno].add_widget(bx_iter)

        #Añado los desplegables de selección a la primera pantalla   
        profes = DropDown()
        aulas = DropDown()
        asigns = DropDown()
        curs = DropDown()
        button = Button()
        profText = self.bd.all_prof_text()
        profId = self.bd.all_prof_id()
        aulaText = self.bd.all_aula_text()
        aulaId = self.bd.all_aula_id()
        cursoText = self.bd.all_curso_text()
        cursoId = self.bd.all_curso_id()
        asigText = self.bd.all_asignatura_text()
        asigId = self.bd.all_asignatura_id()
        # No voy a dividir a los cursos por grupos de practicas
        temporal = list(dedupe([(c_i, c_t[0:-10]) for c_i, c_t in zip(cursoId, cursoText)],
                               key= lambda x: x[1])
        )
        cursoText = [i[1] for i in temporal]
        cursoId = [i[0] for i in temporal]  
        datos_scroll = [(profes,'Profesores',350, profText,profId),
                        (aulas, 'Aulas',300, aulaText, aulaId),
                        (asigns,'Asignaturas',400,asigText, asigId),
                        (curs, 'Cursos',400,cursoText, cursoId)]
        loadSelection = Button(text = 'Cargar selección', size_hint = (None, None), width = 200)
        loadSelection.bind(on_release=lambda btn: self.loadTimetable(horarioPrincipal))
        self.ids['_main'].add_widget(loadSelection)
        for tipo_scroll, nombre_scroll,tamano, tupla_text, tupla_id in datos_scroll:
            for texto, ident in zip(tupla_text,tupla_id):
                btn = Boton(text=texto, size_hint_y=None, height=44)
                btn.setIdent(ident)
                btn.bind(on_release=lambda btn: tipo_scroll.select(btn.text))
                btn.bind(on_release=lambda btn: self.loadFilter(btn.text,filterLoad,btn.ident))
                tipo_scroll.add_widget(btn)
            btn = Boton(text='No seleccionado', size_hint_y=None, height=44)
            btn.bind(on_release=lambda btn: tipo_scroll.select(btn.text))
            tipo_scroll.add_widget(btn)
            scrollButton = Button(text = nombre_scroll,
                                  size_hint = (None, None),
                                  width = tamano)
            tipo_scroll.bind(on_select=lambda instance, x: setattr(scrollButton, 'text', x))
            self.ids['_main'].add_widget(scrollButton)

        nInc = 0
        for incidencia in self.bd.colisiones():
            btn = Button(text = incidencia)
            self.ids['_incidences'].add_widget(btn)
            nInc += 1
            if nInc >= self.numIncidences:
                break                                         
        self.children[1].children[0].children[2].text = 'Acción: Intercambiar asignaturas'
        self.children[0].children[0].children[4].text = 'Horario de mañana'
        self.resetDropdown()
            
    def resetDropdown(self):
        print("SE LAMA resetDropdown")
        #Añado los desplegables de selección a la primera pantalla   
        profes = DropDown()
        aulas = DropDown()
        asigns = DropDown()
        curs = DropDown()
        button = Button()
        profText = self.bd.all_prof_text()
        profId = self.bd.all_prof_id()
        aulaText = self.bd.all_aula_text()
        aulaId = self.bd.all_aula_id()
        cursoText = self.bd.all_curso_text()
        cursoId = self.bd.all_curso_id()
        asigText = self.bd.all_asignatura_text()
        asigId = self.bd.all_asignatura_id()
        temporal = list(dedupe([(c_i, c_t[0:-10]) for c_i, c_t in zip(cursoId, cursoText)],
                               key= lambda x: x[1])
        )
        cursoText = [i[1] for i in temporal]
        cursoId = [i[0] for i in temporal]  
        datos_scroll = [(profes,'Profesores',350,self.ids['_main'].children[3], profText,profId),
                        (aulas, 'Aulas',300,self.ids['_main'].children[2], aulaText, aulaId),
                        (asigns,'Asignaturas',400,self.ids['_main'].children[1],asigText, asigId),
                        (curs, 'Cursos',400,self.ids['_main'].children[0],cursoText, cursoId)]
        filterLoad = set(self.filterTotal)

        for tipo_scroll, nombre_scroll,tamano,wid_pos, tupla_text, tupla_id in datos_scroll:
            for texto, ident in zip(tupla_text,tupla_id):
                btn = Boton(text=texto, size_hint_y=None, height=44)
                btn.setIdent(ident)
                btn.bind(on_release=lambda btn: tipo_scroll.select(btn.text))
                btn.bind(on_release=lambda btn: self.loadFilter(btn.text,filterLoad,btn.ident))
                tipo_scroll.add_widget(btn)
            btn = Boton(text='No seleccionado', size_hint_y=None, height=44)
            btn.bind(on_release=lambda btn: tipo_scroll.select(btn.text))
            tipo_scroll.add_widget(btn)
            wid_pos.bind(on_release=tipo_scroll.open)
            wid_pos.text = nombre_scroll
            scrollButton = Button(text = nombre_scroll,
                                  size_hint = (None, None),
                                  width = tamano)
            tipo_scroll.bind(on_select=lambda instance, x: setattr(wid_pos,
                                                                   'text', x))
            wid_pos.disabled = False

        self.children[1].children[0].children[0].text = 'Filtro seleccionado: Ninguno'
        for turno in ['_morning','_afternoon']:
            for pos,j in self.botones_turno(turno):
                if j.getIdent() in self.days:
                    continue
                j.setAsigID('')
                j.setAulaID('')
                texto = "{0}\n{1}\n{2}--{3}".format('Libre','Sin Aula',pos,pos + 1)
                j.setText(texto)
                j.background_color = [1,1,1,1]
        self.ids['_main'].children[4].disabled = False
        self.lastIdentificador = ''
        self.filterTotal = set()
        self.filterLoad = set()        


        
    def loadFilter(self,text,filterLoad,ident):
        '''
        Método que recupera el filtro seleccionado en los desplegables 
        y que se usa para cargar
        '''
        nombre = ident
        if 'Class' in nombre:# En caso que busquemos un curso
            nombre = '_'.join(ident.split('_')[0:2]) + '_PLACEHOLDER'
        print("Nombre",nombre)
        print("Self.filterLoad",len(self.filterLoad))
        self.filterLoad = set(self.bd.asig_ident(nombre))
        print("Self.filterLoad",len(self.filterLoad))
        #Pongo el texto en el botón que sirve como indicador
        self.children[1].children[0].children[0].text = 'Filtro seleccionado: ' + text
        for pos in range(3,-1,-1):
            self.ids['_main'].children[pos].disabled = True
        #self.filterLoad = self.filterLoad.union(filterLoad)
        if re.search('^G([0-9]{1,3})',ident):
            self.filterLoad.add(ident)
        self.filterTotal = self.filterLoad
        print(f'El filtro:{len(self.filterTotal)}')
        
    def botones_turno(self, turno):
        for i in self.ids[turno].children:
            #Recorro los días
            for pos,j in reversed(list(enumerate(i.children))):
                yield (6-pos if turno =='_morning' else 10-pos),j
                
    def loadTimetable(self,horarioPrincipal):
        '''Método que realiza la carga del horario según lo introducido en el filtro'''
        
        #Inicializaciones
        filterProf = ''
        filterAula = ''
        filterAsig = ''
        filterCurs = ''
        dat_temp = self.bd.contain(self.filterTotal)
        print("esto es dat")
        for temp in dat_temp:
            print(temp)
        listaParcial = [i[0] for i in dat_temp]
        timeParcial = [i[2] for i in dat_temp]
        asigParcial = [i[1] for i in dat_temp]
        nombresAsig = [i[3] for i in dat_temp]
        nombresAulas = [i[4] for i in dat_temp]
        self.ids['_main'].children[4].disabled = True
        dia_id = 'Lunes'
        for turno in ['_morning','_afternoon']:
            for pos,j in self.botones_turno(turno):
                dia_id = j.getIdent() if j.getIdent() in self.days else dia_id
                if j.getIdent() in self.days:
                    continue
                elif dia_id+str(pos) in timeParcial:
                    position = timeParcial.index(dia_id+str(pos))
                    j.setAsigID(asigParcial[position])
                    j.setAulaID(listaParcial[position])
                    texto = "{0}\n{1}\n{2}--{3}".format(nombresAsig[position],
                                                        nombresAulas[position], pos, pos + 1)
                else:
                    j.setAsigID('')
                    j.setAulaID('')
                    texto = "{0}\n{1}\n{2}--{3}".format('Libre','Sin Aula',pos,pos + 1)
                j.setText(texto)
                j.background_color = [1,1,1,1]


    def saveTimetable(self):
        '''
        Método que guarda los datos del horario cargado con el filtro en el fichero
        '''
        #recorro el horario y inserto sus datos
        guardado_dict = defaultdict(lambda : True)
        for i in self.days:
            guardado_dict[i] = False
        #Guardo los datos de la mañana
        for turno in ['_morning','_afternoon']:
            guardado_dict = {i:False for i in self.days}
            for i in self.ids[turno].children:
                #Recorro los días
                for j in i.children:
                    dia_id = j.getIdent()
                    if dia_id in self.days and not guardado_dict[dia_id]:
                        for j in i.children:
                            diaHora = j.getIdent()
                            asignatura = j.getAsigID()
                            aula = j.getAulaID()
                            j.background_color = [1,1,1,1]
                            #Compruebo que no sean horas libres
                            if asignatura:
                                self.bd.cambia_dia(asignatura,
                                            dia_id  + str(diaHora-8))
                                self.bd.cambia_aula(asignatura, aula)
                                print(asignatura, aula, dia_id)
                        guardado_dict[dia_id] = True
        for texto, boton in zip(self.bd.colisiones(),self.ids['_incidences'].children):
            boton.text =  texto
    def intercambia(self, horario):
        print("Sellama intercambia")
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

            # #Asigno el aula seleccionada. No permito cambiar el aula si se está en medio de un intercambio de horas
            # doc = etree.parse('datos/outfile_nuevo_solucion.xml')
            # timegroups = doc.getroot().find('Instances')
            # timegroups = timegroups.find('Instance')
            # resources = timegroups.find('Resources')

            # if self.children[1].children[0].children[2].getText() == 'Acción: Quitar aula':
            #     self.nombreAula = 'Sin Aula'
            #     self.idAula = ''
            # else:
            #     #Recupera las aulas
            #     for child in resources:
            #         resource = child.find('ResourceType')
            #         if resource is not None:
            #             resourceType = resource.get('Reference')

            #         if resourceType == 'Room' or resourceType == 'Laboratory':
            #             if 'Asignar: ' + child.findtext('Name') == self.children[1].children[0].children[2].getText():
            #                 #Datos del aula seleccionada
            #                 self.idAula = child.get('Id')
            #                 self.nombreAula = child.findtext('Name')

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
            
            #Inserto un botón para quitar el aula y otro para volver a intercambiar asignaturas
            btn = Boton2(text='Acción: Intercambiar asignaturas', size_hint_y=None, height=44) 
            btn.bind(on_release=lambda btn: self.aulas.select(btn.text))
            self.aulas.add_widget(btn)
            btn = Boton2(text='Acción: Quitar aula', size_hint_y=None, height=44) 
            btn.bind(on_release=lambda btn: self.aulas.select(btn.text))
            self.aulas.add_widget(btn)
                        
            #Recupera todas las aulas disponibles
            for identificador, nombre in zip(self.bd.all_aula_id(),self.bd.all_aula_text()):
                
                    #Inserto los botones de las aulas
                    texto = 'Asignar: ' + nombre
                    btn = Boton2(text=texto, size_hint_y=None, height=44)
                    btn.setAulaID(identificador)
                    
                    # Mostrar el menu
                    btn.bind(on_release=lambda btn: self.aulas.select(btn.text))

                    # add el boton dentro del dropdown
                    self.aulas.add_widget(btn)

            #añado los desplegables a la pantalla
            self.children[1].children[0].children[2].bind(on_release=self.aulas.open)
            self.inicio = 1

        self.children[1].children[0].children[2].bind(on_release=self.aulas.open)
        self.aulas.bind(on_select=lambda instance, x: setattr(self.children[1].children[0].children[2], 'text', x))        
