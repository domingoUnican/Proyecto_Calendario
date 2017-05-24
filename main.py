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
filterTotal = set()
        
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
        
##        horarioPrincipal.incluye_hora('Lunes','PROGRAMACION: Teoria','Aula 1',timedelta(hours=9), timedelta(hours=10))
##        horarioPrincipal.incluye_hora('Lunes','CALCULO INTEGRAL: Teoria','Aula 2',timedelta(hours=10), timedelta(hours=11))
##        horarioPrincipal.incluye_hora('Lunes','ESTADISTICA BASICA: Teoria','Aula 3',timedelta(hours=11), timedelta(hours=12))
##        horarioPrincipal.incluye_hora('Lunes','CALCULO NUMERICO I: Teoria','Aula 4',timedelta(hours=12), timedelta(hours=13))
##        horarioPrincipal.incluye_hora('Lunes','INGLES: Teoria','Aula 5',timedelta(hours=13), timedelta(hours=14))
##        horarioPrincipal.incluye_hora('Lunes','Libre','Sin Aula',timedelta(hours=14), timedelta(hours=15))
##
##        horarioPrincipal.incluye_hora('Lunes','PROGRAMACION: Práctica','Laboratorio 1',timedelta(hours=15), timedelta(hours=17))
##        horarioPrincipal.incluye_hora('Lunes','FISICA BASICA: Práctica','Laboratorio 2',timedelta(hours=17), timedelta(hours=19))
##        
##        horarioPrincipal.incluye_hora('Martes','PROGRAMACION: Teoria','Aula 1',timedelta(hours=9), timedelta(hours=10))
##        horarioPrincipal.incluye_hora('Martes','CALCULO INTEGRAL: Teoria','Aula 2',timedelta(hours=10), timedelta(hours=11))
##        horarioPrincipal.incluye_hora('Martes','ESTADISTICA BASICA: Teoria','Aula 3',timedelta(hours=11), timedelta(hours=12))
##        horarioPrincipal.incluye_hora('Martes','CALCULO NUMERICO I: Teoria','Aula 4',timedelta(hours=12), timedelta(hours=13))
##        horarioPrincipal.incluye_hora('Martes','INGLES: Teoria','Aula 5',timedelta(hours=13), timedelta(hours=14))
##        horarioPrincipal.incluye_hora('Martes','Libre','Sin Aula',timedelta(hours=14), timedelta(hours=15))
##
##        horarioPrincipal.incluye_hora('Martes','CALCULO NUMERICO I: Práctica','Laboratorio 1',timedelta(hours=15), timedelta(hours=17))
##        horarioPrincipal.incluye_hora('Martes','ESTADISTICA BASICA: Práctica','Laboratorio 2',timedelta(hours=17), timedelta(hours=19))
##
##        horarioPrincipal.incluye_hora('Miercoles','PROGRAMACION: Teoria','Aula 1',timedelta(hours=9), timedelta(hours=10))
##        horarioPrincipal.incluye_hora('Miercoles','CALCULO INTEGRAL: Teoria','Aula 2',timedelta(hours=10), timedelta(hours=11))
##        horarioPrincipal.incluye_hora('Miercoles','ESTADISTICA BASICA: Teoria','Aula 3',timedelta(hours=11), timedelta(hours=12))
##        horarioPrincipal.incluye_hora('Miercoles','CALCULO NUMERICO I: Teoria','Aula 4',timedelta(hours=12), timedelta(hours=13))
##        horarioPrincipal.incluye_hora('Miercoles','INGLES: Teoria','Aula 5',timedelta(hours=13), timedelta(hours=14))
##        horarioPrincipal.incluye_hora('Miercoles','Libre','Sin Aula',timedelta(hours=14), timedelta(hours=15))
##
##        horarioPrincipal.incluye_hora('Miercoles','FISICA EXPERIMENTAL: Práctica','Laboratorio 1',timedelta(hours=15), timedelta(hours=17))
##        horarioPrincipal.incluye_hora('Miercoles','MULTIDISCIPLINAR: Práctica','Laboratorio 2',timedelta(hours=17), timedelta(hours=19))
##                                      
##        horarioPrincipal.incluye_hora('Jueves','PROGRAMACION: Teoria','Aula 1',timedelta(hours=9), timedelta(hours=10))
##        horarioPrincipal.incluye_hora('Jueves','CALCULO INTEGRAL: Teoria','Aula 2',timedelta(hours=10), timedelta(hours=11))
##        horarioPrincipal.incluye_hora('Jueves','ESTADISTICA BASICA: Teoria','Aula 3',timedelta(hours=11), timedelta(hours=12))
##        horarioPrincipal.incluye_hora('Jueves','CALCULO NUMERICO I: Teoria','Aula 4',timedelta(hours=12), timedelta(hours=13))
##        horarioPrincipal.incluye_hora('Jueves','INGLES: Teoria','Aula 5',timedelta(hours=13), timedelta(hours=14))
##        horarioPrincipal.incluye_hora('Jueves','Libre','Sin Aula',timedelta(hours=14), timedelta(hours=15))
##
##        horarioPrincipal.incluye_hora('Jueves','LABORATORIO DE FISICA II','Laboratorio 1',timedelta(hours=15), timedelta(hours=17))
##        horarioPrincipal.incluye_hora('Jueves','Libre','Sin Aula',timedelta(hours=17), timedelta(hours=19))
##
##        horarioPrincipal.incluye_hora('Viernes','PROGRAMACION: Teoria','Aula 1',timedelta(hours=9), timedelta(hours=10))
##        horarioPrincipal.incluye_hora('Viernes','CALCULO INTEGRAL: Teoria','Aula 2',timedelta(hours=10), timedelta(hours=11))
##        horarioPrincipal.incluye_hora('Viernes','ESTADISTICA BASICA: Teoria','Aula 3',timedelta(hours=11), timedelta(hours=12))
##        horarioPrincipal.incluye_hora('Viernes','CALCULO NUMERICO I: Teoria','Aula 4',timedelta(hours=12), timedelta(hours=13))
##        horarioPrincipal.incluye_hora('Viernes','INGLES: Teoria','Aula 5',timedelta(hours=13), timedelta(hours=14))
##        horarioPrincipal.incluye_hora('Viernes','Libre','Sin Aula',timedelta(hours=14), timedelta(hours=15))
##
##        horarioPrincipal.incluye_hora('Viernes','Libre','Sin Aula',timedelta(hours=15), timedelta(hours=17))
##        horarioPrincipal.incluye_hora('Viernes','LABORATORIO DE FISICA II','Laboratorio 2',timedelta(hours=17), timedelta(hours=19))

        
        for dia in ['Lunes','Martes','Miercoles','Jueves','Viernes']:
            for i in range(9,19):
                horarioPrincipal.incluye_hora(dia,'Libre', dia, 'Sin Aula', '', timedelta(hours=i), timedelta(hours=i+1))

        return Boxes(horarioPrincipal,filterTotal)

    def save(self):
        horarioPrincipal.save_timetableXML()
        print('FIN DEL GUARDADO')
        

if __name__ == '__main__':
    TestApp().run()
