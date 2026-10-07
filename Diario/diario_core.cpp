#include "diario_core.h"

#include <algorithm>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <map>
#include <sstream>

using namespace std;

namespace diario {

// ************ UTILITA' INTERNE ************

namespace {

const char* const NOMI_MESI[12] = {
    "gennaio", "febbraio", "marzo", "aprile", "maggio", "giugno",
    "luglio", "agosto", "settembre", "ottobre", "novembre", "dicembre"
};

vector<string> leggi_righe(const string& nome_file) {
    vector<string> righe;
    ifstream in(nome_file);
    string riga;
    while (getline(in, riga)) {
        // i file salvati da Windows possono avere "\r" a fine riga
        if (!riga.empty() && riga.back() == '\r') riga.pop_back();
        righe.push_back(riga);
    }
    return righe;
}

void scrivi_righe(const string& nome_file, const vector<string>& righe) {
    ofstream out(nome_file, ios::trunc);
    for (const string& r : righe) out << r << "\n";
}

bool file_esiste(const string& nome_file) {
    ifstream in(nome_file);
    return in.good();
}

vector<string> dividi(const string& s, char sep) {
    vector<string> campi;
    stringstream ss(s);
    string campo;
    while (getline(ss, campo, sep)) campi.push_back(campo);
    return campi;
}

// I testi possono contenere "a capo": nei file li salviamo come "\n"
// cosi' ogni campo resta su una sola riga.
string codifica(const string& s) {
    string r;
    for (char c : s) {
        if (c == '\\') r += "\\\\";
        else if (c == '\n') r += "\\n";
        else if (c != '\r') r += c;
    }
    return r;
}

string decodifica(const string& s) {
    string r;
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == '\\' && i + 1 < s.size()) {
            if (s[i + 1] == 'n') { r += '\n'; i++; continue; }
            if (s[i + 1] == '\\') { r += '\\'; i++; continue; }
        }
        r += s[i];
    }
    return r;
}

// Legge un file di tipo "chiave: valore" e restituisce la mappa.
map<string, string> leggi_chiavi(const string& nome_file) {
    map<string, string> valori;
    for (const string& riga : leggi_righe(nome_file)) {
        size_t pos = riga.find(": ");
        if (pos == string::npos) continue;
        valori[riga.substr(0, pos)] = decodifica(riga.substr(pos + 2));
    }
    return valori;
}

double leggi_totale(const string& nome_file) {
    ifstream in(nome_file);
    double totale = 0;
    if (!(in >> totale)) totale = 0;
    return totale;
}

void aggiungi_a_totale(const string& nome_file, double delta) {
    if (delta == 0 && file_esiste(nome_file)) return;
    double totale = leggi_totale(nome_file) + delta;
    if (totale < 0.005) totale = 0;     // evita "-0.00" dovuti agli arrotondamenti
    ofstream out(nome_file, ios::trunc);
    out << formatta_euro(totale);
}

string file_spese_mese(int mese, int anno) {
    return "spese_" + to_string(mese) + "_" + to_string(anno) + ".txt";
}

string file_spese_anno(int anno) {
    return "spese_" + to_string(anno) + ".txt";
}

string file_routine_mese(int mese, int anno) {
    return to_string(mese) + "_" + to_string(anno) + "_routine.txt";
}

string file_giorno(Data d) {
    return data_testo(d) + "_giorno.txt";
}

// ---- routine.txt: "nome;anno;volte;anno;volte;" ----

struct RigaRoutine {
    string nome;
    vector<pair<int, int>> anni;    // (anno, volte)
};

vector<RigaRoutine> leggi_routine_txt() {
    vector<RigaRoutine> elenco;
    for (const string& riga : leggi_righe("routine.txt")) {
        vector<string> campi;
        // ignora i campi vuoti (es. ";;" lasciati dalla vecchia versione)
        for (const string& c : dividi(riga, ';')) {
            if (!pulisci(c).empty()) campi.push_back(pulisci(c));
        }
        if (campi.empty()) continue;
        RigaRoutine r;
        r.nome = campi[0];
        for (size_t i = 1; i + 1 < campi.size(); i += 2) {
            try {
                r.anni.push_back({stoi(campi[i]), stoi(campi[i + 1])});
            } catch (...) {
                // campo non numerico: lo saltiamo
            }
        }
        elenco.push_back(r);
    }
    return elenco;
}

