from kivy.uix.button import Button
from horario import horario

class Boton(Button):
    select = 0
    ident = 0
    
    def on_press(self, *args):
        self.disabled = True
        self.select = 2
        
    def setIdent(self, ident):
        self.ident = ident

    def getIdent(self):
        return self.ident

    def printText(self):
        print("El texto del boton es:"+ self.text)
