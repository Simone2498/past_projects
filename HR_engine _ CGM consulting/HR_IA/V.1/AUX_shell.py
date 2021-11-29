import os
from PDFAnalyzer import *
#Aggiungere eventuali nuove key
linguaggi=('Assembly','Bash','C','C#','C++','Cics','Cobol','Css','DB2','Delphi','Html','Java','Javascript','Json','Ladder','Latex','Matlab','NoSQL','Perl','PHP','Python','Q#','R','Ruby','Scala','SQL','Swift','TypeScript','UML','VisualBasic','XML','GO','Kotlin','F#')
framework=('AJAX','Angular','AngualarJS','Apache','Bootstrap','COFFE2','Doctrine','Hibernate','Ionic','JQuery','Keras','Laravel','MongoDB','Node.Js','Owl','RDF','RDFS','React','REST','SASS','Slim','Spring','SWRL','Symfony','TensoFlow','VHDL','VUE Js','RX Java','RX Javascript','.NET Core','Xamarine','ASP Net','Entity','Microsoft sql server','Django','Flask')
software=('AdobeIllustrator','AdobeIndesign','AdobePhotoshop','Access','AWS','Cad','Camtasia','DB2','DVCS','Eagle','Eclipse','Excel','Firebase','Fusion','Git','Linux','Magento','Maxqda','MySQL','PowerPoint','Solidworks','SPSS','Unix','VisualStudio','Wireshark','Word','Wordpress','Jupiter','Google Cloud','Azure','Anaconda','Hadoop','Kubernetes','Docker','Pandas')
cat_=['amministrativi','frontend','backend','fullstack','grafici','scartati'] #aggiungere anche nel seguente codice per grafica menu

#funzione di richiesta e apertura file
def fileopener(file, req=True):
    file.strip() #rimuovi spazio prima e dopo
    data=CVdata.load('./cv/report/'+file+'.txt')
    if(req):
        data.report() #mostra il riassunto del profilo
    path="./cv/"+data.getClasse()+'/'+file
    os.system('"'+path+'"') #apri il pdf originale

# Funzione fit per apprendimento continuo
def fit(file, type, newVal, req_leng=None, req_fram=None, req_soft=None):
    data=CVdata.load('./cv/report/'+file+'.txt');
    if(type=='C'): ##aggiornamento di categorizazione sbagliata
        try:
            cat_.index(newVal) #lancia un eccezione se la classe non è valida
            data.setClasse(newVal) #imposta la classe
            data.saveParametersClass() #aggiorna il dataset
            data.save('./cv/report/'+file) #salva il report
            print('Classe aggiornata')
        except(ValueError):
            print('Hai inserito una classe non valida')
    elif(type=='P'): ##aggiornamento di punteggio sbagliato
        features=data.exportArrayParametersScore(req_leng, req_fram, req_soft, newVal) #usa le richieste per calcolare un array per punteggio
        data.saveParametersScore(features) #aggiorna il dataset punteggi
        print('Modifica del punteggio inserita')


pdfA=PDFAnalyzer() #istanza di analizzatore pdf