void scrivi_routine_txt(const vector<RigaRoutine>& elenco) {
    vector<string> righe;
    for (const RigaRoutine& r : elenco) {
        string riga = r.nome + ";";
        for (const auto& a : r.anni) {
            riga += to_string(a.first) + ";" + to_string(a.second) + ";";
        }
        righe.push_back(riga);
    }
    scrivi_righe("routine.txt", righe);
}

// ---- file mensile: "nome:volte;" ----

vector<pair<string, int>> leggi_routine_mese(int mese, int anno) {
    vector<pair<string, int>> elenco;
    for (const string& riga : leggi_righe(file_routine_mese(mese, anno))) {
        size_t pos = riga.rfind(':');
        if (pos == string::npos) continue;
        string nome = pulisci(riga.substr(0, pos));
        string numero = riga.substr(pos + 1);
        if (!numero.empty() && numero.back() == ';') numero.pop_back();
        int n = 0;
        try { n = stoi(numero); } catch (...) { n = 0; }
        if (!nome.empty()) elenco.push_back({nome, n});
    }
    return elenco;
}

void scrivi_routine_mese(int mese, int anno, const vector<pair<string, int>>& elenco) {
    vector<string> righe;
    for (const auto& r : elenco) righe.push_back(r.first + ":" + to_string(r.second) + ";");
    scrivi_righe(file_routine_mese(mese, anno), righe);
}

// Aggiunge delta (+1 o -1) al conteggio della routine nel mese e nell'anno.
void modifica_conteggio(const string& nome, int mese, int anno, int delta) {
    vector<RigaRoutine> elenco = leggi_routine_txt();
    bool trovata = false;
    for (RigaRoutine& r : elenco) {
        if (r.nome != nome) continue;
        trovata = true;
        bool anno_trovato = false;
        for (auto& a : r.anni) {
            if (a.first == anno) {
                a.second = max(0, a.second + delta);
                anno_trovato = true;
            }
        }
        if (!anno_trovato) r.anni.push_back({anno, max(0, delta)});
    }
    if (!trovata) elenco.push_back({nome, {{anno, max(0, delta)}}});
    scrivi_routine_txt(elenco);

    prepara_mese_routine(mese, anno);
    vector<pair<string, int>> mensili = leggi_routine_mese(mese, anno);
    trovata = false;
    for (auto& r : mensili) {
        if (r.first == nome) {
            r.second = max(0, r.second + delta);
            trovata = true;
        }
    }
    if (!trovata) mensili.push_back({nome, max(0, delta)});
    scrivi_routine_mese(mese, anno, mensili);
}

int giorni_nel_mese(int mese, int anno) {
    static const int giorni[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (mese == 2 && ((anno % 4 == 0 && anno % 100 != 0) || anno % 400 == 0)) return 29;
    return giorni[mese - 1];
}

} // namespace

// ************ DATE ************

Data oggi() {
    time_t adesso = time(nullptr);
    tm* locale = localtime(&adesso);
    return {locale->tm_mday, locale->tm_mon + 1, locale->tm_year + 1900};
}

Data mese_precedente(Data d) {
    if (d.mese == 1) return {1, 12, d.anno - 1};
    return {1, d.mese - 1, d.anno};
}

bool data_valida(Data d) {
    if (d.anno < 1900 || d.anno > 3000 || d.mese < 1 || d.mese > 12) return false;
    return d.giorno >= 1 && d.giorno <= giorni_nel_mese(d.mese, d.anno);
}

string data_testo(Data d) {
    return to_string(d.giorno) + "-" + to_string(d.mese) + "-" + to_string(d.anno);
}

string nome_mese(int mese) {
    if (mese < 1 || mese > 12) return "";
    return NOMI_MESI[mese - 1];
}

// ************ ROUTINE ************

vector<string> elenco_routine() {
    // Unisce routine.txt ed elenco_routine.txt: la vecchia versione a volte
    // sovrascriveva elenco_routine.txt perdendo alcune routine.
    vector<string> nomi;
    for (const RigaRoutine& r : leggi_routine_txt()) nomi.push_back(r.nome);
    for (const string& riga : leggi_righe("elenco_routine.txt")) {
        string nome = pulisci(riga);
        if (!nome.empty() && find(nomi.begin(), nomi.end(), nome) == nomi.end()) {
            nomi.push_back(nome);
        }
    }
    return nomi;
}

