from kivy.uix.button import Button
from horario import horario

class Boton(Button):
    def on_press(self, *args):
        self.disabled = True
        #self.printText()

    def printText(self):
        print("El texto del boton es:"+ self.text)
