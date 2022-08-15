# so_project

## Questo progetto riguarda la riproduzione di un sistema blockchain minimale creato in ambiente Unix con linguaggio C. Versione 24

Il progetto è composto da quattro componenti, utente, nodo, libro mastro e master.

Il master fa da sistema universo dell'esecuzione della simulazione, crea e lancia i processi utente e nodo, crea il libro mastro e si occupa della stampa delle statistiche durante la simulaziione.

Gli utenti generano delle transazioni randomiche che hanno come destinatario altri utenti scelti casualmente. Il sistema è di per sè ancora centralizzato, questo per intendere che la comunicazione fra utenti non è peer to peer ma è mediata da nodi che processano le transazioni.

Questi processi nodo mediatori raccolgono transazioni da processare ed inviare, i nodi a livello implementativo hanno tre funzioni: una è quella di salvare all'interno di una transaction pool unica per nodo le transazioni che vengono inviate, la seconda è quella di far percepire le transazioni agli altri utenti e l'ultima è di trasferire, una volta che un dato blocco della transaction pool è piena, le transazioni in una struttura condivisa chiamata libro mastro

La comunicazione fra utenti e nodi avviene tramite code di messaggi per avere un modo asincrono di gestione messaggi e quindi di permettere invio e ricezioni senza che si debba rimanere in attesa di messaggi durante l'esecuzione per quanto riguarda i nodi , questo è permeso attraverso una flag IPC_NOWAIT nel retrieve della coda di messaggi e ci permette di massimizzare il grado di concorrenza in quanto nessun nodo rimane in attesa di una ricezione di un messaggio e può continuare il proprio ciclo di vita.
Inoltre per quanto riguarda i processi utente la scelta del NO_WAIT ci permettere di depositare le transazioni nella coda e continuare la propria esecuzione, controllando semplicemente che in una coda utente venga data conferma della transazione percepita dall'utente destinatario.

Mentre per quanto riguarda la relazione fra libro mastro e nodi, in cui i nodi si occupano di spostare a transaction pool piena le transazioni su shared memory, ho dovuto regolare l'accesso e la seguente scrittura sul libro mastro attraverso un semaforo per permettere il ritiro da altre strutture come gli utenti che calcolano il proprio budget di informazioni consistenti co lo stato attuale del libro mastro. il semaforo che regola la scrittura è un mutex in quanto il salvataggio dei blocchi in questa shared memory viene fatto in una struttura indicizzata, per cui per evitare sovrascritture di blocchi allo stesso indice la scrittura è mediata da questo mutex.

Per quanto riguarda la simulazione nella sua totalità, abbiamo tutto l'insieme dello strutture condivise fra diversi nodi, utenti e master in un oggetto ipcs che fa da state manager dell'applicazione, questo state manager gestisce l'invio di messaggi attraverso code, accessi, incrementi e decrementi di semafori e gestisce anche un valore 0-1 in shared memory chiamato is_simulation_running, questo valore è ciò che viene modificato in caso si verifichi una delle condizioni di terminazione della simulazione e la sua modifica a 0 determina l'effettiva stampa finale riassuntiva delle statistiche, il successivo detachment e liberazione delle strutture condivise e l'uscita del processo master, l'ultimo ad uscire dalla simulazione.

Attraverso un alarm ogni secondo nel master, viene attivato un handler di SIG_ALARM che gestisce la stampa delle statistiche riprese dal libro mastro, dagli utenti e dai nodi e fa un controllo sulle condizioni di terminazioni, in caso si sia verificata una o più condizioni per garantire la lettura con stato consistente da parte di altri elementi viene fatto accesso ad una sezione critica mediata da un mutex in cui si modifica il valore di is_simulation_running a 0.

Ed infine avviene la stampa riassuntiva finale.