string aggiungi_routine(const string& nome_grezzo) {
    string nome = pulisci(nome_grezzo);
    if (nome.empty()) return "Il nome della routine e' vuoto.";
    if (nome.find_first_of(";:\n\r") != string::npos) {
        return "Il nome della routine non puo' contenere ';' o ':'.";
    }
    vector<string> nomi = elenco_routine();
    if (find(nomi.begin(), nomi.end(), nome) != nomi.end()) {
        return "La routine \"" + nome + "\" esiste gia'.";
    }
    nomi.push_back(nome);
    scrivi_righe("elenco_routine.txt", nomi);

    Data d = oggi();
    vector<RigaRoutine> elenco = leggi_routine_txt();
    elenco.push_back({nome, {{d.anno, 0}}});
    scrivi_routine_txt(elenco);
    prepara_anno_routine(d.anno);
    prepara_mese_routine(d.mese, d.anno);
    return "";
}

int conteggio_routine_anno(const string& nome, int anno) {
    for (const RigaRoutine& r : leggi_routine_txt()) {
        if (r.nome != nome) continue;
        for (const auto& a : r.anni) {
            if (a.first == anno) return a.second;
        }
    }
    return -1;
}

int conteggio_routine_mese(const string& nome, int mese, int anno) {
    for (const auto& r : leggi_routine_mese(mese, anno)) {
        if (r.first == nome) return r.second;
    }
    return -1;
}

void prepara_anno_routine(int anno) {
    vector<RigaRoutine> elenco = leggi_routine_txt();
    vector<string> nomi = elenco_routine();
    bool modificato = false;
    for (const string& nome : nomi) {
        bool presente = false;
        for (const RigaRoutine& r : elenco) presente = presente || r.nome == nome;
        if (!presente) {
            elenco.push_back({nome, {}});
            modificato = true;
        }
    }
    for (RigaRoutine& r : elenco) {
        bool ha_anno = false;
        for (const auto& a : r.anni) ha_anno = ha_anno || a.first == anno;
        if (!ha_anno) {
            r.anni.push_back({anno, 0});
            modificato = true;
        }
    }
    if (modificato) scrivi_routine_txt(elenco);
}

void prepara_mese_routine(int mese, int anno) {
    vector<pair<string, int>> mensili = leggi_routine_mese(mese, anno);
    bool modificato = !file_esiste(file_routine_mese(mese, anno));
    for (const string& nome : elenco_routine()) {
        bool presente = false;
        for (const auto& r : mensili) presente = presente || r.first == nome;
        if (!presente) {
            mensili.push_back({nome, 0});
            modificato = true;
        }
    }
    if (modificato) scrivi_routine_mese(mese, anno, mensili);
}

// ************ GIORNO ************

bool giorno_registrato(Data d) {
    return file_esiste(file_giorno(d));
}

bool leggi_giorno(Data d, VoceGiorno& voce) {
    vector<string> righe = leggi_righe(file_giorno(d));
    if (righe.empty()) return false;

    voce = VoceGiorno();
    voce.data = d;
    bool spesa_trovata = false;

    for (size_t i = 0; i < righe.size(); i++) {
        string riga = pulisci(righe[i]);
        size_t pos = riga.find(':');
        if (pos == string::npos) continue;
        string chiave = riga.substr(0, pos);
        string valore = pulisci(riga.substr(pos + 1));

        // La vecchia versione scriveva il racconto sulla riga successiva
        if (valore.empty() && chiave == "racconto giornata" && i + 1 < righe.size()) {
            valore = pulisci(righe[i + 1]);
        }

        if (chiave == "routine") {
            for (const string& r : dividi(valore, ';')) {
                if (!pulisci(r).empty()) voce.routine_fatte.push_back(pulisci(r));
            }
        } else if (chiave == "racconto giornata") {
            voce.racconto = decodifica(valore);
        } else if (chiave == "tempo atmosferico") {
            voce.tempo_atmosferico = decodifica(valore);
        } else if (chiave == "serie tv / film / libro") {
            voce.film_serie_libro = decodifica(valore);
        } else if (chiave == "voto alla giornata") {
            try { voce.voto = stoi(valore); } catch (...) { voce.voto = 0; }
            // Nella vecchia versione la spesa era il numero sulla riga dopo il voto
            double s;
            if (!spesa_trovata && i + 1 < righe.size() && leggi_numero(righe[i + 1], s)) {
                voce.spesa = s;
            }
        } else if (chiave == "spesa") {
            double s;
            if (leggi_numero(valore, s)) {
                voce.spesa = s;
                spesa_trovata = true;
            }
        }
    }
    return true;
}

