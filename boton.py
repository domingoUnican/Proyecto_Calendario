from kivy.uix.button import Button
from horario import horario

class Boton(Button):
    select = 0
    
    def on_press(self, *args):
        self.disabled = True
        self.select = 2

    def printText(self):
        print("El texto del boton es:"+ self.text)