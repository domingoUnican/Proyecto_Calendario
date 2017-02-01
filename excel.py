
class excel:

    def lectura(direc, array):
        i = 0
        # Iterate over the lines of the file
        with open(direc, encoding = 'utf8') as csvfile:
            for line in csvfile:
                # process line
                d = line.strip()
                array.append(d)
                i = i +1
        return array
