from kivy.uix.button import Button
from horario import horario

class Boton(Button):
    select = 0
    ident = 0
    asigID = ''
    aulaID = ''
    
    def on_press(self, *args):
        self.disabled = True
        self.select = 2
        
    def setIdent(self, ident):
        self.ident = ident

    def getIdent(self):
        return self.ident
    
    def setAsigID(self, asigID):
        self.asigID = asigID

    def getAsigID(self):
        return self.asigID
    
    def setAulaID(self, aulaID):
        self.aulaID = aulaID

    def getAulaID(self):
        return self.aulaID

    def printText(self):
        print("El texto del boton es:"+ self.text)

    def setText(self, text):
        self.text = text
