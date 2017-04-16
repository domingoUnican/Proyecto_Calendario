# -*- coding: utf-8 -*-
from copy import copy
from kivy.app import App
from kivy.lang import Builder
from datetime import timedelta
from horario import horario
from Boxes import Boxes
from kivy.uix.popup import Popup
from kivy.uix.filechooser import FileChooserListView
from kivy.config import Config
from lxml import etree
#from excel import excel

#variables globales
dias=['Lunes','Martes','Miercoles','Jueves','Viernes']
horarioPrincipal = horario(dias,timedelta(hours=9))
        
class TestApp(App):
    def build(self):

        #Modifico la configuracion de kivy
        Config.set('graphics', 'fullscreen', 'auto')
        Config.write()

        #Leo el horario desde el archivo generado por KHE
        #Inicializaciones
##        doc = etree.parse('outfile2.xml')
##        root = doc.getroot()
##
##        #Me posiciono en la solución generada
##        solutionGroup = doc.find('SolutionGroups')
##        solution = solutionGroup.find('SolutionGroup')
##        solution1 = solution.find('Solution')
##        events = solution1.find('Events')
##
##        for event in events:
##            reference = event.get('Reference')
##            print(reference)
        
        horarioPrincipal.incluye_hora('Lunes','Mates',timedelta(hours=9), timedelta(hours=10))
        horarioPrincipal.incluye_hora('Lunes','Ingles',timedelta(hours=10), timedelta(hours=11))
        horarioPrincipal.incluye_hora('Martes','Lengua',timedelta(hours=9), timedelta(hours=11))
        horarioPrincipal.incluye_hora('Miercoles','Frances',timedelta(hours=9), timedelta(hours=11))
        horarioPrincipal.incluye_hora('Jueves','Ingles',timedelta(hours=9), timedelta(hours=10))
        horarioPrincipal.incluye_hora('Jueves','Frances',timedelta(hours=10), timedelta(hours=11))
        horarioPrincipal.incluye_hora('Viernes','Frances',timedelta(hours=9), timedelta(hours=11))

        for dia in ['Lunes','Martes','Miercoles','Jueves','Viernes']:
            for i in range(11,19):
                horarioPrincipal.incluye_hora(dia,'Libre',timedelta(hours=i), timedelta(hours=i+1))

        return Boxes(horarioPrincipal)

    def save(self):
        print('PRINT MONEY')
        horarioPrincipal.save_timetableXML()
        print('AFTER PRINT')

if __name__ == '__main__':
    TestApp().run()
