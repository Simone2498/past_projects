import os
PATH=os.getcwd() #working directory
from empath import Empath #libreria per sentiment analysis
from numpy import mean #media su vettori il resto non serve
class LIWCAnalyzer:
    def __init__(self, path):
        self.DBkeys={}
        file=open(path, 'r') #apre il file di conversione da empath_key in categories.tsv a BF
        #emptobfDB è organizzato: 1 riga = key in BF, 1 riga con tutte le empath_key separate da \t
        line='_';  flag=0
        while(line!=''): #legge tutto il file per righe
            line=file.readline()
            if(line!=''):
                if(flag==0):
                    key=line.rstrip('\n') #estraggo la key BF
                    flag=1
                else:
                    for word in line.split('\t'):
                        self.DBkeys[word.rstrip('\n')]=key #per ogni empath_key inserisco in mappa una tupla (key_empath=>key_BF), in questo modo è efficiente la ricerca senza scorrere un vettore
                    flag=0
        file.close()
        self.lexico=Empath() #istanzio la classe empath
    def getAllDBkeys (self): #ritorna la mappa :perDebug
        return self.DBkeys
    def getKeyGroup(self, key): #ritorna la classe BF data un empath_key
            return self.DBkeys[key]
    def LIWCanalyze(self,text): #effettua l'analisi empath
        res=self.lexico.analyze(text, normalize=True) #normalize true significa lavorare per parole non per frasi, restituisce un vettore con i risultati
        BFMap={} #Big Five Method Map
        for k in res.keys(): # per ogni empath_key
            group=self.getKeyGroup(k) #classe BF
            if (not group in BFMap): #se il gruppo è nuovo
                BFMap[group]=list() #lo aggiungo e instanzio il suo vettore
            BFMap[group].append(res[k]) #altrimenti appendo solo il valore della empath_key
        #medie e manipolazioni dei dati
        for k in BFMap.keys(): #sostituisco nella mappa delle classi BF il vettore con la media dei valori, per l'uso scelto fattori opposti non devono essere necessariamente complementari
            BFMap[k]=round(mean(BFMap[k]),4)
        return BFMap #restituisco la mappa

if(__name__=='__main__'): #test
    liwc=LIWCAnalyzer(PATH+'\\empath\\emptobfDB.txt')
    text='ciao a tutti, come va?'
    liwctext=liwc.LIWCanalyze(text)
    print(str(liwctext))