void salva_giorno(const VoceGiorno& voce) {
    const Data d = voce.data;

    // Se il giorno era gia' registrato annulliamo la registrazione precedente,
    // altrimenti routine e spese verrebbero contate due volte.
    VoceGiorno vecchia;
    if (leggi_giorno(d, vecchia)) {
        for (const string& r : vecchia.routine_fatte) modifica_conteggio(r, d.mese, d.anno, -1);
        aggiungi_a_totale(file_spese_mese(d.mese, d.anno), -vecchia.spesa);
        aggiungi_a_totale(file_spese_anno(d.anno), -vecchia.spesa);
    }

    prepara_anno_routine(d.anno);
    prepara_mese_routine(d.mese, d.anno);
    for (const string& r : voce.routine_fatte) modifica_conteggio(r, d.mese, d.anno, +1);
    aggiungi_a_totale(file_spese_mese(d.mese, d.anno), voce.spesa);
    aggiungi_a_totale(file_spese_anno(d.anno), voce.spesa);

    string routine;
    for (const string& r : voce.routine_fatte) routine += r + ";";

    ofstream out(file_giorno(d), ios::trunc);
    out << data_testo(d) << "\n";
    out << "routine: " << routine << "\n";
    out << "racconto giornata: " << codifica(voce.racconto) << "\n";
    out << "tempo atmosferico: " << codifica(voce.tempo_atmosferico) << "\n";
    out << "serie tv / film / libro: " << codifica(voce.film_serie_libro) << "\n";
    out << "voto alla giornata: " << voce.voto << "\n";
    out << "spesa: " << formatta_euro(voce.spesa) << "\n";
}

// ************ MESE ************

namespace {
string file_mese(int mese, int anno) {
    return to_string(mese) + "_" + to_string(anno) + ".txt";
}
}

bool leggi_mese(int mese, int anno, PreferitiMese& p) {
    if (!file_esiste(file_mese(mese, anno))) return false;
    map<string, string> v = leggi_chiavi(file_mese(mese, anno));
    p.film = v["film preferito"];
    p.serie_tv = v["serie tv preferita"];
    p.gioco = v["gioco preferito"];
    p.libro = v["libro preferito"];
    p.canzone = v["canzone preferita"];
    p.oggetto = v["oggetto preferito"];
    p.video = v["video preferito"];
    p.miglior_acquisto = v["acquisto preferito"];
    p.miglior_regalo = v["regalo preferito"];
    p.giorno_preferito = v["giorno preferito"];
    p.racconto = v["racconto del mese"];
    return true;
}

void salva_mese(int mese, int anno, const PreferitiMese& p) {
    ofstream out(file_mese(mese, anno), ios::trunc);
    out << "preferiti di " << nome_mese(mese) << " " << anno << "\n";
    out << "film preferito: " << codifica(p.film) << "\n";
    out << "serie tv preferita: " << codifica(p.serie_tv) << "\n";
    out << "gioco preferito: " << codifica(p.gioco) << "\n";
    out << "libro preferito: " << codifica(p.libro) << "\n";
    out << "canzone preferita: " << codifica(p.canzone) << "\n";
    out << "oggetto preferito: " << codifica(p.oggetto) << "\n";
    out << "video preferito: " << codifica(p.video) << "\n";
    out << "acquisto preferito: " << codifica(p.miglior_acquisto) << "\n";
    out << "regalo preferito: " << codifica(p.miglior_regalo) << "\n";
    out << "giorno preferito: " << codifica(p.giorno_preferito) << "\n";
    out << "racconto del mese: " << codifica(p.racconto) << "\n";
}

// ************ ANNO ************

namespace {
string file_obiettivi(int anno) {
    return "obbiettivi_anno_" + to_string(anno) + ".txt";
}
string file_preferiti_anno(int anno) {
    return "preferiti_anno_" + to_string(anno) + ".txt";
}
}

