# Obiettivo

Obiettivo del progetto è eseguire una corretta taratura di una cella di carico a doppia flessione, in modo da poterla utilizzare per leggere pesi e studiarne il funzionamento. Tra le funzioni da implementare si trovano:

- Taratura ed equilibratura della cella di carico.
- Lettura di pesi noti e costruzione della retta di taratura.
- Comparazione di valori letti con diversi guadagni.
- Lettura di pesi non noti e conversione della stessa in grammi.

# Strumenti utilizzati

Tra gli strumenti utilizzati per il progetto si distinguono:

- Componenti hardware
  - Arduino
  - Amplificatore HX711
  - Cella di carico
- Componenti software
  - LabVIEW
  - Sketch Arduino

# Descrizione del progetto

Il progetto sfrutta la taratura di una cella di carico per analizzare nel complesso il funzionamento del trasduttore. In particolare, tramite una comunicazione tra Arduino e LabVIEW, permette di eseguire tre funzioni principali:

- Lettura dei dati: acquisizione di dati grezzi dalla cella, calcolo di parametri e ingegnerizzazione del dato.
- Comparazione dei guadagni: lettura di dati grezzi dalla cella variando il guadagno dell&#39;amplificatore.
- Equilibratura e taratura: taratura della cella di carico con relativa impostazione dei parametri della retta di taratura: pendenza e intercetta.

In aggiunta alle funzioni principali, il programma prevede:

- Salvataggio dei dati acquisiti su file esterni, utilizzabili con software di calcolo.
- Salvataggio su EEPROM Arduino dei parametri principali di taratura, in modo da poterli utilizzare successivamente senza bisogno di nuova taratura.

# Schema circuitale

Arduino e l&#39;amplificatore HX711 sono collegati tramite i pin D6 e D7 di Arduino. In particolare,

- 5V  ==> VCC
- D6  ==> SCK
- D7  ==> DT
- GND ==> GND

L&#39;amplificatore comunica con la cella di carico a singolo punto con i seguenti collegamenti:

- E+  ==> Cavo rosso
- E-  ==> Cavo nero
- A-  ==> Cavo bianco
- A+  ==> Cavo verde

Per la mini-cella di carico sono necessari due resistori da 1kΩ per completare il ponte di Wheatstone:

- E+  ==> Cavo bianco
- E-  ==> Cavo nero
- A-  ==> Completamento del ponte di Wheatstone
- A+  ==> Cavo rosso

# Arduino

Lo sketch Arduino è stato progettato per l&#39;acquisizione dei dati dalla cella di carico; per fare ciò non è stata utilizzata la libreria dell&#39;amplificatore HX711 disponibile per questo tipo di applicazioni, ma si è proceduto all&#39;implementazione delle funzioni direttamente nello sketch.

Dopo la definizione dei PIN di collegamento dell&#39;amplificatore e il settaggio della velocità di comunicazione della porta COM, si è proceduto all&#39;implementazione del menu di funzionamento, raggiungibile mediante sequenza numerica scritta sul buffer della porta COM.

In particolare, la sequenza numerica prevede la scelta del tipo di guadagno e successivamente la scelta della funzione da utilizzare.

Nella funzione _loop_ è stata implementata la verifica della presenza di nuovi elementi sul buffer della porta COM: nel caso in cui ci siano dati, viene settato basso il PIN di clock (PD\_SCK) dell&#39;amplificatore HX711; successivamente viene effettuato il _parse_dei valori presenti sul buffer, mediante i quali si settano GAIN e la funzione.

Il valore di GAIN può acquisire i valori:

- 32
- 64
- 128

Le funzioni implementate sono:

- Lettura (_reading_)
  - La funzione di lettura inizializza un array vuoto nel quale, dopo aver verificato che i dati siano pronti alla lettura mediante lo stato _LOW_ del pin _DOUT,_ si vanno a inserire i dati provenienti dall&#39;amplificatore mediante la funzione _shiftin,_ la quale sposta un byte di dati un bit alla volta, in questo caso con politica _ **M** __ost__ **S** __ignificant__ **B** __it__ **First.** _ L&#39;operazione è ripetuta fino al riempimento delle tre celle dell&#39;array.

Si ricompone infine il numero, applicando a tutti i componenti l&#39;operatore _\&lt;\&lt;_ che effettua lo shift dei bit della quantità desiderata. Al numero ottenuto si sottrae l&#39;offset calcolato mediante equilibratura.

- Confronto dei GAIN (_compare\_gain_)
  - La funzione effettua la comparazione dei GAIN, effettuando dieci misurazioni e restituendo la media di queste per ognuno dei GAIN.
