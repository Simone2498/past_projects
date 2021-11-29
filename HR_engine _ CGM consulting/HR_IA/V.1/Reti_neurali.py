from keras.models import Sequential
from keras.layers import Dense
#from keras.callbacks import EarlyStopping
#from keras.layers import Dropout
#from keras.utils import normalize
from keras.models import load_model
import pandas as pd
import numpy as np
import os
PATH=os.getcwd()

class RNClassificatore(): #classificatore multi-classe

    def __init__(self):
        self.data=pd.read_csv('./RNdata/tableClassificatore.csv', index_col=0)
        self.data=self.data.dropna() #elimina righe non valide
        self.x=self.data.drop(['id','classe'],axis=1).values #estrai gli ingressi eliminando l'id utente e la label
        self.y=pd.get_dummies(self.data['classe'], columns=['classe']) #attenzione, se non sono presenti tutte le classi si perdono delle dimensioni x questo ci sono delle righe fake
        if not os.path.isfile('.\\RNdata\\RNClassificatore.h5'):
            #Primo avvio e modello non ancora addestrato
            #La rete è molto semplice per avere pochi parametri da addestrare, possibili migliorie sono aggiungere strati densi (nascosti), filtrando con dropout per evitare l'overfit sul dataset di addestramento
            #La lambda EarlyStopping può essere utile a fermare l'addestramento quando si raggiunge il massimo, in questo momento è deleterio perchè c'è overfitting sul dataset di addestramento
            self.model=Sequential()
            self.model.add(Dense(10, activation='relu', input_dim=self.x.shape[1]))
            self.model.add(Dense(6, activation='softmax'))
            self.model.compile(loss='categorical_crossentropy', optimizer='sgd', metrics=['accuracy']) #Possibile miglioria con optimizer="Adam"
        else:
            #Modello già presente e addestrato -> lo carico
            self.model=load_model('.\\RNdata\\RNClassificatore.h5')

    def get(self): #ottieni modello e dati per test
        return self.model, self.x, self.y

    def predici(self, x): #predici una classe
        classi=['amministrativi','frontend','backend','fullstack','grafici','scartati']
        #Predict ritorna la probabilità di appartenenza ad una data classe, trovando il max ottengo l'indice della classe
        return classi[np.argmax(self.model.predict(x), axis=1)[0]]

    def addestra(self): #addestra la rete, con dataset grandi epochs può essere ridotto, inoltre batch_size può essere maggiore per ridurre l'overfitting
        self.model.fit(self.x, self.y, epochs=100, batch_size=1)

    def salva(self): #salva la rete neurale
        self.model.save('.\\RNdata\\RNClassificatore.h5')

class RNScore(): #classificatore multi-classe
    def __init__(self):
        self.data=pd.read_csv('./RNdata/tableScore.csv', index_col=0)
        self.data=self.data.dropna()
        self.x=self.data.drop(['id','score'],axis=1).values
        self.y=self.data['score']
        if not os.path.isfile('.\\RNdata\\RNScore.h5'):
            #Primo avvio e modello non ancora addestrato
            #si tratta di un singolo neurone non di una rete neurale
            self.model=Sequential()
            self.model.add(Dense(1, activation='linear', input_dim=self.x.shape[1]))
            self.model.compile(loss='mean_squared_error', optimizer='sgd', metrics=['accuracy'])
        else:
            #Modello già presente e addestrato
            self.model=load_model('.\\RNdata\\RNScore.h5')

    def get(self):
        return self.model, self.x, self.y

    def predici(self, x):
        return quantizzatore(self.model.predict(x), 0.5)

    def addestra(self):
        self.model.fit(self.x, self.y, epochs=100, batch_size=1)

    def salva(self):
        self.model.save('.\\RNdata\\RNScore.h5')

#Per ottenere dei punteggi discreti e più simili alla logica umana servirebbe una rete multi-classe, ma servono 20 classi in uscita
#Alternativa per ridotto dataset -> 1 uscita lineare che poi discretizzo a step di 0.5, la rete imparerà ad assegnare voti da 0 a 10
def quantizzatore (val, step):
    n = val%step
    if n!=0:
        if int(val/step): #se minore di step approssimo a zero
            if n<=2.5:
                return val+step-n
            else:
                return val-n
        else:
            return step
    else:
        return val

if __name__=='__main__': #test
    rn= RNClassificatore()
    rn.addestra()
    model, x, y = rn.get()
    model.summary()
    x_=x[0].reshape((1,20))
    y_=model.predict(x_, batch_size=1); y_
    i=np.argmax(y_, axis=1);i
    classi=['amministrativi','frontend','backend','fullstack','grafici','scartati']
    classi[i[0]]
    model.evaluate(x_,y_)
    y.head()