vector<Obiettivo> leggi_obiettivi(int anno) {
    vector<Obiettivo> obiettivi;
    for (const string& riga : leggi_righe(file_obiettivi(anno))) {
        string testo = pulisci(riga);
        if (!testo.empty() && testo.back() == ';') testo.pop_back();
        if (testo.empty()) continue;
        Obiettivo o;
        const string fatto = ": fatto", non_fatto = ": non fatto";
        if (testo.size() > non_fatto.size() &&
            testo.compare(testo.size() - non_fatto.size(), non_fatto.size(), non_fatto) == 0) {
            o.stato = NON_RAGGIUNTO;
            testo.erase(testo.size() - non_fatto.size());
        } else if (testo.size() > fatto.size() &&
                   testo.compare(testo.size() - fatto.size(), fatto.size(), fatto) == 0) {
            o.stato = RAGGIUNTO;
            testo.erase(testo.size() - fatto.size());
        }
        o.testo = decodifica(testo);
        obiettivi.push_back(o);
    }
    return obiettivi;
}

void salva_obiettivi(int anno, const vector<Obiettivo>& obiettivi) {
    vector<string> righe;
    for (const Obiettivo& o : obiettivi) {
        string testo = pulisci(o.testo);
        if (testo.empty()) continue;
        string riga = codifica(testo);
        if (o.stato == RAGGIUNTO) riga += ": fatto";
        else if (o.stato == NON_RAGGIUNTO) riga += ": non fatto";
        righe.push_back(riga + ";");
    }
    scrivi_righe(file_obiettivi(anno), righe);
}

bool leggi_preferiti_anno(int anno, PreferitiAnno& p) {
    if (!file_esiste(file_preferiti_anno(anno))) return false;
    map<string, string> v = leggi_chiavi(file_preferiti_anno(anno));
    for (int i = 0; i < 3; i++) {
        string n = " " + to_string(i + 1);
        p.film[i] = v["film" + n];
        p.serie[i] = v["serie tv" + n];
        p.giochi[i] = v["gioco" + n];
        p.libri[i] = v["libro" + n];
    }
    p.giorno_bello = v["giorno piu' bello"];
    p.racconto = v["racconto dell'anno"];
    return true;
}

void salva_preferiti_anno(int anno, const PreferitiAnno& p) {
    ofstream out(file_preferiti_anno(anno), ios::trunc);
    out << "preferiti dell'anno " << anno << "\n";
    for (int i = 0; i < 3; i++) out << "film " << i + 1 << ": " << codifica(p.film[i]) << "\n";
    for (int i = 0; i < 3; i++) out << "serie tv " << i + 1 << ": " << codifica(p.serie[i]) << "\n";
    for (int i = 0; i < 3; i++) out << "gioco " << i + 1 << ": " << codifica(p.giochi[i]) << "\n";
    for (int i = 0; i < 3; i++) out << "libro " << i + 1 << ": " << codifica(p.libri[i]) << "\n";
    out << "giorno piu' bello: " << codifica(p.giorno_bello) << "\n";
    out << "racconto dell'anno: " << codifica(p.racconto) << "\n";
}

// ************ SPESE ************

double spesa_mese(int mese, int anno) {
    return leggi_totale(file_spese_mese(mese, anno));
}

double spesa_anno(int anno) {
    return leggi_totale(file_spese_anno(anno));
}

double media_mensile(int anno, int* mesi_con_dati) {
    double totale = 0;
    int mesi = 0;
    for (int mese = 1; mese <= 12; mese++) {
        if (file_esiste(file_spese_mese(mese, anno))) {
            totale += spesa_mese(mese, anno);
            mesi++;
        }
    }
    if (mesi_con_dati) *mesi_con_dati = mesi;
    return mesi == 0 ? 0 : totale / mesi;
}

double media_annuale(int* anni_con_dati) {
    double totale = 0;
    int anni = 0;
    for (int anno = 2000; anno <= 2100; anno++) {
        if (file_esiste(file_spese_anno(anno))) {
            totale += spesa_anno(anno);
            anni++;
        }
    }
    if (anni_con_dati) *anni_con_dati = anni;
    return anni == 0 ? 0 : totale / anni;
}

// ************ UTILITA' ************

string pulisci(const string& s) {
    size_t inizio = s.find_first_not_of(" \t\r\n");
    if (inizio == string::npos) return "";
    size_t fine = s.find_last_not_of(" \t\r\n");
    return s.substr(inizio, fine - inizio + 1);
}

string formatta_euro(double valore) {
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%.2f", valore);
    return buffer;
}

bool leggi_numero(const string& testo, double& valore) {
    string t = pulisci(testo);
    replace(t.begin(), t.end(), ',', '.');
    if (t.empty()) return false;
    try {
        size_t letti = 0;
        valore = stod(t, &letti);
        return letti == t.size();
    } catch (...) {
        return false;
    }
}

} // namespace diario