- Calibrazione (_calibration_)
  - La funzione di calibrazione in Arduino si occupa di effettuare l&#39;equilibratura, acquisendo il primo valore e usandolo come offset per le successive. Per ottimizzare l&#39;acquisizione dell&#39;offset si è pensato di effettuare una verifica sul valore restituito dalla differenza tra offset e misura, verificando che questa non sia maggiore di 200 in valore assoluto.
- Recupero dei dati dalla EEPROM (_get\_parameters_):
  - La funzione prevede di acquisire dai registri EEPROM di Arduino tutti i valori di taratura scritti in precedenza, partendo dal primo valore e settando il registro successivo come la somma del registro precedente più la lunghezza del valore appena letto.
- Salvataggio dei valori di taratura nei registri EEPROM (_set\_parameters_):
  - All&#39;interno di questa funzione è presente una nuova sezione di acquisizione dati dalla porta seriale, il cui protocollo prevede di acquisire per primo il valore del GAIN, poi il valore di _slope_ e _intercept_ della retta individuata mediante LabVIEW.
- Equilibratura iniziale (_set\_all\_offset_)
  - Questa funzione, a differenza di _calibration,_permette di fornire all&#39;avvio dell&#39;applicativo LabVIEW i valori di offset aggiornati per effettuare le misure, cosi da sfruttare le tarature precedentemente realizzate e salvate in EEPROM.

# LabVIEW

## Front panel

L&#39;interfaccia grafica su LabVIEW è stata studiata per risultare di facile utilizzo e ben organizzata. In particolare, si compone di tre tab principali:

- Home
- Execute calibration
- Nerd zone

In Home si possono impostare i principali parametri necessari per il corretto funzionamento del programma come numero di cicli, funzione da utilizzare e guadagno.

- Ring per scegliere la funzione da eseguire, mappata secondo la seguente regola:
  - 0 – Read Data
  - 1 – Compare Different Gains
  - 2 – Execute Calibration
- Ring per scegliere il guadagno, mappato secondo la seguente regola:
  - 0 – guadagno a 32
  - 1 – guadagno a 64
  - 2 – guadagno a 128

Il tab mostra informazioni aggiuntive di configurazione iniziale – impostazione degli offset su Arduino e restituzione dei parametri per le conversioni – e altri riferimenti utili in fase di lettura dei dati. È inoltre possibile selezionare il formato del file di salvataggio dei dati – xls, tsv o txt.

Il tab Execute calibration mostra:

- Il rumore calcolato in base al guadagno impostato.
- L&#39;array degli errori assoluti.
- L&#39;array degli errori relativi.
- Un checkbox che mostra il corretto aggiornamento dei dati nell&#39;EEPROM di Arduino.
- I dati dei pesi noti letti in fase di taratura.
- Un grafico che mostra la retta di taratura, basata sulle letture dei pesi noti effettuate.

Il tab Nerd zone mostra informazioni aggiuntive come gli offset letti in fase di collegamento con Arduino, i vari dati ricevuti durante l&#39;intera esecuzione e un array di debug contenente i comandi inviati ad Arduino, insieme al comando di stop.

## Block Diagram

Il diagramma a blocchi è composto da vari elementi e sub vi. Quando il VI viene eseguito, appare un popup che chiede all&#39;utente di selezionare la porta COM cui è collegato Arduino. Una volta stabilita la connessione – con baud rate 115200 – l&#39;esecuzione entra in una sequenza temporale formata da:

- Inizializzazione delle variabili utilizzate e tempo di attesa di tre secondi.
- Impostazione su Arduino dei tre offset – uno per ogni gain – con conseguente accensione del relativo checkbox nel front panel in caso di successo.
- Recupero delle variabili relative ai tre guadagni dall&#39;EEPROM di Arduino con conseguente accensione del relativo checkbox nel front panel in caso di successo.
- Ingresso nel loop di esecuzione principale.

Il loop principale consente l&#39;esecuzione dell&#39;intero programma. Inizialmente è presente un loop secondario in cui il programma cicla indefinitamente – con attese di 500 ms per non intasare il buffer – fino alla pressione del pulsante Start operation o del pulsante di stop. Il loop secondario imposta quindi funzione e guadagno scelti.

Funzione e guadagno vengono fusi in una stringa correttamente formattata da inviare ad Arduino; contestualmente, viene aggiornato l&#39;array di debug e impostato il numero di cicli da eseguire. Il sub VI di comunicazione con Arduino restituisce un array contenente i dati ricevuti dal microcontrollore. In base alla funzione scelta, è possibile entrare in un case:

