import os
from googletrans import Translator
import string
import json
import numpy as np
from keras.utils import to_categorical
from keras.models import Sequential
from keras.layers import Dense
from keras.layers import LSTM
from keras.layers import Embedding
import re
import spacy
import numpy as np
import pandas as pd
from scipy.spatial import distance

#Questo script è un insieme di tool per i test su una rete neurale molto più complessa che costituirà il cuore del vero HR_IA quando il dataset di addestramento raggiungerà l'ordine delle migliaia di campioni

class HR_IA():
    def __init__(self):
        self.emb_dim=50 #dimensione dell'embedding -> cambia aumenta info e complessità -> il resto si regola automaticamente di conseguenza -> la rete va eliminata e riaddestrata
        with open('.\\data\\vocabolario.txt','r',encoding='utf-8') as file:
            self.vocabulary=json.loads(file.readline()) #carica il vocabolario Glove
        self.max_len=100000; ## TODO: Da controllare e impostare quando avrete il dataset -> è la lughezza di caratteri massima di un singolo cv
        self.tr=Translator() #istanza del traduttore basato su API google
        self.nlp=spacy.load('en_core_web_sm') #tool per preprocessing del testo
        self.emb_weigth=HR_IA.generatore_matrice_pesi(self.emb_dim) #genera la matrice dei pesi di embedding basata sul database GloVe
        #Load o creazione di nuova rete, commentato per evitare memoria mentre effettuo test
        """
        if(os.path.exists('.\\data\\RN.h5')):
            self.RN=loadRN()
        else:
            self.RN=self.rete_neurale()
        """
    #DATI PRELIMINARI -> necessari se non si vuole usare il database GloVe, ha 400001 parole, sfruttando questo metodo si usano solo quelle necessarie riducendo la complessità, ma si riduce l'adattabilità del sistema
    @classmethod
    def analisi_preliminare(path):
        vocabulary=[""]
        nlp=spacy.load('en_core_web_sm')
        len_max=0
        n=0
        for file in os.listdir(path):
            if(re.match('.\.pdf', file)):
                text=fix_text(pdf_text_extract(path+"\\"+file))
                sentence=[]
                token=nlp(text)
                for tkn in token:
                    if(True):#not tkn.is_stop
                        try:
                            idx=vocabulary.index(tkn.lemma_)
                        except:
                            vocabulary.append(tkn.lemma_)
                            idx=0
                        sentence.append(idx)
                l=len(sentence)
                if(l>len_max):
                    len_max=l
                os.system(f"move \"{path+file}\" \"{path+str(n)}.pdf\"")
                n+=1
        # salvo il vocabolario e len_max e n
        with open('.\\data\\dati_preliminari.txt','w', encoding='utf-8') as file:
            file.write(json.dumps(n)+'\n')
            file.write(json.dumps(len_max)+'\n')
            file.write(json.dumps(vocabulary))
        #elimino il vecchio dataset perchè potrrebbe essere cambiato il vocabolario e la lunghezza
        os.remove('.\\data\\dataset.csv')
        #os.remove('.\\data\\dataset_y.csv')
    #Carica dati preliminari
    def carica_preliminari():
        with open('.\\data\\dati_preliminari.txt','r') as file:
            n=json.loads(file.readline())
            len_max=json.loads(file.readline())
            vocabulary=json.loads(file.read())
        return (n, len_max, vocabulary)

    #ANALISI DEL TESTO
    # traduce il testo text in una lingua destinazione
    def traduci(self, text, leng='it'): #lingua di destinazione
        new=''
        min=0
        max=10000 #a causa di limiti d'uso dell'api non si possono tradurre tutto il testo, divido in blocchi e poi lo riassemblo
        l=len(text)
        while(1):
            new+=self.tr.translate(text[min:max], dest=leng).text;
            if max>l:
                break
            else:
                min=max
                max+=max
        return new
    #corregge e rende uniforme il testo
    def fix_text(self, text):
        text.lower()
         #possibili aggiunte di altri filtri basati su regexp
        return text
    #estrae il testo da un pdf, lo pulisce e lo traduce per poi ritornarlo
    def pdf_text_extract(self, file_path, trad=True):
        path=file_path[:file_path.find(os.path.basename(file_path))]
        os.system("pdftotext -enc UTF-8 \""+file_path+"\" \""+path+'temp.txt'+"\"")
        with open(path+'\\temp.txt', 'r', encoding='utf-8') as file:
            if (trad) :
                text=self.traduci(file.read(), leng='it')
            else:
                text=file.read()
        text=self.fix_text(text)
        return text
    #codificatore della frase a seconda del vocabolario e max_len
    def codificatore(self, text):
        sentence=[]
        token=self.nlp(text)
        for tkn in token:
            if(len(sentence)<=self.max_len): ###
                if(True):#not tkn.is_stop
                    try:
                        idx=self.vocabulary.index(tkn.lemma_)
                    except:
                        idx=0
                    sentence.append(idx)
        for i in range(len(sentence),self.max_len): #padding per portare alla lunghezza comune
            sentence.append(0)
        return sentence

    #EMBEDDING
    #da vocabolario a matriche dei pesi embedding usando il database GloVe -> evoluzione di word2vec
    @staticmethod
    def generatore_matrice_pesi(dim=50):
        with open(f".\\data\\glove.6B\\glove.6B.{dim}d.txt", 'r', encoding='utf-8') as file: #sceglie automaticamente il file più adatto
            lines=file.readlines()
        weigth=[]
        for line in lines:
            tkn=line.split()
            weigth.append(np.array(tkn[1:], 'float'))
        return weigth

    #GESTIONE DATASET E ADDESTRAMENTO
    #due file di dataset, uno per gli input e uno per le label, il nome file permette il join senza mantenere una mappa
    def add_in_Dataset(self, path):
        for file in os.listdir(path):
            if(re.match('.\.pdf', file)):
                text=fix_text(pdf_text_extract(path+"\\"+file))
                text_cod=self.codificatore(text)
                with open('.\\data\\dataset.csv','a', encoding='utf-8') as fp:
                    print(file+',', end='', file=fp)
                    print(",".join(str(p) for p in text_cod), file=fp)
                with open('.\\data\\dataset_y.csv','a', encoding='utf-8') as fp:
                    print(file,'', sep=',', file=fp)

    #RETE NEURALE
    #rete neurale ricorsiva
    def rete_neurale(self):
        idx_max=len(self.vocabulary)
        #self.max_len
        model=Sequential()
        model.add(Embedding(idx_max+1,  self.emb_dim, input_length=self.max_len)) #codici_possibili<, len_embedding, len_sequenza
        model.add(LSTM(50, dropout=0.4, recurrent_dropout=0.2, return_sequences=True)) #layer ricorrenti per dati sequenziali
        model.add(LSTM(50, dropout=0.4, recurrent_dropout=0.2))
        model.add(Dense(20, activation='relu'))
        model.add(Dense(len(self.classi), activation='softmax'))
        model.compile(loss='categorical_crossentropy', optimizer='RMSprop', metrics=['accuracy'])
        return model

    def addestra(self):
        tx=pd.read_csv('.\\data\\dataset.csv', header=None)
        ty=pd.read_csv('.\\data\\dataset_y.csv', header=None)
        t=pd.merge(tx,ty, on=0, suffixes=('','_y'))
        x=t.drop(0, axis=1).drop('1_y', axis=1).values
        y=to_categorical(t['1_y'].values, num_classes=len(self.classi))
        #addestramento della rete_neurale
        #Con dataset maggiori si può modificare batch_size per evitare overfit
        self.RN.fit(x, y, epochs=10, batch_size=100, validation_split=0.2)
        self.RN.summary() #stampa un summary che può essere interessante per l'addestramento

    def predici(self, file_path): #ritorna la classe
        x=self.codificatore(pdf_text_extract(file_path))
        y=self.RN.predict(x)
        return self.classi[y.index(max(y))]

    def salvaRN(self):
        self.RN.save('.\\data\\RN.h5')
        print('Rete salvata')
    def loadRN():
        return load_model('.\\data\\RN.h5')

    #TOOL
    def cos_dist(x,y): #distanza del coseno, tra due vettori ottenuti come codifica embedding di parole permette di misurare la loro similarità semantica, quanto le due parole sono simili di significato? analogo per frasi
    #utile per gestione dell'attenzione in future implementazioni più complesse per insegnare alla rete a basarsi su più fonti per giungere ad una conclusione
        return distance.cosine(x,y)


if __name__ == '__main__': #test
    waw=HR_IA()