#interfaccia utente
if __name__ == '__main__':
    print('HR Assistant caricato')
    while(1):
        print('Menu:',' 1. Ricerca', ' 2. Elenca per categoria', ' 3. Cataloga nuovi', ' 4. Addestra reti', ' 5. Correggi previsione',' 6. Salva ed Esci', sep="\n")
        cmd=int(input())
        if cmd==1: #Ricerca
            print('Categoria:',' 1. Amministrativi',' 2. Front-End',' 3. Back-End',' 4. Full-Stack',' 5. Grafici', sep='\n')
            cat=int(input())
            print('Che linguaggi sono richiesti? (case sensitive, separati da virgola)')
            req_leng=list()
            for j in range(len(linguaggi)):
                print(" ",linguaggi[j]) #
                req_leng.append(False)
            str_ling=input()
            for tkn in str_ling.replace(" ","").split(','):
                try: #controllo se è contenuto x poter usare index altrimenti lo scarto
                    if linguaggi.index(tkn)>=0 and linguaggi.index(tkn)<len(linguaggi):
                        req_leng[linguaggi.index(tkn)]=True
                except:
                    pass
            print('Che framework sono richiesti? (case sensitive, separati da virgola)')
            req_fram=list()
            for j in range(len(framework)):
                print(" ",framework[j]) #
                req_fram.append(False)
            str_fram=input()
            for tkn in str_fram.replace(" ","").split(','):
                try: #controllo se è contenuto x poter usare index
                    if framework.index(tkn)>=0 and framework.index(tkn)<len(framework):
                        req_fram[framework.index(tkn)]=True
                except:
                    pass
            print('Che software sono richiesti? (case sensitive, separati da virgola)')
            req_soft=list()
            for j in range(len(software)):
                print(" ",software[j]) #
                req_soft.append(False)
            str_soft=input()
            for tkn in str_soft.replace(" ","").split(','):
                try: #controllo se è contenuto x poter usare index
                    if software.index(tkn)>=0 and software.index(tkn)<len(software):
                        req_soft[software.index(tkn)]=True
                except:
                    pass
            lista=pdfA.listPDF('./cv/'+cat_[cat-1]) #vado nella cartella della classe richiesta e prendo tutti i candidati
            punteggi=list() #array di punteggi
            for j in range(len(lista)):
                nome=os.path.basename(lista[j])
                data=CVdata.load('./cv/report/'+nome) #carico i dati del candidato
                punteggi.append({'nome': nome, 'point': data.valuta(req_leng, req_fram, req_soft)}) #inserisco in lista una mappa con nome->punteggio
                del data #forzo un'eliminazione della classe data (non indispensabile ma evita alcuni problemi)
            punteggi=sorted(punteggi, key=lambda e : e['point'], reverse=True) #ordina i candidati in base al punteggio
            for j in range(len(punteggi)): #stampa a video
                print('-> '+punteggi[j]['nome']+' : '+str(punteggi[j]['point']))
            while(1): #modifiche manuali
                print('Vuoi visualizzare dei candidati? Y o N') #visualizza candidati
                cmd=input()
                if(cmd=='Y'):
                    print("Che candidato vuoi visualizzare? (nome file)")
                    nome=input()
                    fileopener(nome)
                print('Vuoi aggiornare degli score? Y o N') #aggiornamento punteggi
                cmd1=input()
                if(cmd1=='Y'):
                    print('Inserisci il nome del file:')
                    file=input()
                    print("Inserisci il nuovo punteggio: ")
                    newVal=float(input())
                    #print('--'+newVal)
                    fit(file,'P', newVal, req_leng, req_fram, req_soft)
                #si può aggiungere anche l'aggiornamento classe qui volendo
                else:
                    break #chiude tutto
        elif cmd==2: #Elenca
            print('Categoria:',' 1. Amministrativi',' 2. Front-End',' 3. Back-End',' 4. Full-Stack',' 5. Grafici',' 6. Scartati', sep='\n')
            cat=int(input())
            lista=pdfA.listPDF('./cv/'+cat_[cat-1]) #lista candidati di quella categoria
            print("Elenco candidati: ")
            for j in range(len(lista)):
                print(os.path.basename(lista[j])) #stampa il nome candidati
            while(1): #visualizza
                print('Vuoi visualizzare dei candidati? Y o N')
                cmd=input()
                if(cmd=='Y'):
                    print("Che candidato vuoi visualizzare? (nome file)")
                    nome=input()
                    fileopener(nome)
                else:
                    break
        elif cmd==3: #Cataloga nuovi
            pdfList=pdfA.listPDF('./cv/new') #lista di tutti i file accettabili nella cartella new
            for pdfTest in pdfList:
                data=pdfA.analyze(pdfTest, report=None) #analizza pdf
                if(data!=None): ## TODO: gestione con eccezioni più raffinata
                    ft=np.reshape(np.array(data.exportArrayParametersClass())[1:-1], (1,20))
                    data.classifica(ft) #classifica, sposta, salva report e parametri
            print('Aggiornamento catalogo avvenuto con successo!')
        elif cmd==4: #Addestra reti
            RNclass=RNClassificatore()
            RNscore=RNScore()
            RNclass.addestra()
            RNscore.addestra()
            RNclass.salva()
            RNscore.salva()
            print('Adderstramento concluso con successo!')
        elif cmd==5: #Correggi previsione
            print('Inserisci il nome del file:')
            file=input()
            print("Inserisci la nuova classe: \namministrativi\nfrontend\nbackend\nfullstack\ngrafici\nscartati")
            newVal=input()
            fit(file,'C',newVal)
        else:
            print('Fine')
            break
