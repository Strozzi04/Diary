// Diario digitale - versione a console.
// La logica e i file sono gestiti da diario_core.cpp; qui c'e' solo il menu.
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

#include "diario_core.h"

using namespace std;
using namespace diario;

// ************ INPUT ************

string chiedi_testo(const string& domanda) {
    cout << domanda;
    string risposta;
    if (!getline(cin, risposta)) exit(0);   // input chiuso (es. Ctrl+Z)
    return pulisci(risposta);
}

int chiedi_intero(const string& domanda, int minimo, int massimo) {
    while (true) {
        string testo = chiedi_testo(domanda);
        try {
            size_t letti = 0;
            int valore = stoi(testo, &letti);
            if (letti == testo.size() && valore >= minimo && valore <= massimo) return valore;
        } catch (...) {
        }
        cout << "Valore non valido, inserisci un numero da " << minimo << " a " << massimo << ".\n";
    }
}

double chiedi_euro(const string& domanda) {
    while (true) {
        double valore;
        if (leggi_numero(chiedi_testo(domanda), valore) && valore >= 0) return valore;
        cout << "Importo non valido (esempio: 12.50).\n";
    }
}

bool chiedi_si_no(const string& domanda) {
    while (true) {
        string r = chiedi_testo(domanda + " (s/n): ");
        if (r == "s" || r == "S" || r == "si" || r == "1") return true;
        if (r == "n" || r == "N" || r == "no" || r == "0") return false;
    }
}

// ************ FUNZIONI DEL MENU ************

void setta_diario() {
    vector<string> nomi = elenco_routine();
    if (nomi.empty()) {
        cout << "Non hai ancora nessuna routine.\n";
    } else {
        cout << "Le tue routine attuali:\n";
        for (const string& n : nomi) cout << " - " << n << "\n";
    }
    int n = chiedi_intero("Quante routine vuoi aggiungere? ", 0, 100);
    for (int i = 0; i < n; i++) {
        string errore = aggiungi_routine(
            chiedi_testo("Nome della routine " + to_string(i + 1) + " (es. leggere, allenarsi): "));
        if (!errore.empty()) {
            cout << errore << "\n";
            i--;
        }
    }
    if (n > 0) cout << "Routine salvate.\n";
}

void registra_giorno() {
    VoceGiorno voce;
    voce.data = oggi();
    cout << "\n--- Diario del " << data_testo(voce.data) << " ---\n";

    if (giorno_registrato(voce.data) &&
        !chiedi_si_no("Hai gia' registrato il diario di oggi. Vuoi sovrascriverlo?")) {
        return;
    }

    vector<string> nomi = elenco_routine();
    if (nomi.empty()) {
        cout << "Non hai routine: puoi aggiungerle con l'opzione 1.\n";
    }
    for (const string& nome : nomi) {
        if (chiedi_si_no("Hai fatto \"" + nome + "\" oggi?")) voce.routine_fatte.push_back(nome);
    }

    voce.racconto = chiedi_testo("Racconta la tua giornata: ");
    voce.tempo_atmosferico = chiedi_testo("Com'era il tempo oggi? ");
    voce.film_serie_libro = chiedi_testo("Racconta una serie tv, un film o un libro: ");
    voce.voto = chiedi_intero("Dai un voto alla giornata (1-10): ", 1, 10);
    voce.spesa = chiedi_euro("Quanto hai speso oggi? ");

    salva_giorno(voce);
    cout << "Diario del giorno salvato.\n";
}

void registra_mese() {
    Data precedente = mese_precedente(oggi());
    cout << "\nOra registrerai i preferiti di " << nome_mese(precedente.mese) << " "
         << precedente.anno << ".\n";

    PreferitiMese p;
    p.film = chiedi_testo("Film: ");
    p.serie_tv = chiedi_testo("Serie tv: ");
    p.gioco = chiedi_testo("Gioco: ");
    p.libro = chiedi_testo("Libro: ");
    p.canzone = chiedi_testo("Canzone: ");
    p.oggetto = chiedi_testo("Oggetto: ");
    p.video = chiedi_testo("Video: ");
    p.miglior_acquisto = chiedi_testo("Miglior acquisto: ");
    p.miglior_regalo = chiedi_testo("Miglior regalo: ");
    p.giorno_preferito = chiedi_testo("Racconta il giorno preferito: ");
    p.racconto = chiedi_testo("Racconta il mese: ");

    salva_mese(precedente.mese, precedente.anno, p);
    cout << "Preferiti del mese salvati.\n";
}

