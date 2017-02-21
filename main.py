# -*- coding: utf-8 -*-
from copy import copy
from kivy.app import App
from kivy.lang import Builder
from datetime import timedelta
from horario import horario
from Boxes import Boxes
from kivy.uix.popup import Popup
from kivy.uix.filechooser import FileChooserListView
#from excel import excel

#variables globales
dias=['Lunes','Martes','Miércoles','Jueves','Viernes']
horarioPrincipal = horario(dias,timedelta(hours=9))
        
class TestApp(App):
    def build(self):
        horarioPrincipal.incluye_hora('Lunes','Mates',timedelta(hours=9), timedelta(hours=10))
        horarioPrincipal.incluye_hora('Lunes','Ingles',timedelta(hours=10), timedelta(hours=11))
        horarioPrincipal.incluye_hora('Martes','Lengua',timedelta(hours=9), timedelta(hours=11))
        horarioPrincipal.incluye_hora('Miércoles','Frances',timedelta(hours=9), timedelta(hours=11))
        horarioPrincipal.incluye_hora('Jueves','Frances',timedelta(hours=9), timedelta(hours=10))
        horarioPrincipal.incluye_hora('Jueves','Ingles',timedelta(hours=10), timedelta(hours=11))
        horarioPrincipal.incluye_hora('Viernes','Frances',timedelta(hours=9), timedelta(hours=11))

        for dia in ['Lunes','Martes','Miércoles','Jueves','Viernes']:
            for i in range(11,19):
                horarioPrincipal.incluye_hora(dia,'Libre',timedelta(hours=i), timedelta(hours=i+1))

        return Boxes(horarioPrincipal)

    def save(self):
        horarioPrincipal.save_timetableXML()

if __name__ == '__main__':
    TestApp().run()
