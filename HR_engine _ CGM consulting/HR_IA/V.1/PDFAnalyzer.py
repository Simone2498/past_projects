from pdfminer.pdfparser import PDFParser
from pdfminer.pdfdocument import PDFDocument
from pdfminer.pdfpage import PDFTextExtractionNotAllowed, PDFPage
from pdfminer.pdfinterp import PDFResourceManager
from pdfminer.layout import LAParams
from pdfminer.pdfinterp import PDFPageInterpreter
from pdfminer.converter import PDFPageAggregator
from pdfminer.layout import LTTextBox,LTTextLine
import string
import re #regular expression
import os
import json #per i/o su file
PATH=os.getcwd() #directory di lavoro
from LIWCAnalyzer import * #analisi LIWC
from Reti_neurali import * #Reti neurali di class e score
import pandas as pd
import numpy as np

class PDFAnalyzer:
        def  __init__(self):
            #Tutte le macros, ricorda di effettuare le modifiche anche negli altri file!
            self.social=['linkedin','facebook','instagram','live','git']
            self.lenguage=[('inglese','english'),('francese', 'french'),('spagnolo','spanish'),('italiano','italian'),('tedesco','german'),('russo','russian'),('cinese','chinese')]
            self.degree=[('lauea[ ]?magistrale','master'),('laurea[ ]?triennale','backelor[\'s]?','laurea','university','ingegneria'),('maturità','high[ ]?school','perito','diploma','liceo','istituto[ ]?tecnico'),('certificates','certificati','licences','attestato'),('studio[\w\s\' ]*estero', 'study[ ]?abroad')]
            self.programming=['assembly','bash','c','c#','c\+\+','cics','cobol','css','db2','delphi','html','java','javascript','json','ladder','latex','matlab','nosql','perl','php','python','q#','r','ruby','scala','sql','swift','typescript','uml','visual[ ]?basic','xml','go','kotlin','f#']
            self.framework=['ajax','angular','anguar[ ]?js','apache','bootstrap','coffe[ ]?2','doctrine','hibernate','ionic','jquery','keras','laravel','mongodb','node.js','owl','rdf','rdfs','react','rest','sass','slim','spring','swrl','symfony','tensorflow','vhdl','vue[ -]?js','rx[ -]?java','rx[ -]?javascript','\.net[ -]?core','xamarine','asp\.net','entity','microsoft[ -]?sql[ -]?server','django','flask']
            self.software=['[adobe]?[ ]?illustrator','[adobe]?[ ]?indesign','[adobe]?[ ]?photoshop','access','aws','cad','camtasia','db2','dvcs','eagle','eclipse','excel','firebase','fusion','git','linux','magento','maxqda','mysql','power[ ]?point','solidworks','spss','unix','visual[ ]?studio','wireshark','word','wordpress','jupiter','google[ -]?cloud','azure','anaconda','hadoop','kubernetes','docker','pandas']
            self.drivingLicence=[('patente[ ]?[abc]','driving[ ]?license'),('automunit[oa]','own[ ]?car')]
            self.trip=['available[]?for[ ]?travel','disponibile[ ]?a[ ]?trasferimenti','disponibilie[ ]?a[ ]?viaggiare']
            self.extracurricula=[('brevett[oi]', 'patent[s]?'),('privatamente[ ]?sviluppo', 'privately[ ]?develop'),('progett[oi]', 'project[s]?'),('attività[ ]?di[ ]?ricerca','reserach[ ]?activit[y|ies]'),('tutor[ ]?didattico','didactic[ ]?tutor'),('arduino','arduino'),('raspberry','raspberry'),('android','android'),('ios','ios'),('pubblicazion[ei]','pubblication[s]?'),('app|applicazion[ei]','app|application[s]?'),('internship','internship'),('hackathlon','hackathlon'),('riconoscimenti|premi','awards'),('attività[ ]?extra[ -]?curriculari','extra[ -]?curricular[ ]?activit[y|(ies)]'),('volontariato','volunteering'),('videogioc[o|(?:hi)]','(?:video)?game[s]?'),('associazione[ ]?studentesc[ae]','student[ ]?association[s]?'),('esperienz[ae][ ]?internazionale','international[ ]?experience[s]?'),('prototi[oi]','prototype[s]?'),('team[ ]?working','team[ ]?working')]
            self.oldExperiences=[('dipendente[ ]?presso','employee[ ]?at'),('programmatore','programmer'),('consulente','consultant'),('direttore','director'),('esperienz[ae][ ]?lavorativ[ae]','working[ ]?experience[s]?'),('lavor[o|(ato)][.\s]*come','work[ed]?[ ]?like'),('assistenza','assistance'),('manutenzione','maintenance'),('vendita','sale|selling'),('gestione','management'),('campagn[ae][ ]?di[ ]?comunicazione','communication[ ]?campaign[s]? '),('analista','analyst'),('docenza','teaching'),('junior','junior'),('senior','senior'),('lavori[ ]?precedenti','previous[ ]?work[s]?'),('tirocinio[ ]?curriculare','curricular[ ]?internship'),('mentore','mentor')]
            self.skills=[('sviluppatore|programmatore','software[ ]?[(?:developer)|(?:development)|(?:design)]'),('back[ -]?end','back[ -]?end'),('front[ -]?end','front[ -]?end'),('full[ -]?stack','full[ -]?stack'),('seo','pubblicità'),('[(?:cyber)|(?:server)][ ]?security','crittografia'),('stampa[ ]?3d','3d[ ]?printing'),('[(?:deep)|(?:machine)|(?:reinforcement)][ -]?learning','[(?:deep)|(?:machine)|(?:reinforcement)][ -]?learning'),('text[ -]?mining','data[ -]?science'),('designer|grafic[ao]','loghi|brochure'),('video','audio'),('mobile','responsive'),('web','landing[ -]?page'),('comunicazione[ ]?digitale'),('(?:human[ ]?resource)|(?:HR)','risorse[ ]?umane'),('management','manager'),('project[ ]?[(?:manager)|(?:coordinator)]','coordinatore|responsabile')]
            self.LIWCanalyzer=LIWCAnalyzer(PATH+'\\empath\\emptobfDB.txt')

        def PDFtoText(self, path, report=0): #importa il pdf sotto forma di studio
            #"""
            fp=open(path, 'rb') #apri il file pdf
            parser=PDFParser(fp) #associa un parser
            doc=PDFDocument(parser) #associa un documento
            rsrcManager=PDFResourceManager() #associa un manager delle risorse all'interno del documento
            laparams=LAParams() #parametri di layout
            device=PDFPageAggregator(rsrcManager, laparams=laparams)
            interpreter=PDFPageInterpreter(rsrcManager, device) #interprete della pagina

            text=''
            for page in PDFPage.create_pages(doc): #per ogni pagina del documento
                interpreter.process_page(page) #processa la pagina
                layout=device.get_result() #ottieni il layout
                for lt_obj in layout: #per ogni oggetto nella pagina
                    if isinstance(lt_obj, LTTextBox) or isinstance(lt_obj, LTTextLine): #se sono stati identificati come gruppo di linee o linea singola
                        text+=' '+lt_obj.get_text()+' ' #estrai il testo dall'oggetto e concatenalo
            fp.close()
            """
            #Alternativa con pdftotext
            os.chdir('.')
            os.system('pdftotext -enc UTF-8 "'+path+'" ".\\cv\\'+os.path.splitext(os.path.basename(path))[0]+'.text"')
            fp=open('.\\cv\\'+os.path.splitext(os.path.basename(path))[0]+'.text','r')
            text=fp.read()
            fp.close()
            """
            return text

        def textCleaner(self, text): #ripulisce il testo da eventuali errori di import rendendolo più adatto alle successive manipolazioni
            stampabili=set(string.printable) #elimina caratteri non interpretati
            text=re.sub('\n+',' ',text)#elimina gli a capo
            text=re.sub('[ ,_]+',' ', text)#inserisci come unico separatore il singolo spazio
            text=text.lower()#porta tutto in minuscolo
            return text

        def listPDF(self, pathRoot): #elenca tutti i file pdf presenti in una directory pathRoot
            tmp=os.getcwd()
            lista=list()
            os.chdir(pathRoot)
            for cv in os.listdir():
                if re.match('.+.pdf',cv): #identifica solo i .pdf
                   lista.append(pathRoot+'\\'+cv)
            os.chdir(tmp)
            return lista #lista contiene i path di ogni file

            ### FUNZIONI DI ANALISI A MATCH SU TOKEN ###
        def getEmail(self, text): #lista di mail
            return re.findall('[A-z0-9\.\+_-]+?@[A-z0-9\._-]+?\.[A-z]{2,6}', text)

        def getSocial(self, text): #lista di contatti social
            lista=list()
            for x in self.social:
                for account in re.findall('[A-z0-9:\.\+\\/_-]*'+x+'[A-z0-9:\.\+\\/_-]{2}[A-z0-9:\.\+\\/_-]+',text):
                    lista.append(account)
            return lista

        def getCell(self, text): # lista di numeri di telefono
                return re.findall('(?:(?:00|\+)??\d{2}[\. ]??)??\d{3}[\. ]??(?:\d{6,7}|(?:\d{3}[\. ]\d{3,4}))',text)

        def getLengSkills(self, text): #lista di bool delle lingue conosciute
            array=list()
            for idx in range(len(self.lenguage)):
                if(re.findall('[A-z0-9:\.\+\\/_ -]*(?:(?:'+self.lenguage[idx][0]+')|(?:'+self.lenguage[idx][1]+'))[A-z0-9:\.\+\\/_ -]+',text)):
                    array.insert(idx,True)
                else:
                    array.insert(idx,False)
            return array

        def privacyCheck(self, text): #Bool sulla presenza dell'autorizzazione al trattamento dei dati personali
            return re.findall('(?:gpdr[ ]?679/16)|(?:296/2003)|(?:art[.]?[ ]?13[ ]?[\w. ]*lgs[.]?30)|(?:legge[ ]?31[/-]12[/-]1996)|(?:codice[ ]?in[ ]?materia[ ]?di[ ]?protezione)|(?:autorizzo[\w ]*trattamento)|(?:196/2003)',text)!=[]

        def getDegree(self, text): #ritorna vettore [titolo di studio in label ordinali, bool presenza di certificati, bool studio all'estero]
            array=['',False,False]
            for idx in range(len(self.degree)):
                str='(?:(?:'+self.degree[idx][0]+')'
                for j in range(len(self.degree[idx])):
                    if j>0:
                        str+='|(?:'+self.degree[idx][j]+')'
                str+=')'
                if(re.findall('[A-z0-9:\.\+\\/_ -]*'+str+'[A-z0-9:\.\+\\/_ -]+',text)):#titolo di studio ordinale -> magistale,laurea,diploma,nessuna
                    if(idx==0):
                        array[0]='magistrale'
                        idx=3
                        break
                    elif (idx==1):
                        array[0]='laurea'
                        idx=3
                        break
                    elif (idx==2):
                        array[0]='diploma'
                        idx=3
                        break
                    elif (idx==3):
                        array[1]=True
                        break
                    else:
                        array[2]=True
                        break
            if array[0]=='': array[0]='nessuna'
            return array

        def getHardSkills(self, text): # ritorna tre liste di bool, con liguaggi di programmazione conosciuti, framework e software
            prog_=[]
            fram_=[]
            soft_=[]
            for j in range(len(self.programming)):
                if(re.findall(' '+self.programming[j]+' ',text)):
                    prog_.insert(j,True)
                else:
                    prog_.insert(j,False)
            for j in range(len(self.framework)):
                if(re.findall(' '+self.framework[j]+' ',text)):
                    fram_.insert(j,True)
                else:
                    fram_.insert(j,False)
            for j in range(len(self.software)):
                if(re.findall(' '+self.software[j]+' ',text)):
                    soft_.insert(j,True)
                else:
                    soft_.insert(j,False)
            return prog_, fram_, soft_

        def getDrivingLicense(self, text): #ritorna un array bool con patente, automunito
            array=list()
            if(re.findall('[A-z0-9:\.\+\\/_ -]*(?:(?:'+self.drivingLicence[1][0]+')|(?:'+self.drivingLicence[1][1]+'))[A-z0-9:\.\+\\/_ -]+',text)):
                array.insert(0,True)
                array.insert(1,True)
                return array
            else:
                array.insert(1,False)
            if(re.findall('[A-z0-9:\.\+\\/_ -]*(?:(?:'+self.drivingLicence[0][0]+')|(?:'+self.drivingLicence[0][1]+'))[A-z0-9:\.\+\\/_ -]+',text)):
                array.insert(0,True)
            else:
                array.insert(0,False)
            return array

        def getTrip(self, text): #bool se disponibilità a trasferte
            str='[A-z0-9:\.\+\\/_ -]*(?:(?:'
            l=len(self.trip)-1
            for i in range(l):
                str+=self.trip[i][0]+')|(?:'+self.trip[i][1]+')|(?:'
            str+=self.trip[l][0]+')|(?:'+self.trip[l][1]+'))[A-z0-9:\.\+\\/_ -]+'
            return re.findall(str,text)!=[]

        def getExtracurricula(self,text): #punteggio delle attività extracurriculari per successivi livelli di analisi
            score=0
            for idx in range(len(self.extracurricula)):
                if(re.findall('[A-z0-9:\.\+\\/_ -]*(?:(?:'+self.extracurricula[idx][0]+')|(?:'+self.extracurricula[idx][1]+'))[A-z0-9:\.\+\\/_ -]+',text)):
                    score+=1
            return score/len(self.extracurricula)

        def getOldExperiences(self, text): #punteggio delle esperienze pregresse per successivi livelli di analisi
            score=0
            for idx in range(len(self.oldExperiences)):
                if(re.findall('[A-z0-9:\.\+\\/_ -]*(?:(?:'+self.oldExperiences[idx][0]+')|(?:'+self.oldExperiences[idx][1]+'))[A-z0-9:\.\+\\/_ -]+',text)):
                    score+=1
            return score/len(self.oldExperiences)

        def getSkills(self, text): #array di bool delle capacità citate nel cv, necessario per rete neurale di classificazione
            array=list()
            for idx in range(len(self.skills)):
                if(re.findall('[A-z0-9:\.\+\\/_ -]*(?:(?:'+self.skills[idx][0]+')|(?:'+self.skills[idx][1]+'))[A-z0-9:\.\+\\/_ -]+',text)):
                    array.insert(idx,True)
                else:
                    array.insert(idx,False)
            return array

        def analyze(self, nome, report='stdout'): #analisi di un singolo pdf e salvataggio dei dati in un istanza di CVdata ritornata, report stampa a video (report='stdout') o su file il report dei dati letti(report=file_path)
            text=self.textCleaner(self.PDFtoText(nome)) #importa e pulisce i dati
            if(text!=''):
                #estrai tutti i dati e li salva in un istanza CVdata
                tmp=self.getDegree(text)
                prog_, fram_, soft_ = self.getHardSkills(text)
                tmp1=self.getDrivingLicense(text)
                data=CVdata(os.path.basename(nome),self.getEmail(text),self.getSocial(text),self.getCell(text),self.getLengSkills(text),self.privacyCheck(text),tmp[0],tmp[1],tmp[2],prog_,fram_,soft_,tmp1[0],tmp1[1],self.getTrip(text),self.getExtracurricula(text),self.getOldExperiences(text), self.getSkills(text), self.LIWCanalyzer.LIWCanalyze(text))
                if report!=None:#no report
                    data.report(report)
                return data
            else:
                return None