void registra_anno() {
    int anno_corrente = oggi().anno;
    int anno_precedente = anno_corrente - 1;
    cout << "\nRegistrazione anno " << anno_corrente << "\n";

    prepara_anno_routine(anno_corrente);
    cout << "Aggiunto l'anno " << anno_corrente << " alle routine.\n";

    // 1) obiettivi dell'anno precedente
    vector<Obiettivo> vecchi = leggi_obiettivi(anno_precedente);
    if (vecchi.empty()) {
        cout << "Nessun obiettivo dell'anno precedente trovato.\n";
    } else {
        cout << "Obiettivi del " << anno_precedente << ":\n";
        for (Obiettivo& o : vecchi) {
            o.stato = chiedi_si_no(o.testo + " - raggiunto?") ? RAGGIUNTO : NON_RAGGIUNTO;
        }
        salva_obiettivi(anno_precedente, vecchi);
    }

    // 2) nuovi obiettivi
    vector<Obiettivo> nuovi = leggi_obiettivi(anno_corrente);
    int n = chiedi_intero("Quanti obiettivi vuoi registrare per il " + to_string(anno_corrente) + "? ",
                          0, 100);
    for (int i = 0; i < n; i++) {
        Obiettivo o;
        o.testo = chiedi_testo("Obiettivo " + to_string(i + 1) + ": ");
        if (o.testo.empty()) { i--; continue; }
        nuovi.push_back(o);
    }
    salva_obiettivi(anno_corrente, nuovi);

    // 3) preferiti dell'anno precedente
    cout << "Ora registrerai i preferiti del " << anno_precedente << ".\n";
    PreferitiAnno p;
    for (int i = 0; i < 3; i++) p.film[i] = chiedi_testo("Film " + to_string(i + 1) + ": ");
    for (int i = 0; i < 3; i++) p.serie[i] = chiedi_testo("Serie tv " + to_string(i + 1) + ": ");
    for (int i = 0; i < 3; i++) p.giochi[i] = chiedi_testo("Gioco " + to_string(i + 1) + ": ");
    for (int i = 0; i < 3; i++) p.libri[i] = chiedi_testo("Libro " + to_string(i + 1) + ": ");
    p.giorno_bello = chiedi_testo("Qual e' stato il giorno piu' bello dell'anno? ");
    p.racconto = chiedi_testo("Racconta com'e' andato l'anno: ");
    salva_preferiti_anno(anno_precedente, p);

    cout << "Registrazione anno completata.\n";
}

void mostra_statistiche_routine() {
    vector<string> nomi = elenco_routine();
    if (nomi.empty()) {
        cout << "Non hai ancora nessuna routine.\n";
        return;
    }
    cout << "Le tue routine:\n";
    for (size_t i = 0; i < nomi.size(); i++) cout << " " << i + 1 << ") " << nomi[i] << "\n";
    string nome = nomi[chiedi_intero("Quale routine vuoi controllare? ", 1, (int)nomi.size()) - 1];

    cout << "1) Quante volte l'hai fatta in un mese\n";
    cout << "2) Quante volte l'hai fatta in un anno\n";
    int scelta = chiedi_intero("Scelta: ", 1, 2);
    int anno = chiedi_intero("Anno (es. " + to_string(oggi().anno) + "): ", 1900, 3000);

    if (scelta == 1) {
        int mese = chiedi_intero("Mese (1-12): ", 1, 12);
        int volte = conteggio_routine_mese(nome, mese, anno);
        if (volte < 0) cout << "Nessun dato per quel mese.\n";
        else cout << "Hai fatto \"" << nome << "\" " << volte << " volte a "
                  << nome_mese(mese) << " " << anno << ".\n";
    } else {
        int volte = conteggio_routine_anno(nome, anno);
        if (volte < 0) cout << "Nessun dato per quell'anno.\n";
        else cout << "Nel " << anno << " hai fatto \"" << nome << "\" " << volte << " volte.\n";
    }
}

