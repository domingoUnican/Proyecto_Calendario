# -*- coding: utf-8 -*-


from kivy.app import App
from kivy.lang import Builder
from kivy.uix.floatlayout import FloatLayout
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.button import Button
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


class TestApp(App):
    def build(self):
        h = horario(['Lunes','Martes','Miércoles'],timedelta(hours=9))
        h.incluye_hora('Lunes','Mates',timedelta(hours=9), timedelta(hours=10))
        h.incluye_hora('Lunes','Ingles',timedelta(hours=10), timedelta(hours=11))
        h.incluye_hora('Martes','Lengua',timedelta(hours=9), timedelta(hours=11))
        h.incluye_hora('Miércoles','Frances',timedelta(hours=9), timedelta(hours=11))
        for dia in ['Lunes','Martes','Miércoles']:
            for i in range(11,19):
                h.incluye_hora(dia,'Libre',timedelta(hours=i), timedelta(hours=i+1))
        return Boxes(h)

if __name__ == '__main__':
    TestApp().run()
