from kivy.uix.floatlayout import FloatLayout
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
    
    def __init__(self, horarioPrincipal, **kwargs):
        ''' Inicialización del horario '''
        
        super(Boxes, self).__init__(**kwargs)

        #añado los botones al horario
        for dia in horarioPrincipal.dias:
            bx_m = BoxLayout(orientation='vertical')#mañana
            bx_t = BoxLayout(orientation='vertical')#tarde

            #añado dias
            nombre = dia
            bx_m.add_widget(Boton(text=nombre, size_hint_y=0.02))
            bx_t.add_widget(Boton(text=nombre, size_hint_y=0.02))
            
            #Botones de mañana
            for texto, porcentaje in horarioPrincipal.clases_manana(nombre):
                btn = Boton(text = texto,
                             size_hint_y = porcentaje)
                btn.bind(on_release = lambda x:self.intercambia(horario=horarioPrincipal))
                bx_m.add_widget(btn)
            self.ids['_morning'].add_widget(bx_m)

            #Botones de tarde
            for texto, porcentaje in horarioPrincipal.clases_tarde(nombre):
                btn = Boton(text = texto,
                             size_hint_y = porcentaje)
                btn.bind(on_release = lambda x:self.intercambia(horario=horarioPrincipal))
                bx_t.add_widget(btn)
            self.ids['_afternoon'].add_widget(bx_t)

        #Añado los desplegables de la primera pantalla   
        profes = DropDown()
        aulas = DropDown()
        asigns = DropDown()
        curs = DropDown()
        button = Button()
        profesores=[]
        salas=[]
        asignaturas=[]
        cursos=[]

        print('LEYENDO XML')

        doc = etree.parse('outfile.xml')
        #print(doc)
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
                btn = Button(text=child.findtext('Name'), size_hint_y=None, height=44)
                
                # Mostrar el menu
                btn.bind(on_release=lambda btn: profes.select(btn.text))

                # add el boton dentro del dropdown
                profes.add_widget(btn)

            if resourceType == 'Room':
                #Inserto los botones de los profesores
                #print(child.findtext('Name'))
                btn = Button(text=child.findtext('Name'), size_hint_y=None, height=44)
                
                # Mostrar el menu
                btn.bind(on_release=lambda btn: aulas.select(btn.text))

                # add el boton dentro del dropdown
                aulas.add_widget(btn)

            if resourceType == 'Class':
                #Inserto los botones de los profesores
                #print(child.findtext('Name'))
                btn = Button(text=child.findtext('Name'), size_hint_y=None, height=44)
                # Mostrar el menu
                btn.bind(on_release=lambda btn: curs.select(btn.text))

                # add el boton dentro del dropdown
                curs.add_widget(btn)

        #Añado las asignaturas
        events = timegroups.find('Events')
        
        for child in events:
            '''event = child.find('Event')'''

            '''Inserto las asignaturas'''
            if child.findtext('Name') is not None:
                #print(child.findtext('Name'))
                btn = Button(text=child.findtext('Name'), size_hint_y=None, height=44)
                # Mostrar el menu
                btn.bind(on_release=lambda btn: asigns.select(btn.text))

                # add el boton dentro del dropdown
                asigns.add_widget(btn)

        #Añado un boton vacio a cada desplegable      
        # Profesores
        btn = Button(text='Ninguno', size_hint_y=None, height=44)
        btn.bind(on_release=lambda btn: profes.select(btn.text))
        profes.add_widget(btn)
        # aulas
        btn = Button(text='Ninguna', size_hint_y=None, height=44) 
        btn.bind(on_release=lambda btn: aulas.select(btn.text))
        aulas.add_widget(btn)
        #Cursos
        btn = Button(text='Ninguno', size_hint_y=None, height=44) 
        btn.bind(on_release=lambda btn: curs.select(btn.text))
        curs.add_widget(btn)

        #añado los desplegables a la pantalla
        profesbutton = Button(text = 'Profesores', size_hint = (None, None), width = 350)
        profesbutton.bind(on_release=profes.open)
        aulasbutton= Button(text = 'Aulas', size_hint = (None, None), width = 250)
        aulasbutton.bind(on_release=aulas.open)
        asignsbutton= Button(text = 'Asignaturas', size_hint = (None, None), width = 450)
        asignsbutton.bind(on_release=asigns.open)
        cursosbutton= Button(text = 'Cursos', size_hint = (None, None), width = 150)
        cursosbutton.bind(on_release=curs.open)
        aulas.bind(on_select=lambda instance, x: setattr(aulasbutton, 'text', x))
        profes.bind(on_select=lambda instance, x: setattr(profesbutton, 'text', x))
        asigns.bind(on_select=lambda instance, x: setattr(asignsbutton, 'text', x))
        curs.bind(on_select=lambda instance, x: setattr(cursosbutton, 'text', x))
        self.ids['_main'].add_widget(profesbutton)
        self.ids['_main'].add_widget(aulasbutton)
        self.ids['_main'].add_widget(asignsbutton)
        self.ids['_main'].add_widget(cursosbutton)

        #Separación y boton de exportacion
        empty = Button(text = '', size_hint = (None, None))
        self.ids['_main'].add_widget(empty)

        export = Button(text = 'Exportar', size_hint = (None, None))
        self.ids['_main'].add_widget(export)

        
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

            self.numPulsaciones = 0