void mostra_obiettivi_anno() {
    int anno = oggi().anno;
    vector<Obiettivo> obiettivi = leggi_obiettivi(anno);
    if (obiettivi.empty()) {
        cout << "Non ci sono obiettivi registrati per quest'anno.\n";
        return;
    }
    cout << "\n--- OBIETTIVI " << anno << " ---\n";
    for (const Obiettivo& o : obiettivi) {
        cout << " - " << o.testo;
        if (o.stato == RAGGIUNTO) cout << " (raggiunto)";
        if (o.stato == NON_RAGGIUNTO) cout << " (non raggiunto)";
        cout << "\n";
    }
}

void mostra_spese() {
    Data d = oggi();
    cout << "\n1) Spese del mese corrente\n";
    cout << "2) Spese di un mese specifico\n";
    cout << "3) Spese dell'anno corrente\n";
    cout << "4) Spese di un anno specifico\n";
    cout << "5) Media mensile dell'anno corrente\n";
    cout << "6) Media annuale (media degli anni registrati)\n";
    int scelta = chiedi_intero("Scelta: ", 1, 6);

    if (scelta == 1) {
        cout << "Hai speso " << formatta_euro(spesa_mese(d.mese, d.anno)) << " euro questo mese.\n";
    } else if (scelta == 2) {
        int mese = chiedi_intero("Mese (1-12): ", 1, 12);
        int anno = chiedi_intero("Anno: ", 1900, 3000);
        cout << "Hai speso " << formatta_euro(spesa_mese(mese, anno)) << " euro a "
             << nome_mese(mese) << " " << anno << ".\n";
    } else if (scelta == 3) {
        cout << "Hai speso " << formatta_euro(spesa_anno(d.anno)) << " euro quest'anno.\n";
    } else if (scelta == 4) {
        int anno = chiedi_intero("Anno: ", 1900, 3000);
        cout << "Hai speso " << formatta_euro(spesa_anno(anno)) << " euro nel " << anno << ".\n";
    } else if (scelta == 5) {
        int mesi = 0;
        double media = media_mensile(d.anno, &mesi);
        if (mesi == 0) cout << "Nessuna spesa registrata quest'anno.\n";
        else cout << "Media mensile nel " << d.anno << ": " << formatta_euro(media)
                  << " euro (su " << mesi << (mesi == 1 ? " mese registrato).\n" : " mesi registrati).\n");
    } else {
        int anni = 0;
        double media = media_annuale(&anni);
        if (anni == 0) cout << "Non ci sono dati annuali registrati.\n";
        else cout << "Media annuale: " << formatta_euro(media) << " euro (su " << anni
                  << (anni == 1 ? " anno registrato).\n" : " anni registrati).\n");
    }
}

// ************ MAIN ************

int main() {
#ifdef _WIN32
    // Per mostrare correttamente le lettere accentate nella console di Windows
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    while (true) {
        Data d = oggi();
        cout << "\n===== DIARIO - " << data_testo(d) << " =====\n";
        cout << "1) Imposta il diario / aggiungi routine\n";
        cout << "2) Registra il diario del giorno\n";
        cout << "3) Registra i preferiti del mese scorso";
        if (d.giorno == 1) cout << "  <-- oggi e' il primo del mese!";
        cout << "\n";
        cout << "4) Registra l'anno (obiettivi e preferiti)";
        if (d.giorno == 1 && d.mese == 1) cout << "  <-- oggi e' il primo dell'anno!";
        cout << "\n";
        cout << "5) Statistiche routine\n";
        cout << "6) Obiettivi dell'anno corrente\n";
        cout << "7) Spese registrate\n";
        cout << "0) Esci\n";

        switch (chiedi_intero("Scelta: ", 0, 7)) {
            case 1: setta_diario(); break;
            case 2: registra_giorno(); break;
            case 3: registra_mese(); break;
            case 4: registra_anno(); break;
            case 5: mostra_statistiche_routine(); break;
            case 6: mostra_obiettivi_anno(); break;
            case 7: mostra_spese(); break;
            case 0: return 0;
        }
    }
}