class CVdata():

    # label generiche (no regex) per stampa report
    lingue_=('Inglese','Francese','Spagnolo','Italiano','Tedesco','Russo','Cinese')
    linguaggi_=('Assembly','Bash','C','C#','C++','Cics','Cobol','Css','DB2','Delphi','Html','Java','Javascript','Json','Ladder','Latex','Matlab','NoSQL','Perl','PHP','Python','Q#','R','Ruby','Scala','SQL','Swift','TypeScript','UML','VisualBasic','XML','Go','Kotlin','F#')
    framework_=('AJAX','Angular','AngualarJS','Apache','Bootstrap','COFFE2','Doctrine','Hibernate','Ionic','JQuery','Keras','Laravel','MongoDB','Node.Js','Owl','RDF','RDFS','React','REST','SASS','Slim','Spring','SWRL','Symfony','TensoFlow','VHDL','Vue Js','RX java','RX javascript','.Net Core','Xamarine','ASP.NET','Entity','Microsoft SQL Server','Django','Flask')
    software_=('AdobeIllustrator','AdobeIndesign','AdobePhotoshop','Access','AWS','Cad','Camtasia','DB2','DVCS','Eagle','Eclipse','Excel','Firebase','Fusion','Git','Linux','Magento','Maxqda','MySQL','PowerPoint','Solidworks','SPSS','Unix','VisualStudio','Wireshark','Word','Wordpress','Jupiter','Google Cloud','Azure','Anaconda','Hadoop','Kubernetes','Docker','Pandas')

    def __init__(self, nome='', email=[], social=[], cell=[], lingue=[], privacy=False, titoloDiStudio='None', certificati=False, studioEstero=False, linguaggi=[], framework=[], software=[], patente=False, automunito=False, trasferte=False, punteggioExtracurriculari=0.0, punteggioEsperienze=0.0, skills=[], liwc=[], classe=None):
        self.nome = nome #nome del cv
        self.email =email
        self.social =social
        self.cell =cell
        self.lingue=lingue
        self.privacy =privacy #consenso al trattamento esplicito
        self.titoloDiStudio=titoloDiStudio
        self.certificati=certificati
        self.studioEstero=studioEstero
        self.linguaggi =linguaggi #di programmazione
        self.framework =framework
        self.software =software
        self.patente =patente
        self.automunito =automunito
        self.trasferte =trasferte #disponibilità a
        self.punteggioExtracurriculari =punteggioExtracurriculari
        self.punteggioEsperienze =punteggioEsperienze
        self.skills=skills #bool per classificazione
        self.liwc=liwc #risultato LIWC in metriche BF
        self.classe=classe # classe disponibile dopo il metodo di predizione
        fp=open('./RNdata/indexMap.text','r')
        self.map=json.loads(fp.read()) #mappa di associazione codice utente <-> nome cv.pdf
        fp.close()
        # @TODO in caso di multi-utente gestire l'accesso concorrente
        self.RNclass=RNClassificatore() #rete di classificazione
        self.RNscore=RNScore() #rete di punteggio

    # Getter utili
    def getLingue(self):
        a=list()
        for i in range(len(self.lingue)):
            if self.lingue[i]:
                a.append(self.lingue_[i])
        return a
    def getLinguaggi(self):
        a=list()
        for i in range(len(self.linguaggi)):
            if self.linguaggi[i]:
                a.append(self.linguaggi_[i])
        return a
    def getFramework(self):
        a=list()
        for i in range(len(self.framework)):
            if self.framework[i]:
                a.append(self.framework_[i])
        return a
    def getSoftware(self):
        a=list()
        for i in range(len(self.software)):
            if self.software[i]:
                a.append(self.software_[i])
        return a
    def getClasse(self):
        return self.classe
    def setClasse(self, val):
        self.classe=val

    # Stampa del report
    def report(self, file='stdout'): #se file='stdout'-> stampa a video altrimenti file=path_file_report.txt
        if file=='stdout':
            print('-----------',self.nome,'----------')
            print('Email:',self.email ,sep='\t')
            print('Social:',self.social,sep='\t')
            print('Cell:', self.cell,sep='\t')
            print('Lingue:',self.getLingue(),sep='\t')
            print('Privacy:',self.privacy,sep='\t')
            print('Titolo di Studio:', self.titoloDiStudio,sep='\t')
            print('Certificati:', self.certificati,sep='\t')
            print('Studio all\'estero', self.studioEstero,sep='\t')
            print('Linguaggi:', self.getLinguaggi(),sep='\t')
            print('Framework:',self.getFramework(),sep='\t')
            print('Software:',self.getSoftware(),sep='\t')
            print('Patente:',self.patente,sep='\t')
            print('Automunito:',self.automunito,sep='\t')
            print('Trasferte:',self.trasferte,sep='\t')
            print('Valutazione Attività Extra:', self.punteggioExtracurriculari,sep='\t')
            print('Valutazione Esperienze:', self.punteggioEsperienze,sep='\t')
            print('SkillParams:', self.skills,sep='\t')
            print('LIWC:', self.liwc,sep='\t')
            print('Classe: ', self.classe, sep='\t')
        else:
            fpt=open(file,'w')
            fpt.write('-----------',self.nome,'----------')
            fpt.write('Email:',json.dumps(self.email),sep='\t')
            fpt.write('Social:',json.dumps(self.social),sep='\t')
            fpt.write('Cell:', json.dumps(self.cell),sep='\t')
            fpt.write('Lingue:',json.dumps(self.getLingue()),sep='\t')
            fpt.write('Privacy:',json.dumps(self.privacy),sep='\t')
            fpt.write('Titolo di Studio:', json.dumps(self.titoloDiStudio),sep='\t')
            fpt.write('Certificati:', json.dumps(self.certificati),sep='\t')
            fpt.write('Studio all\'estero', json.dumps(self.studioEstero),sep='\t')
            fpt.write('Linguaggi:', json.dumps(getLingue()),sep='\t')
            fpt.write('Framework:',json.dumps(getFramework()),sep='\t')
            fpt.write('Software:',json.dumps(getSoftware()),sep='\t')
            fpt.write('Patente:',json.dumps(self.patente),sep='\t')
            fpt.write('Automunito:',json.dumps(self.automunito),sep='\t')
            fpt.write('Trasferte:',json.dumps(self.trasferte),sep='\t')
            fpt.write('Valutazione Attività Extra:', json.dumps(self.punteggioExtracurriculari),sep='\t')
            fpt.write('Valutazione Esperienze:', json.dumps(self.punteggioEsperienze),sep='\t')
            fpt.write('Skills:', json.dumps(self.skills),sep='\t')
            fpt.write('LIWC:', json.dumps(self.liwc),sep='\t')
            fpt.write('Classe: ', json.dumps(self.classe), sep='\t')
            fpt.close()

    # Save e load su txt temporaneo ->  @TODO adattare all'infrastruttura richiesta
    def save(self, CV):
        name=CV+'.txt'
        fp=open(name,'w')
        fp.write(json.dumps(self.nome)+'\n')
        fp.write(json.dumps(self.email)+'\n')
        fp.write(json.dumps(self.social)+'\n')
        fp.write(json.dumps(self.cell)+'\n')
        fp.write(json.dumps(self.lingue)+'\n')
        fp.write(json.dumps(self.privacy)+'\n')
        fp.write(json.dumps(self.titoloDiStudio)+'\n')
        fp.write(json.dumps(self.certificati)+'\n')
        fp.write(json.dumps(self.studioEstero)+'\n')
        fp.write(json.dumps(self.linguaggi)+'\n')
        fp.write(json.dumps(self.framework)+'\n')
        fp.write(json.dumps(self.software)+'\n')
        fp.write(json.dumps(self.patente)+'\n')
        fp.write(json.dumps(self.automunito)+'\n')
        fp.write(json.dumps(self.trasferte)+'\n')
        fp.write(json.dumps(self.punteggioExtracurriculari)+'\n')
        fp.write(json.dumps(self.punteggioEsperienze)+'\n')
        fp.write(json.dumps(self.skills)+'\n')
        fp.write(json.dumps(self.liwc)+'\n')
        fp.write(json.dumps(self.classe)+'\n')
        print('Salvataggio avvenuto...')
        fp.close()
    @classmethod
    def load(cls, CV): #carica l'istanza da un file pre-salvato
        #print(CV)
        if(not re.match('.*.txt',CV)):
            CV=CV+'.txt'
        fp=open(CV,'r')
        obj=CVdata(json.loads(fp.readline()),json.loads(fp.readline()),json.loads(fp.readline()),json.loads(fp.readline()),json.loads(fp.readline()),json.loads(fp.readline()),json.loads(fp.readline()),json.loads(fp.readline()),json.loads(fp.readline()),json.loads(fp.readline()),json.loads(fp.readline()),json.loads(fp.readline()),json.loads(fp.readline()),json.loads(fp.readline()),json.loads(fp.readline()),json.loads(fp.readline()),json.loads(fp.readline()),json.loads(fp.readline()),json.loads(fp.readline()),json.loads(fp.readline()))
        fp.close()
        return obj

    # Tool per RN classificazione
    def exportArrayParametersClass(self): #esporta l'array di features per classificazione
        features=list()
        if(self.nome in self.map): #codice utente
            features.append(self.map[self.nome]) #ggiungi se già esistente
        else: #oppure aggiorna la mappa e aggiungi se nuovo
            l=len(self.map.keys())
            self.map[self.nome]=l
            fp=open('./RNdata/indexMap.text','w')
            fp.write(json.dumps(self.map))
            fp.close()
            features.append(l)
        for j in range(len(self.skills)): #aggiungi l'array di skills possedute convertito da bool a int 0-1
            features.append(self.skills[j]*1)
        features.append(self.punteggioExtracurriculari) #aggiungi il punteggio per attività extra
        features.append(self.punteggioEsperienze) #aggiungi punteggio per esperienze passate
        if (self.titoloDiStudio=='nessuna'):
                tmp=0
        elif (self.titoloDiStudio=='diploma'):
                tmp=1
        elif (self.titoloDiStudio=='laurea'):
                tmp=2
        elif (self.titoloDiStudio=='magistrale'):
                tmp=3
        features.append(tmp) #aggiungi una variabile ordinaria per il titolo di studio
        features.append(self.classe) #appendi la classe se già disponibile
        return features
    def saveParametersClass(self): #aggiunge una nuova riga al database classificazione
        row_=self.exportArrayParametersClass() #esporta l'array per la RN classe
        #print(self.nome)
        #print(row_)
        tc=pd.read_csv('./RNdata/tableClassificatore.csv', index_col=0) #importa il dataset della rete neurale classe
        #crea un'oggetto dataframe pandas con la nuova riga
        row=pd.DataFrame.from_records(data=[row_], columns=['id','programmatore','backend','frontend','fullstack','seo','security','stampa3d','ia','datamining','design','video-audio','mobile','web','comunicazione','hr','management','team','extrascore','oldscore','formazione','classe'])
        #concatena i due dataframe, aggiornando se già presente sulla base della key utente e ripristina gli indici (aggiungi la riga o aggiornala)
        tc=pd.concat([tc,row]).drop_duplicates(['id'], keep='last').reset_index(drop=True)
        tc.to_csv('./RNdata/tableClassificatore.csv', header=True) #salva il database aggiornato
    def classifica(self, ft=[]): #classifica sulla base del vettore di features classificatore
        if ft==[]:
            ft=np.reshape(np.array(self.exportArrayParametersClass())[1:-1], (1,20))
        #print(ft.shape)
        classe=self.RNclass.predici(ft) #predici la classe
        self.classe=classe
        if(classe=='amministrativi'): #sposta il cv nella cartella adatta
            os.replace('./cv/new/'+self.nome, './cv/amministrativi/'+self.nome)
        elif(classe=='frontend'):
            os.replace('./cv/new/'+self.nome, './cv/frontend/'+self.nome)
        elif(classe=='backend'):
            os.replace('./cv/new/'+self.nome, './cv/backend/'+self.nome)
        elif(classe=='fullstack'):
            os.replace('./cv/new/'+self.nome, './cv/fullstack/'+self.nome)
        elif(classe=='grafici'):
            os.replace('./cv/new/'+self.nome, './cv/grafici/'+self.nome)
        elif(classe=='scartati'):
            os.replace('./cv/new/'+self.nome, './cv/scartati/'+self.nome)
        self.save('./cv/report/'+self.nome) #salva l'oggetto corrente con la classe aggiornata
        self.saveParametersClass() #aggiorna il database

    # Tool per RN score
    def exportArrayParametersScore(self, req_plen, req_fram, req_soft, Vscore=0):
        features=list()
        if(self.nome in self.map): #codice utente
            features.append(self.map[self.nome]) #ggiungi se già esistente
        else: #oppure aggiorna la mappa e aggiungi se nuovo
            l=len(self.map.keys())
            self.map[self.nome]=l
            fp=open('./RNdata/indexMap.text','w')
            fp.write(json.dumps(self.map))
            fp.close()
            features.append(l)
        # Conosce l'italiano
        if self.lingue[3]==True:
            features.append(1)
        else:
            features.append(0)
        # Punteggio sulle lingue straniere
        score=0
        for j in range(len(self.lingue)):
            if self.lingue[j]==True:
                score+=1
        features.append(score/len(self.lingue))
        # Ordinale del titolo di studio
        if (self.titoloDiStudio=='nessuna'):
                tmp=0
        elif (self.titoloDiStudio=='diploma'):
                tmp=1
        elif (self.titoloDiStudio=='laurea'):
                tmp=2
        elif (self.titoloDiStudio=='magistrale'):
                tmp=3
        features.append(tmp) #aggiungi una variabile ordinaria per il titolo di studio
        # Numero di match con le richieste
        score=0
        tmp=self.linguaggi and req_plen
        for j in range(len(tmp)):
            if tmp[j]==True:
                score+=1
        features.append(1 if np.count_nonzero(req_plen)==0 else score/np.count_nonzero(req_plen))
        score=0
        tmp=self.framework and req_fram
        for j in range(len(tmp)):
            if tmp[j]==True:
                score+=1
        features.append(1 if np.count_nonzero(req_fram)==0 else score/np.count_nonzero(req_fram))
        score=0
        tmp=self.software and req_soft
        for j in range(len(tmp)):
            if tmp[j]==True:
                score+=1
        features.append(1 if np.count_nonzero(req_soft)==0 else score/np.count_nonzero(req_soft))
        features.append(self.punteggioExtracurriculari) #aggiungi il punteggio per attività extra
        features.append(self.punteggioEsperienze) #aggiungi punteggio per esperienze passate
        cat_=['amministrativi','frontend','backend','fullstack','grafici','scartati']
        features.append(cat_.index(self.classe))
        features.append(Vscore)
        return features
    def saveParametersScore(self, row):
        #print(self.nome)
        #print(row_)
        tc=pd.read_csv('./RNdata/tableScore.csv', index_col=0) #importa il dataset della rete neurale score
        row=pd.DataFrame.from_records(data=[row], columns=['id','italiano','lingue','formazione','linguaggi','framework','software','extrascore','oldscore','categoria','score'])
        tc=pd.concat([tc,row]).drop_duplicates(keep='last').reset_index(drop=True)
        tc.to_csv('./RNdata/tableScore.csv', header=True) #salva il database aggiornato
    def valuta(self, req_plen, req_fram, req_soft, ft=[]):
        if ft==[]:
            ft=np.reshape(np.array(self.exportArrayParametersScore(req_plen, req_fram, req_soft))[1:-1], (1,9))
        punteggio=self.RNscore.predici(ft)
        return punteggio


if __name__=='__main__':
    path=r'C:\Users\Simone\Desktop\Tirocinio\testPy\HR_IA\cv\new' #path dei cv
    #TEST DI LETTURA DA PDF
    pdfA=PDFAnalyzer()
    pdfTest=pdfA.listPDF(path)[0]
    data=pdfA.analyze(pdfTest, report='stdout')
    #if(data!=None): # TODO: gestione con eccezioni
    data.classifica() #classifica, sposta, salva report e parametri

    #text=pdfA.PDFtoText('./cv/new/AntonioMarianiCv.pdf');text
#id, programmatore, backend, frontend, fullstack, seo, security, stampa3d, ia, datamining, design, video-audio, mobile, web, comunicazione, hr, management, team, extrascore, oldscore, formazione
    #TEST DI IMPORT DA SALVATAGGIO PRECEDENTE
    #pdfTest='C:\\Users\\Simone\\Desktop\\Tirocinio\\testPy\\cv\\AntonioMarianiCv.pdf'
    #pdf=CVdata.load(pdfTest)
    #pdf.report()
