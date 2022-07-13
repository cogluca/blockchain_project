# so_project

## Questo progetto riguarda la riproduzione di un sistema blockchain minimale creato in ambiente Unix con linguaggio C. Versione 24

Il progetto è composto da quattro componenti, utente, nodo, libro mastro e master.

Il master fa da sistema universo dell'esecuzione della simulazione, crea e lancia i processi utente e nodo, crea il libro mastro e si occupa della stampa delle statistiche durante la simulaziione.

Gli utenti generano delle transazioni randomiche che hanno come destinatario altri utenti scelti casualmente. Il sistema è di per sè ancora centralizzato, questo per intendere che la comunicazione fra utenti non è peer to peer ma è mediata da nodi che processano le transazioni.

Questi processi nodo mediatori raccolgono transazioni da processare ed inviare, i nodi a livello implementativo hanno due funzioni: una è quella di salvare all'interno di una transaction pool unica per nodo le transazioni che vengono inviate e l'altra è quella di far percepire le transazioni agli altri utenti.

La comunicazione fra utenti e nodi avviene tramite code di messaggi (perchè ?) per avere un modo asincrono di gestione messaggi e quindi di permettere invio e ricezioni senza che si debba rimanere in attesa di messaggi durante l'esecuzione, questo è permeso attraverso una flag IPC_NOWAIT e ci permette di massimizzare il grado di concorrenza.

Mentre per quanto riguarda la relazione fra libro mastro e nodi, in cui i nodi si occupano di spostare a transaction pool piena le transazioni su shared memory, ho affrontato il (cazzo ho fatto qui) ho dovuto regolare l'accesso e la seguente scrittura sul libro mastro in quanto il salvataggio dei blocchi in questa shared memory viene fatto in una struttura indicizzata, per cui per evitare sovrascritture allo stesso indice la scrittura è mediata da un semaforo.

- Parlare della relazione fra stampa statistiche del master e il libro mastro
- Parlare del rimbalzo all'utente per confermargli la transazione come inviata
- Parlare del calcolo interno negli utenti del proprio saldo anche secondo gli invii effettuati