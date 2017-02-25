def save_timetableXML(self):
        print('GUARDANDO XML')

        #Inicializaciones
        root = etree.Element('HighSchoolTimetableArchive', id="Root")
        doc = etree.ElementTree(root)
        instances = etree.SubElement(root, "Instancias")
        instance1 = etree.SubElement(root[0], "Instancias", id="1")
        instance2 = etree.SubElement(root[0], "Instancias", id="2")
        etree.SubElement(instance1, "Metadatos", id="Meta")
        etree.SubElement(instance1, "Times", id="Tm")
        etree.SubElement(instance1, "Resources", id="Res")
        etree.SubElement(instance1, "Events", id="Ev")
        etree.SubElement(instance1, "Constraints", id="Cons")
        
        etree.SubElement(root, "Soluciones")
        solucion1 = etree.SubElement(root[1], "SolucionManana", id="1")

        #Recorro el horario guardando sus datos
        #pos = self.dias.index('Lunes')

        for pos in self.dias:
            print(pos)
            solucionDia = etree.SubElement(solucion1, pos, id=pos)
            pos = self.dias.index(pos)
            for asignatura, hora in self.h_dia[pos]:
                etree.SubElement(solucionDia, asignatura, id=asignatura)
                #hr = str(hora)
                #etree.SubElement(solucionDia, hora, id=hr)
                print(asignatura)
                print(hora)
        
        #solucion2 = etree.SubElement(root[1], "SolucionTarde", id="2")


        #Guardado de datos
        outFile = open('homemade.xml', 'wb')
        doc.write(outFile)
        print('XML GUARDADO')
        print('Estos son los datos guardados')
        print(etree.tostring(root))
