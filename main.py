# -*- coding: UTF-8 -*-
from kivy.config import Config
from copy import copy
from kivy.app import App
from kivy.lang import Builder
from datetime import timedelta
from horario import horario
from Boxes import Boxes
from kivy.uix.popup import Popup
from kivy.uix.filechooser import FileChooserListView
from lxml import etree
#from excel import excel

#variables globales
dias=['Lunes','Martes','Miercoles','Jueves','Viernes']
horarioPrincipal = horario(dias,1)
filterTotal = set()
documento = 'datos/outfile_nuevo_solucion.xml'
        
class TestApp(App):
    def build(self):

        #Modifico la configuracion de kivy
        Config.set('input', 'mouse', 'mouse,multitouch_on_demand')
        Config.set('graphics', 'fullscreen', 'auto')
        Config.write()

        '''Creación de un horario vacío para rellenarlo con los datos posteriormente'''
        for dia in ['Lunes','Martes','Miercoles','Jueves','Viernes']:
            for i in range(9,19):
                horarioPrincipal.incluye_hora(dia,'Libre', dia, 'Sin Aula', '', i-8, i-7)
        
        self.box = Boxes(horarioPrincipal,filterTotal,documento)

        return self.box

    def save(self):
        '''Llamada al guardado del horario'''
        self.box.saveTimetable()

    def changeAula(self):
        '''Llamada al cambio de asignatura'''
        self.box.changeAsignaturaAula()

    def resetFilter(self):
        '''Llamada al reseteo del filtro'''
        self.box.resetDropdown()

if __name__ == '__main__':
    TestApp().run()
