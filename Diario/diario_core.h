// Nucleo del diario: tutta la logica e la gestione dei file.
// Non stampa nulla e non legge da tastiera, cosi' puo' essere usato
// sia dalla versione a console (Diario.cpp) sia da quella grafica (DiarioGUI.cpp).
#ifndef DIARIO_CORE_H
#define DIARIO_CORE_H

#include <string>
#include <vector>

namespace diario {

struct Data {
    int giorno;
    int mese;
    int anno;
};

Data oggi();
Data mese_precedente(Data d);           // giorno = 1, gestisce gennaio -> dicembre
bool data_valida(Data d);
std::string data_testo(Data d);         // "7-10-2026"
std::string nome_mese(int mese);        // "ottobre"

// ************ ROUTINE ************

std::vector<std::string> elenco_routine();
// Restituisce un messaggio di errore, oppure stringa vuota se tutto ok.
std::string aggiungi_routine(const std::string& nome);
// Restituisce -1 se la routine non esiste per quel periodo.
int conteggio_routine_anno(const std::string& nome, int anno);
int conteggio_routine_mese(const std::string& nome, int mese, int anno);
// Aggiunge l'anno alle routine che non lo hanno ancora (senza duplicarlo).
void prepara_anno_routine(int anno);
// Crea il file mensile delle routine se manca e aggiunge le routine mancanti,
// senza azzerare i conteggi gia' presenti.
void prepara_mese_routine(int mese, int anno);

// ************ GIORNO ************

struct VoceGiorno {
    Data data{};
    std::vector<std::string> routine_fatte;
    std::string racconto;
    std::string tempo_atmosferico;
    std::string film_serie_libro;
    int voto = 0;                       // 1 - 10
    double spesa = 0;
};

bool giorno_registrato(Data d);
bool leggi_giorno(Data d, VoceGiorno& voce);
// Salva il giorno. Se era gia' stato registrato, prima annulla i conteggi
// delle routine e la spesa della registrazione precedente (niente doppioni).
void salva_giorno(const VoceGiorno& voce);

// ************ MESE ************

struct PreferitiMese {
    std::string film, serie_tv, gioco, libro, canzone, oggetto, video;
    std::string miglior_acquisto, miglior_regalo, giorno_preferito, racconto;
};

bool leggi_mese(int mese, int anno, PreferitiMese& p);
void salva_mese(int mese, int anno, const PreferitiMese& p);

// ************ ANNO ************

enum StatoObiettivo { IN_CORSO = 0, RAGGIUNTO = 1, NON_RAGGIUNTO = 2 };

struct Obiettivo {
    std::string testo;
    StatoObiettivo stato = IN_CORSO;
};

std::vector<Obiettivo> leggi_obiettivi(int anno);
void salva_obiettivi(int anno, const std::vector<Obiettivo>& obiettivi);

struct PreferitiAnno {
    std::string film[3], serie[3], giochi[3], libri[3];
    std::string giorno_bello, racconto;
};

bool leggi_preferiti_anno(int anno, PreferitiAnno& p);
void salva_preferiti_anno(int anno, const PreferitiAnno& p);

// ************ SPESE ************

double spesa_mese(int mese, int anno);
double spesa_anno(int anno);
// Media sui mesi dell'anno in cui e' stata registrata almeno una spesa.
double media_mensile(int anno, int* mesi_con_dati = nullptr);
// Media sugli anni registrati. Restituisce 0 se non ci sono dati.
double media_annuale(int* anni_con_dati = nullptr);

// ************ UTILITA' ************

std::string pulisci(const std::string& s);      // toglie spazi iniziali/finali
std::string formatta_euro(double valore);       // "12.50"
bool leggi_numero(const std::string& testo, double& valore);   // accetta anche la virgola

} // namespace diario

#endif
