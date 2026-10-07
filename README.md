📔 Diario Digitale in C++

Diario digitale sviluppato in C++ per tenere traccia della vita quotidiana, delle routine, delle spese e degli obiettivi personali su base giornaliera, mensile e annuale.

Il programma salva automaticamente tutti i dati in file di testo, rendendoli semplici da consultare e modificare.

🚀 Funzionalità
📅 Diario giornaliero

Racconto della giornata

Film, serie TV, libri o giochi

Voto della giornata

Spesa giornaliera

Routine svolte

🔁 Gestione delle routine

Creazione e aggiunta di routine personalizzate

Conteggio automatico delle routine svolte

Visualizzazione delle routine per mese e per anno

📆 Diario mensile

Preferiti del mese (film, serie TV, gioco, libro, canzone, ecc.)

Racconto del mese

Routine mensili

🎯 Gestione annuale

Inserimento degli obiettivi dell’anno

Visualizzazione degli obiettivi durante l’anno

Verifica degli obiettivi raggiunti e non raggiunti

💰 Gestione delle spese

Registrazione delle spese giornaliere

Totale spese mensili e annuali

Media mensile e media annuale delle spese

🖥️ Interfaccia grafica

Oltre alla versione a console c'è ora una versione con finestre (DiarioGUI.exe), divisa in schede:

Giorno: scegli la data, spunta le routine fatte, racconta la giornata, dai un voto e inserisci la spesa. Puoi riaprire e correggere un giorno già scritto.

Routine: quante volte hai fatto ogni routine nel mese e nell'anno scelti; da qui puoi anche aggiungerne di nuove.

Mese: i preferiti di ogni mese (si apre già sul mese scorso).

Anno: obiettivi dell'anno (aggiungi, segna come raggiunto / non raggiunto, elimina) e preferiti dell'anno.

Spese: riepilogo del mese e dell'anno, medie e grafico a barre delle spese mese per mese.

Le due versioni usano gli stessi file: i dati scritti con una si vedono anche con l'altra.

▶️ Avvio del programma

Potete scaricare la cartella completa del progetto e avviare, dalla cartella Diario:

DiarioGUI.exe (versione grafica)

Diario.exe (versione a console)

I file del diario vengono salvati nella stessa cartella del programma.

🔨 Compilazione

Il codice è diviso in tre parti:

diario_core.cpp / diario_core.h: tutta la logica e la gestione dei file

Diario.cpp: menu a console

DiarioGUI.cpp: interfaccia grafica (API Win32, non servono librerie esterne)

Con MinGW (ad esempio quello di Dev-C++) basta eseguire build.bat nella cartella Diario.

📌 Consigli

Si consiglia di iniziare a inizio anno per sfruttare tutte le funzionalità

In alternativa, è possibile iniziare a inizio mese

🛠️ Personalizzazione

Il codice è pensato per essere facilmente modificabile.

Per modifiche particolari o richieste personalizzate potete contattarmi via mail:

📧 andreastrozziero@gmail.com

📝 Note

Questo progetto nasce come diario personale ma può essere utile anche per:

migliorare l’organizzazione personale

monitorare abitudini e spese

fare pratica con il C++ e la gestione dei file

👤 Autore

Andrea Strozziero
