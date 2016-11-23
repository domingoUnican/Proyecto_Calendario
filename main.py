# -*- coding: utf-8 -*-

from kivy.app import App
from kivy.lang import Builder
from kivy.uix.floatlayout import FloatLayout
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.dropdown import DropDown
from kivy.uix.button import Button
from kivy.uix.listview import ListView
from datetime import timedelta
from horario import horario

class Boxes(FloatLayout):
    def __init__(self, h, **kwargs):
        super(Boxes, self).__init__(**kwargs)
        for dia in h.dias:
            bx_m = BoxLayout(orientation='vertical')#mañana
            bx_t = BoxLayout(orientation='vertical')#tarde
            nombre = dia
            bx_m.add_widget(Button(text=nombre, size_hint_y=0.02))
            bx_t.add_widget(Button(text=nombre, size_hint_y=0.02))
            for texto, porcentaje in h.clases_manana(nombre):
                btn = Button(text = texto,
                             size_hint_y = porcentaje)
                bx_m.add_widget(btn)
            self.ids['_morning'].add_widget(bx_m)
            for texto, porcentaje in h.clases_tarde(nombre):
                btn = Button(text = texto,
                             size_hint_y = porcentaje)
                bx_t.add_widget(btn)
            self.ids['_afternoon'].add_widget(bx_t)
        profes = DropDown()
        aulas =  DropDown()
        asigns = DropDown()
        for index in range(10):
            btn = Button(text='profe %d' % index, size_hint_y=None, height=44)
            # Mostrar el menu
            btn.bind(on_release=lambda btn: profes.select(btn.text))

            # add el boton dentro del dropdown
            profes.add_widget(btn)
        for index in range(10):
            btn = Button(text='aulas %d' % index, size_hint_y=None, height=44)
            # Mostrar el menu
            btn.bind(on_release=lambda btn: aulas.select(btn.text))

            # add el boton dentro del dropdown
            aulas.add_widget(btn)
        for index in range(10):
            btn = Button(text='profe %d' % index, size_hint_y=None, height=44)
            # Mostrar el menu
            btn.bind(on_release=lambda btn: profes.select(btn.text))

            # add el boton dentro del dropdown
            profes.add_widget(btn)
        for index in range(10):
            btn = Button(text='asignatura %d' % index, size_hint_y=None, height=44)
            # Mostrar el menu
            btn.bind(on_release=lambda btn: aulas.select(btn.text))

            # add el boton dentro del dropdown
            asigns.add_widget(btn)
        profesbutton = Button(text = 'Profesores', size_hint = (None, None))
        profesbutton.bind(on_release=profes.open)
        aulasbutton= Button(text = 'Aulas', size_hint = (None, None))
        aulasbutton.bind(on_release=aulas.open)
        asignsbutton= Button(text = 'Asignaturas', size_hint = (None, None))
        asignsbutton.bind(on_release=asigns.open)
        aulas.bind(on_select=lambda instance, x: setattr(aulasbutton, 'text', x))
        profes.bind(on_select=lambda instance, x: setattr(profesbutton, 'text', x))
        asigns.bind(on_select=lambda instance, x: setattr(asignsbutton, 'text', x))
        self.ids['_main'].add_widget(profesbutton)
        self.ids['_main'].add_widget(aulasbutton)
        self.ids['_main'].add_widget(asignsbutton)
class TestApp(App):
    def build(self):
        h = horario(['Lunes','Martes','Miércoles','Jueves','Viernes'],timedelta(hours=9))
        h.incluye_hora('Lunes','Mates',timedelta(hours=9), timedelta(hours=10))
        h.incluye_hora('Lunes','Ingles',timedelta(hours=10), timedelta(hours=11))
        h.incluye_hora('Martes','Lengua',timedelta(hours=9), timedelta(hours=11))
        h.incluye_hora('Miércoles','Frances',timedelta(hours=9), timedelta(hours=11))
        h.incluye_hora('Jueves','Frances',timedelta(hours=9), timedelta(hours=10))
        h.incluye_hora('Jueves','Ingles',timedelta(hours=10), timedelta(hours=11))
        h.incluye_hora('Viernes','Frances',timedelta(hours=9), timedelta(hours=11))
        for dia in ['Lunes','Martes','Miércoles','Jueves','Viernes']:
            for i in range(11,19):
                h.incluye_hora(dia,'Libre',timedelta(hours=i), timedelta(hours=i+1))

        return Boxes(h)

if __name__ == '__main__':
    TestApp().run()