- Lettura dei dati: i dati letti vengono utilizzati per calcolare media, deviazione standard e varianza. Grazie a pendenza e intercetta della retta – ottenuti in precedenza e selezionati in base al guadagno impostato – è possibile convertire la media delle letture effettuate in grammi. I vari dati vengono infine salvati nell&#39;array principale, mostrati a video nel front panel e salvati in un file.
- Comparazione dei gain: richiama la relativa funzione su Arduino, che restituisce le medie delle letture effettuate con i vari guadagni. Le tre letture vengono mostrate nel front panel e salvate in un file di log.
- Taratura: inizialmente viene richiamato il sub VI relativo alla taratura, che restituisce vari valori:
  - Offset impostato e nuovo valore letto.
  - Array contenente i pesi noti impostati.
  - Array contenente le misure effettuate con i pesi noti impostati.
  - Array degli errori assoluti.
  - Array degli errori relativi.
  - Rumore ottenuto sulle letture.

Gli array contenenti pesi noti e relative misure sono utilizzati per creare la retta di taratura mostrata nel front panel e per calcolare pendenza e intercetta. Questi ultimi due valori, insieme al rumore, vengono salvati nelle relative variabili in base al guadagno impostato. Pendenza e intercetta sono inoltre salvate nell&#39;EEPROM di Arduino grazie al relativo sub VI, accendendo il relativo checkbox nel front panel in caso di successo. Tutti i dati calcolati sono infine salvati in un file di log.

Il programma termina con la chiusura della connessione con Arduino e un Simple Error Handler per la cattura di eventuali errori ottenuti nelle varie fasi.

## Sub VI

Seguendo l&#39;ordine di esecuzione del programma, si incontrano vari sub VI, descritti di seguito.

### Sub\_vi\_arduino\_com\_port\_popup

Semplice sub VI, mostrato come finestra di dialogo, che permette di impostare la porta COM cui è collegato Arduino.

### Sub\_vi\_arduino\_communication

Sub VI che manda una stringa correttamente formattata ad Arduino per poi attendere una risposta, che viene raccolta, elaborata per trasformarla in array numerico e restituita. Inizialmente utilizzato per impostare i tre offset su Arduino, viene poi richiamato nella fase principale di comunicazione.

### Sub\_vi\_get\_linear\_parameters

Sub VI necessario per l&#39;acquisizione dei parametri di taratura dall&#39;EEPROM di Arduino. I parametri restituiti da Arduino sono inseriti in una stringa, che deve essere modificata – convertendo il punto in virgola per i numeri double – e trasformata in array, usando il tabulatore come delimitatore. L&#39;array viene poi scompattato nei sei parametri richiesti.

### Sub\_vi\_prompt\_calibration\_weigths

Sub VI utilizzato per creare i messaggi di dialogo necessari a inserire il numero di pesi noti e i relativi pesi in grammi.

### Sub\_vi\_calibration

Sub VI chiamato in fase di taratura; permette di scegliere quanti pesi noti utilizzare per la creazione dei parametri e della retta di taratura e di segnalare il peso effettivo in grammi, tramite il sub VI descritto in precedenza. Per ogni peso noto vengono effettuate dieci letture e restituita la media. Pesi noti e medie sono collezionati in due array. Sono inoltre qui calcolati i due vettori di errori assoluti ed errori relativi. Per il calcolo del rumore è utilizzato un ulteriore sub VI.

### Sub\_vi\_noise

Sub VI con cinque input: due pesi noti (x2 e x1), due misure (y2 e y1) e deviazione standard (σ). L&#39;output conferito riguarda il rumore, calcolato come segue.

### Sub\_vi\_convert\_measures

Sub VI che converte i valori di media, deviazione standard e varianza, ottenuti in fase di lettura, in grammi, sfruttando la pendenza e l&#39;offset calcolati.

### Sub\_vi\_set\_linear\_parameters

Sub VI necessario per salvare pendenza e intercetta relative al guadagno impostato nell&#39;EEPROM di Arduino. In caso di successo, il relativo checkbox presente nel tab Execute calibration viene attivato.

### Sub\_vi\_saving\_string\_read e Sub\_vi\_saving\_string\_calibration

Sub VI utilizzati per creare la stringa correttamente formattata da salvare nei relativi file di log.

### Sub\_vi\_write\_file

Il sub VI crea il nome del file da scrivere – diverso in base alla funzione utilizzata – con indicazione dell&#39;orario di salvataggio per evitare collisioni con file dello stesso nome. La stringa – ottenuta con uno dei due precedenti sub VI – è convertita in spreadsheet string e salvata in file. Infine, viene mostrato un popup che segnala il path relativo utilizzato e il nome del file.
