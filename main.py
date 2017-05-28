# -*- coding: utf-8 -*-
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
horarioPrincipal = horario(dias,timedelta(hours=9))
filterTotal = set()
box = ''
        
class TestApp(App):
    def build(self):

        #Modifico la configuracion de kivy
        Config.set('input', 'mouse', 'mouse,multitouch_on_demand')
        Config.set('graphics', 'fullscreen', 'auto')
        Config.write()
        
        for dia in ['Lunes','Martes','Miercoles','Jueves','Viernes']:
            for i in range(9,19):
                horarioPrincipal.incluye_hora(dia,'Libre', dia, 'Sin Aula', '', timedelta(hours=i), timedelta(hours=i+1))

        self.box = Boxes(horarioPrincipal,filterTotal)

        return self.box

    def save(self):
        print('COMIENZO DEL GUARDADO')

        self.box.saveTimetable()

        print('FIN DEL GUARDADO')

    def changeAula(self):
        print('SELECCIONA AULA')

        self.box.changeAsignaturaAula()

        print('AULA CAMBIADA')

if __name__ == '__main__':
    TestApp().run()
