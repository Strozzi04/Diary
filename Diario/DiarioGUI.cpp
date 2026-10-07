// Diario digitale - interfaccia grafica per Windows (API Win32, nessuna libreria esterna).
// La logica e i file sono gestiti da diario_core.cpp, gli stessi della versione a console:
// i dati salvati con una versione si vedono anche con l'altra.
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>
#include <commctrl.h>

#include <algorithm>
#include <string>
#include <vector>

#include "diario_core.h"

using namespace std;
using namespace diario;

// ************ IDENTIFICATIVI DEI CONTROLLI ************

enum {
    // pagina Giorno
    IDC_G_DATA = 100, IDC_G_STATO, IDC_G_ROUTINE, IDC_G_NUOVA_ROUTINE, IDC_G_AGGIUNGI_ROUTINE,
    IDC_G_RACCONTO, IDC_G_TEMPO, IDC_G_FILM, IDC_G_VOTO, IDC_G_SPESA, IDC_G_SALVA,
    // pagina Routine
    IDC_R_MESE = 200, IDC_R_ANNO, IDC_R_LISTA, IDC_R_NUOVA, IDC_R_AGGIUNGI,
    // pagina Mese
    IDC_M_MESE = 300, IDC_M_ANNO, IDC_M_STATO, IDC_M_CAMPO /* 11 campi: 303 - 313 */,
    IDC_M_SALVA = 330,
    // pagina Anno
    IDC_A_ANNO = 400, IDC_A_OBIETTIVI, IDC_A_NUOVO, IDC_A_AGGIUNGI, IDC_A_RAGGIUNTO,
    IDC_A_NON_RAGGIUNTO, IDC_A_IN_CORSO, IDC_A_ELIMINA, IDC_A_SALVA,
    IDC_A_CAMPO = 420 /* 14 campi: 420 - 433 */,
    // pagina Spese
    IDC_S_MESE = 500, IDC_S_ANNO, IDC_S_RIEPILOGO, IDC_S_GRAFICO,
};

enum Pagina { P_GIORNO, P_ROUTINE, P_MESE, P_ANNO, P_SPESE, N_PAGINE };

const int N_CAMPI_MESE = 11;
const int N_CAMPI_ANNO = 14;

// ************ VARIABILI GLOBALI ************

HINSTANCE g_istanza;
HWND g_finestra, g_schede;
HWND g_pagine[N_PAGINE];
HFONT g_font, g_font_titolo;
int g_dpi = 96;
bool g_caricamento = false;     // true mentre riempiamo i campi da codice
vector<string> g_routine_giorno;
vector<Obiettivo> g_obiettivi;

// ************ UTILITA' ************

int S(int valore) { return MulDiv(valore, g_dpi, 96); }   // scala per schermi ad alta risoluzione

wstring a_wide(const string& s) {
    if (s.empty()) return L"";
    int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), nullptr, 0);
    wstring r(n, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), &r[0], n);
    return r;
}

string a_utf8(const wstring& s) {
    if (s.empty()) return "";
    int n = WideCharToMultiByte(CP_UTF8, 0, s.c_str(), (int)s.size(), nullptr, 0, nullptr, nullptr);
    string r(n, '\0');
    WideCharToMultiByte(CP_UTF8, 0, s.c_str(), (int)s.size(), &r[0], n, nullptr, nullptr);
    return r;
}

// Le caselle di testo di Windows vogliono "\r\n" per andare a capo.
string testo_di(HWND controllo) {
    int n = GetWindowTextLengthW(controllo);
    wstring w(n + 1, L'\0');
    GetWindowTextW(controllo, &w[0], n + 1);
    w.resize(n);
    string s = a_utf8(w);
    s.erase(remove(s.begin(), s.end(), '\r'), s.end());
    return pulisci(s);
}

void imposta_testo(HWND controllo, const string& s) {
    string con_cr;
    for (char c : s) {
        if (c == '\n') con_cr += '\r';
        con_cr += c;
    }
    SetWindowTextW(controllo, a_wide(con_cr).c_str());
}

HWND ctrl(Pagina p, int id) { return GetDlgItem(g_pagine[p], id); }

void messaggio(const wstring& testo, UINT icona = MB_ICONINFORMATION) {
    MessageBoxW(g_finestra, testo.c_str(), L"Diario", MB_OK | icona);
}

HWND crea(HWND padre, const wchar_t* classe, const wchar_t* testo, DWORD stile,
          int x, int y, int l, int a, int id = 0, DWORD stile_ex = 0) {
    HWND h = CreateWindowExW(stile_ex, classe, testo, WS_CHILD | WS_VISIBLE | stile,
                             S(x), S(y), S(l), S(a), padre, (HMENU)(INT_PTR)id, g_istanza, nullptr);
    SendMessageW(h, WM_SETFONT, (WPARAM)g_font, TRUE);
    return h;
}

HWND etichetta(HWND padre, const wchar_t* testo, int x, int y, int l, int a = 20, int id = 0) {
    return crea(padre, L"STATIC", testo, SS_LEFT, x, y, l, a, id);
}

HWND titolo(HWND padre, const wchar_t* testo, int x, int y, int l) {
    HWND h = etichetta(padre, testo, x, y, l, 22);
    SendMessageW(h, WM_SETFONT, (WPARAM)g_font_titolo, TRUE);
    return h;
}

HWND casella(HWND padre, int x, int y, int l, int id, bool multilinea = false, int a = 24) {
    DWORD stile = WS_TABSTOP | WS_BORDER | ES_AUTOHSCROLL;
    if (multilinea) stile = WS_TABSTOP | WS_BORDER | WS_VSCROLL | ES_MULTILINE | ES_WANTRETURN | ES_AUTOVSCROLL;
    return crea(padre, L"EDIT", L"", stile, x, y, l, a, id, WS_EX_CLIENTEDGE);
}

HWND bottone(HWND padre, const wchar_t* testo, int x, int y, int l, int id, int a = 28) {
    return crea(padre, L"BUTTON", testo, WS_TABSTOP | BS_PUSHBUTTON, x, y, l, a, id);
}

HWND lista(HWND padre, int x, int y, int l, int a, int id, DWORD stile_extra, DWORD stile_ex_lista) {
    HWND h = crea(padre, WC_LISTVIEWW, L"", WS_TABSTOP | WS_BORDER | LVS_REPORT | LVS_SINGLESEL |
                  LVS_SHOWSELALWAYS | stile_extra, x, y, l, a, id);
    ListView_SetExtendedListViewStyle(h, LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | stile_ex_lista);
    return h;
}

void aggiungi_colonna(HWND lista, int indice, const wchar_t* testo, int larghezza) {
    LVCOLUMNW c = {};
    c.mask = LVCF_TEXT | LVCF_WIDTH;
    c.pszText = (LPWSTR)testo;
    c.cx = S(larghezza);
    ListView_InsertColumn(lista, indice, &c);
}

void riga_lista(HWND lista, int riga, const vector<wstring>& celle) {
    LVITEMW it = {};
    it.mask = LVIF_TEXT;
    it.iItem = riga;
    it.pszText = (LPWSTR)celle[0].c_str();
    ListView_InsertItem(lista, &it);
    for (size_t i = 1; i < celle.size(); i++) {
        ListView_SetItemText(lista, riga, (int)i, (LPWSTR)celle[i].c_str());
    }
}

// Selettore del mese (menu a tendina) e dell'anno (casella con frecce).
void selettore_periodo(HWND padre, int id_mese, int id_anno, bool con_mese, int mese, int anno) {
    int x = 12;
    if (con_mese) {
        etichetta(padre, L"Mese:", x, 15, 40);
        HWND combo = crea(padre, L"COMBOBOX", L"", WS_TABSTOP | CBS_DROPDOWNLIST | WS_VSCROLL,
                          x + 42, 11, 130, 300, id_mese);
        for (int m = 1; m <= 12; m++) {
            string nome = nome_mese(m);
            nome[0] = (char)toupper(nome[0]);
            SendMessageW(combo, CB_ADDSTRING, 0, (LPARAM)a_wide(nome).c_str());
        }
        SendMessageW(combo, CB_SETCURSEL, mese - 1, 0);
        x += 190;
    }
    etichetta(padre, L"Anno:", x, 15, 40);
    HWND edit = crea(padre, L"EDIT", L"", WS_TABSTOP | WS_BORDER | ES_NUMBER, x + 42, 11, 70, 24,
                     id_anno, WS_EX_CLIENTEDGE);
    HWND frecce = CreateWindowExW(0, UPDOWN_CLASSW, L"", WS_CHILD | WS_VISIBLE | UDS_SETBUDDYINT |
                                  UDS_ALIGNRIGHT | UDS_ARROWKEYS | UDS_NOTHOUSANDS,
                                  0, 0, 0, 0, padre, nullptr, g_istanza, nullptr);
    SendMessageW(frecce, UDM_SETBUDDY, (WPARAM)edit, 0);
    SendMessageW(frecce, UDM_SETRANGE32, 2000, 2100);
    SendMessageW(frecce, UDM_SETPOS32, 0, anno);
}

int mese_scelto(Pagina p, int id) { return (int)SendMessageW(ctrl(p, id), CB_GETCURSEL, 0, 0) + 1; }

int anno_scelto(Pagina p, int id) {
    BOOL ok = FALSE;
    int anno = (int)GetDlgItemInt(g_pagine[p], id, &ok, FALSE);
    return (ok && anno >= 1900 && anno <= 3000) ? anno : 0;
}

wstring euro(double valore) { return a_wide(formatta_euro(valore)) + L" €"; }

// ************ PAGINA GIORNO ************

Data data_scelta() {
    SYSTEMTIME st;
    DateTime_GetSystemtime(ctrl(P_GIORNO, IDC_G_DATA), &st);
    return {st.wDay, st.wMonth, st.wYear};
}

void aggiorna_routine_giorno(const vector<string>& da_spuntare) {
    HWND l = ctrl(P_GIORNO, IDC_G_ROUTINE);
    ListView_DeleteAllItems(l);
    g_routine_giorno = elenco_routine();
    for (size_t i = 0; i < g_routine_giorno.size(); i++) {
        riga_lista(l, (int)i, {a_wide(g_routine_giorno[i])});
        bool fatta = find(da_spuntare.begin(), da_spuntare.end(), g_routine_giorno[i]) != da_spuntare.end();
        ListView_SetCheckState(l, (int)i, fatta);
    }
}

vector<string> routine_spuntate() {
    vector<string> fatte;
    HWND l = ctrl(P_GIORNO, IDC_G_ROUTINE);
    for (size_t i = 0; i < g_routine_giorno.size(); i++) {
        if (ListView_GetCheckState(l, (int)i)) fatte.push_back(g_routine_giorno[i]);
    }
    return fatte;
}

void carica_giorno() {
    g_caricamento = true;
    Data d = data_scelta();
    VoceGiorno voce;
    bool registrato = leggi_giorno(d, voce);

    aggiorna_routine_giorno(voce.routine_fatte);
    imposta_testo(ctrl(P_GIORNO, IDC_G_RACCONTO), voce.racconto);
    imposta_testo(ctrl(P_GIORNO, IDC_G_TEMPO), voce.tempo_atmosferico);
    imposta_testo(ctrl(P_GIORNO, IDC_G_FILM), voce.film_serie_libro);
    SendMessageW(ctrl(P_GIORNO, IDC_G_VOTO), CB_SETCURSEL, voce.voto >= 1 && voce.voto <= 10 ? voce.voto - 1 : -1, 0);
    imposta_testo(ctrl(P_GIORNO, IDC_G_SPESA), registrato ? formatta_euro(voce.spesa) : "");

    wstring stato = registrato
        ? L"Hai già scritto questa giornata: puoi modificarla e salvarla di nuovo."
        : L"Pagina ancora vuota: racconta la tua giornata.";
    SetWindowTextW(ctrl(P_GIORNO, IDC_G_STATO), stato.c_str());
    g_caricamento = false;
}

void salva_pagina_giorno() {
    VoceGiorno voce;
    voce.data = data_scelta();
    voce.routine_fatte = routine_spuntate();
    voce.racconto = testo_di(ctrl(P_GIORNO, IDC_G_RACCONTO));
    voce.tempo_atmosferico = testo_di(ctrl(P_GIORNO, IDC_G_TEMPO));
    voce.film_serie_libro = testo_di(ctrl(P_GIORNO, IDC_G_FILM));

    int voto = (int)SendMessageW(ctrl(P_GIORNO, IDC_G_VOTO), CB_GETCURSEL, 0, 0);
    if (voto < 0) {
        messaggio(L"Scegli un voto per la giornata (da 1 a 10).", MB_ICONWARNING);
        SetFocus(ctrl(P_GIORNO, IDC_G_VOTO));
        return;
    }
    voce.voto = voto + 1;

    string spesa = testo_di(ctrl(P_GIORNO, IDC_G_SPESA));
    if (spesa.empty()) {
        voce.spesa = 0;
    } else if (!leggi_numero(spesa, voce.spesa) || voce.spesa < 0) {
        messaggio(L"La spesa non è valida. Scrivi un numero, ad esempio 12,50.", MB_ICONWARNING);
        SetFocus(ctrl(P_GIORNO, IDC_G_SPESA));
        return;
    }

    salva_giorno(voce);
    carica_giorno();
    messaggio(L"Giornata del " + a_wide(data_testo(voce.data)) + L" salvata!");
}

void costruisci_pagina_giorno(HWND p) {
    etichetta(p, L"Data:", 12, 15, 40);
    HWND data = crea(p, DATETIMEPICK_CLASSW, L"", WS_TABSTOP | DTS_SHORTDATEFORMAT, 54, 11, 130, 24, IDC_G_DATA);
    SYSTEMTIME limiti[2] = {};
    GetLocalTime(&limiti[1]);
    DateTime_SetRange(data, GDTR_MAX, limiti);
    DateTime_SetFormat(data, L"dd/MM/yyyy");
    etichetta(p, L"", 200, 15, 620, 20, IDC_G_STATO);

    titolo(p, L"Routine fatte", 12, 50, 260);
    lista(p, 12, 74, 260, 368, IDC_G_ROUTINE, LVS_NOCOLUMNHEADER, LVS_EX_CHECKBOXES);
    aggiungi_colonna(ctrl(P_GIORNO, IDC_G_ROUTINE), 0, L"Routine", 236);
    etichetta(p, L"Nuova routine:", 12, 452, 260);
    casella(p, 12, 472, 170, IDC_G_NUOVA_ROUTINE);
    bottone(p, L"Aggiungi", 188, 470, 84, IDC_G_AGGIUNGI_ROUTINE);

    titolo(p, L"Racconta la tua giornata", 292, 50, 400);
    casella(p, 292, 74, 528, IDC_G_RACCONTO, true, 186);
    etichetta(p, L"Tempo atmosferico:", 292, 272, 528);
    casella(p, 292, 292, 528, IDC_G_TEMPO);
    etichetta(p, L"Serie tv / film / libro:", 292, 326, 528);
    casella(p, 292, 346, 528, IDC_G_FILM);

    etichetta(p, L"Voto alla giornata:", 292, 382, 150);
    HWND voto = crea(p, L"COMBOBOX", L"", WS_TABSTOP | CBS_DROPDOWNLIST | WS_VSCROLL, 292, 402, 120, 300, IDC_G_VOTO);
    for (int i = 1; i <= 10; i++) SendMessageW(voto, CB_ADDSTRING, 0, (LPARAM)to_wstring(i).c_str());
    etichetta(p, L"Spesa del giorno (€):", 440, 382, 160);
    casella(p, 440, 402, 120, IDC_G_SPESA);

    bottone(p, L"Salva giornata", 292, 452, 180, IDC_G_SALVA, 36);
}

// ************ PAGINA ROUTINE ************

void carica_routine() {
    int mese = mese_scelto(P_ROUTINE, IDC_R_MESE);
    int anno = anno_scelto(P_ROUTINE, IDC_R_ANNO);
    HWND l = ctrl(P_ROUTINE, IDC_R_LISTA);
    ListView_DeleteAllItems(l);
    if (anno == 0) return;
    vector<string> nomi = elenco_routine();
    for (size_t i = 0; i < nomi.size(); i++) {
        int nel_mese = conteggio_routine_mese(nomi[i], mese, anno);
        int nell_anno = conteggio_routine_anno(nomi[i], anno);
        riga_lista(l, (int)i, {a_wide(nomi[i]), to_wstring(max(0, nel_mese)), to_wstring(max(0, nell_anno))});
    }
}

void aggiungi_routine_da(Pagina p, int id_casella) {
    string nome = testo_di(ctrl(p, id_casella));
    string errore = aggiungi_routine(nome);
    if (!errore.empty()) {
        messaggio(a_wide(errore), MB_ICONWARNING);
        return;
    }
    SetWindowTextW(ctrl(p, id_casella), L"");
    aggiorna_routine_giorno(routine_spuntate());
    carica_routine();
}

void costruisci_pagina_routine(HWND p) {
    Data d = oggi();
    selettore_periodo(p, IDC_R_MESE, IDC_R_ANNO, true, d.mese, d.anno);
    titolo(p, L"Quante volte hai fatto le tue routine", 12, 50, 560);
    lista(p, 12, 74, 560, 380, IDC_R_LISTA, 0, LVS_EX_GRIDLINES);
    HWND l = ctrl(P_ROUTINE, IDC_R_LISTA);
    aggiungi_colonna(l, 0, L"Routine", 270);
    aggiungi_colonna(l, 1, L"Volte nel mese", 135);
    aggiungi_colonna(l, 2, L"Volte nell'anno", 135);

    etichetta(p, L"Nuova routine:", 12, 470, 100);
    casella(p, 112, 466, 300, IDC_R_NUOVA);
    bottone(p, L"Aggiungi", 420, 464, 100, IDC_R_AGGIUNGI);

    etichetta(p, L"Le routine sono le abitudini che vuoi tenere sotto controllo "
                 L"(leggere, allenarsi, studiare...).\n\nOgni giorno, nella scheda \"Giorno\", "
                 L"spunta quelle che hai fatto: qui vedrai quante volte le hai fatte in ogni mese "
                 L"e in tutto l'anno.", 592, 74, 228, 200);
}

// ************ PAGINA MESE ************

const wchar_t* const ETICHETTE_MESE[N_CAMPI_MESE] = {
    L"Film:", L"Serie tv:", L"Gioco:", L"Libro:", L"Canzone:",
    L"Oggetto:", L"Video:", L"Miglior acquisto:", L"Miglior regalo:",
    L"Racconta il giorno preferito:", L"Racconta il mese:"
};

vector<string*> campi_mese(PreferitiMese& p) {
    return {&p.film, &p.serie_tv, &p.gioco, &p.libro, &p.canzone, &p.oggetto, &p.video,
            &p.miglior_acquisto, &p.miglior_regalo, &p.giorno_preferito, &p.racconto};
}

void carica_mese() {
    int mese = mese_scelto(P_MESE, IDC_M_MESE);
    int anno = anno_scelto(P_MESE, IDC_M_ANNO);
    if (anno == 0) return;
    g_caricamento = true;
    PreferitiMese p;
    bool presente = leggi_mese(mese, anno, p);
    vector<string*> campi = campi_mese(p);
    for (int i = 0; i < N_CAMPI_MESE; i++) imposta_testo(ctrl(P_MESE, IDC_M_CAMPO + i), *campi[i]);
    SetWindowTextW(ctrl(P_MESE, IDC_M_STATO), presente
        ? L"Preferiti già salvati per questo mese."
        : L"Non hai ancora registrato questo mese.");
    g_caricamento = false;
}

void salva_pagina_mese() {
    int mese = mese_scelto(P_MESE, IDC_M_MESE);
    int anno = anno_scelto(P_MESE, IDC_M_ANNO);
    if (anno == 0) {
        messaggio(L"Anno non valido.", MB_ICONWARNING);
        return;
    }
    PreferitiMese p;
    vector<string*> campi = campi_mese(p);
    for (int i = 0; i < N_CAMPI_MESE; i++) *campi[i] = testo_di(ctrl(P_MESE, IDC_M_CAMPO + i));
    salva_mese(mese, anno, p);
    carica_mese();
    messaggio(L"Preferiti di " + a_wide(nome_mese(mese)) + L" " + to_wstring(anno) + L" salvati!");
}

void costruisci_pagina_mese(HWND p) {
    Data precedente = mese_precedente(oggi());
    selettore_periodo(p, IDC_M_MESE, IDC_M_ANNO, true, precedente.mese, precedente.anno);
    etichetta(p, L"", 340, 15, 480, 20, IDC_M_STATO);
    titolo(p, L"I preferiti del mese", 12, 46, 400);

    for (int i = 0; i < 9; i++) {
        int x = i < 5 ? 12 : 424;
        int y = 74 + (i % 5) * 50;
        etichetta(p, ETICHETTE_MESE[i], x, y, 396);
        casella(p, x, y + 20, 396, IDC_M_CAMPO + i);
    }
    etichetta(p, ETICHETTE_MESE[9], 12, 324, 400);
    casella(p, 12, 344, 808, IDC_M_CAMPO + 9, true, 56);
    etichetta(p, ETICHETTE_MESE[10], 12, 406, 400);
    casella(p, 12, 426, 808, IDC_M_CAMPO + 10, true, 76);
    bottone(p, L"Salva preferiti del mese", 12, 512, 230, IDC_M_SALVA, 34);
}

// ************ PAGINA ANNO ************

const wchar_t* const NOMI_STATO[3] = {L"in corso", L"raggiunto", L"non raggiunto"};

vector<string*> campi_anno(PreferitiAnno& p) {
    vector<string*> campi;
    for (int i = 0; i < 3; i++) campi.push_back(&p.film[i]);
    for (int i = 0; i < 3; i++) campi.push_back(&p.serie[i]);
    for (int i = 0; i < 3; i++) campi.push_back(&p.giochi[i]);
    for (int i = 0; i < 3; i++) campi.push_back(&p.libri[i]);
    campi.push_back(&p.giorno_bello);
    campi.push_back(&p.racconto);
    return campi;
}

void mostra_obiettivi(int selezione = -1) {
    HWND l = ctrl(P_ANNO, IDC_A_OBIETTIVI);
    ListView_DeleteAllItems(l);
    for (size_t i = 0; i < g_obiettivi.size(); i++) {
        riga_lista(l, (int)i, {a_wide(g_obiettivi[i].testo), NOMI_STATO[g_obiettivi[i].stato]});
    }
    if (selezione >= 0 && selezione < (int)g_obiettivi.size()) {
        ListView_SetItemState(l, selezione, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
    }
}

void carica_anno() {
    int anno = anno_scelto(P_ANNO, IDC_A_ANNO);
    if (anno == 0) return;
    g_caricamento = true;
    g_obiettivi = leggi_obiettivi(anno);
    mostra_obiettivi();
    PreferitiAnno p;
    leggi_preferiti_anno(anno, p);
    vector<string*> campi = campi_anno(p);
    for (int i = 0; i < N_CAMPI_ANNO; i++) imposta_testo(ctrl(P_ANNO, IDC_A_CAMPO + i), *campi[i]);
    g_caricamento = false;
}

void modifica_obiettivi(int comando) {
    int anno = anno_scelto(P_ANNO, IDC_A_ANNO);
    if (anno == 0) return;
    int selezione = ListView_GetNextItem(ctrl(P_ANNO, IDC_A_OBIETTIVI), -1, LVNI_SELECTED);

    if (comando == IDC_A_AGGIUNGI) {
        Obiettivo o;
        o.testo = testo_di(ctrl(P_ANNO, IDC_A_NUOVO));
        if (o.testo.empty()) {
            messaggio(L"Scrivi prima il testo dell'obiettivo.", MB_ICONWARNING);
            return;
        }
        g_obiettivi.push_back(o);
        selezione = (int)g_obiettivi.size() - 1;
        SetWindowTextW(ctrl(P_ANNO, IDC_A_NUOVO), L"");
    } else {
        if (selezione < 0) {
            messaggio(L"Seleziona prima un obiettivo dall'elenco.", MB_ICONWARNING);
            return;
        }
        if (comando == IDC_A_RAGGIUNTO) g_obiettivi[selezione].stato = RAGGIUNTO;
        if (comando == IDC_A_NON_RAGGIUNTO) g_obiettivi[selezione].stato = NON_RAGGIUNTO;
        if (comando == IDC_A_IN_CORSO) g_obiettivi[selezione].stato = IN_CORSO;
        if (comando == IDC_A_ELIMINA) {
            wstring domanda = L"Eliminare l'obiettivo \"" + a_wide(g_obiettivi[selezione].testo) + L"\"?";
            if (MessageBoxW(g_finestra, domanda.c_str(), L"Diario", MB_YESNO | MB_ICONQUESTION) != IDYES) return;
            g_obiettivi.erase(g_obiettivi.begin() + selezione);
            selezione = -1;
        }
    }
    salva_obiettivi(anno, g_obiettivi);
    mostra_obiettivi(selezione);
}

void salva_pagina_anno() {
    int anno = anno_scelto(P_ANNO, IDC_A_ANNO);
    if (anno == 0) {
        messaggio(L"Anno non valido.", MB_ICONWARNING);
        return;
    }
    PreferitiAnno p;
    vector<string*> campi = campi_anno(p);
    for (int i = 0; i < N_CAMPI_ANNO; i++) *campi[i] = testo_di(ctrl(P_ANNO, IDC_A_CAMPO + i));
    salva_preferiti_anno(anno, p);
    prepara_anno_routine(anno);
    messaggio(L"Preferiti del " + to_wstring(anno) + L" salvati!");
}

void costruisci_pagina_anno(HWND p) {
    selettore_periodo(p, 0, IDC_A_ANNO, false, 0, oggi().anno);

    titolo(p, L"Obiettivi dell'anno", 12, 46, 400);
    lista(p, 12, 70, 400, 330, IDC_A_OBIETTIVI, 0, LVS_EX_GRIDLINES);
    aggiungi_colonna(ctrl(P_ANNO, IDC_A_OBIETTIVI), 0, L"Obiettivo", 270);
    aggiungi_colonna(ctrl(P_ANNO, IDC_A_OBIETTIVI), 1, L"Stato", 120);
    casella(p, 12, 410, 300, IDC_A_NUOVO);
    bottone(p, L"Aggiungi", 318, 408, 94, IDC_A_AGGIUNGI);
    bottone(p, L"Raggiunto", 12, 444, 95, IDC_A_RAGGIUNTO);
    bottone(p, L"Non raggiunto", 112, 444, 110, IDC_A_NON_RAGGIUNTO);
    bottone(p, L"In corso", 227, 444, 85, IDC_A_IN_CORSO);
    bottone(p, L"Elimina", 317, 444, 95, IDC_A_ELIMINA);

    titolo(p, L"I preferiti dell'anno", 432, 46, 388);
    const wchar_t* gruppi[4] = {L"Film:", L"Serie tv:", L"Giochi:", L"Libri:"};
    for (int g = 0; g < 4; g++) {
        int y = 72 + g * 50;
        etichetta(p, gruppi[g], 432, y, 388);
        for (int i = 0; i < 3; i++) casella(p, 432 + i * 131, y + 20, 125, IDC_A_CAMPO + g * 3 + i);
    }
    etichetta(p, L"Il giorno più bello:", 432, 272, 388);
    casella(p, 432, 292, 388, IDC_A_CAMPO + 12);
    etichetta(p, L"Racconta com'è andato l'anno:", 432, 324, 388);
    casella(p, 432, 344, 388, IDC_A_CAMPO + 13, true, 126);
    bottone(p, L"Salva preferiti dell'anno", 432, 480, 230, IDC_A_SALVA, 34);
}

// ************ PAGINA SPESE ************

void carica_spese() {
    int mese = mese_scelto(P_SPESE, IDC_S_MESE);
    int anno = anno_scelto(P_SPESE, IDC_S_ANNO);
    if (anno == 0) return;
    int mesi = 0, anni = 0;
    double media_m = media_mensile(anno, &mesi);
    double media_a = media_annuale(&anni);

    wstring testo = L"Spesa di " + a_wide(nome_mese(mese)) + L" " + to_wstring(anno) + L":   " +
                    euro(spesa_mese(mese, anno)) + L"\n";
    testo += L"Spesa di tutto il " + to_wstring(anno) + L":   " + euro(spesa_anno(anno)) + L"\n";
    testo += L"Media mensile del " + to_wstring(anno) + L":   " +
             (mesi ? euro(media_m) + L"   (su " + to_wstring(mesi) + (mesi == 1 ? L" mese registrato)" : L" mesi registrati)") : L"nessun dato") + L"\n";
    testo += L"Media annuale:   " +
             (anni ? euro(media_a) + L"   (su " + to_wstring(anni) + (anni == 1 ? L" anno registrato)" : L" anni registrati)") : L"nessun dato");
    SetWindowTextW(ctrl(P_SPESE, IDC_S_RIEPILOGO), testo.c_str());
    InvalidateRect(ctrl(P_SPESE, IDC_S_GRAFICO), nullptr, TRUE);
}

// Grafico a barre delle spese mese per mese dell'anno scelto.
LRESULT CALLBACK GraficoProc(HWND h, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg != WM_PAINT) return DefWindowProcW(h, msg, wp, lp);

    PAINTSTRUCT ps;
    HDC dc = BeginPaint(h, &ps);
    RECT r;
    GetClientRect(h, &r);
    FillRect(dc, &r, GetSysColorBrush(COLOR_WINDOW));
    SelectObject(dc, g_font);
    SetBkMode(dc, TRANSPARENT);

    int anno = anno_scelto(P_SPESE, IDC_S_ANNO);
    int mese_evidenziato = mese_scelto(P_SPESE, IDC_S_MESE);
    double valori[12] = {};
    double massimo = 0;
    for (int m = 0; m < 12 && anno; m++) {
        valori[m] = spesa_mese(m + 1, anno);
        massimo = max(massimo, valori[m]);
    }

    int alto = S(24), basso = r.bottom - S(28);
    int larghezza = (r.right - S(20)) / 12;
    HPEN linea = CreatePen(PS_SOLID, 1, RGB(200, 200, 200));
    HGDIOBJ vecchia = SelectObject(dc, linea);
    MoveToEx(dc, S(10), basso, nullptr);
    LineTo(dc, r.right - S(10), basso);
    SelectObject(dc, vecchia);
    DeleteObject(linea);

    HBRUSH colore = CreateSolidBrush(RGB(92, 132, 204));
    HBRUSH colore_sel = CreateSolidBrush(RGB(232, 140, 60));
    for (int m = 0; m < 12; m++) {
        int x = S(10) + m * larghezza;
        int altezza = massimo > 0 ? (int)((basso - alto) * (valori[m] / massimo)) : 0;
        RECT barra = {x + larghezza / 5, basso - altezza, x + larghezza - larghezza / 5, basso};
        FillRect(dc, &barra, m + 1 == mese_evidenziato ? colore_sel : colore);

        RECT testo_mese = {x, basso + S(4), x + larghezza, r.bottom};
        wstring nome = a_wide(nome_mese(m + 1).substr(0, 3));
        SetTextColor(dc, GetSysColor(COLOR_WINDOWTEXT));
        DrawTextW(dc, nome.c_str(), -1, &testo_mese, DT_CENTER | DT_SINGLELINE);
        if (valori[m] > 0) {
            RECT testo_valore = {x, basso - altezza - S(20), x + larghezza, basso - altezza};
            wstring v = a_wide(formatta_euro(valori[m]));
            DrawTextW(dc, v.c_str(), -1, &testo_valore, DT_CENTER | DT_SINGLELINE | DT_BOTTOM);
        }
    }
    if (massimo == 0) {
        SetTextColor(dc, GetSysColor(COLOR_GRAYTEXT));
        DrawTextW(dc, L"Nessuna spesa registrata in quest'anno", -1, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    DeleteObject(colore);
    DeleteObject(colore_sel);
    EndPaint(h, &ps);
    return 0;
}

void costruisci_pagina_spese(HWND p) {
    Data d = oggi();
    selettore_periodo(p, IDC_S_MESE, IDC_S_ANNO, true, d.mese, d.anno);
    titolo(p, L"Riepilogo spese", 12, 46, 400);
    etichetta(p, L"", 12, 72, 808, 84, IDC_S_RIEPILOGO);
    titolo(p, L"Spese mese per mese", 12, 164, 400);
    crea(p, L"DiarioGrafico", L"", WS_BORDER, 12, 190, 808, 330, IDC_S_GRAFICO);
}

// ************ GESTIONE DEGLI EVENTI ************

void comando(Pagina p, int id, int notifica) {
    bool cambio_periodo = notifica == CBN_SELCHANGE || (notifica == EN_CHANGE &&
        (id == IDC_R_ANNO || id == IDC_M_ANNO || id == IDC_A_ANNO || id == IDC_S_ANNO));
    if (cambio_periodo && !g_caricamento) {
        if (p == P_ROUTINE) carica_routine();
        if (p == P_MESE) carica_mese();
        if (p == P_ANNO) carica_anno();
        if (p == P_SPESE) carica_spese();
        return;
    }
    if (notifica != BN_CLICKED) return;
    switch (id) {
        case IDC_G_SALVA: salva_pagina_giorno(); break;
        case IDC_G_AGGIUNGI_ROUTINE: aggiungi_routine_da(P_GIORNO, IDC_G_NUOVA_ROUTINE); break;
        case IDC_R_AGGIUNGI: aggiungi_routine_da(P_ROUTINE, IDC_R_NUOVA); break;
        case IDC_M_SALVA: salva_pagina_mese(); break;
        case IDC_A_SALVA: salva_pagina_anno(); break;
        case IDC_A_AGGIUNGI: case IDC_A_RAGGIUNTO: case IDC_A_NON_RAGGIUNTO:
        case IDC_A_IN_CORSO: case IDC_A_ELIMINA:
            modifica_obiettivi(id);
            break;
    }
}

LRESULT CALLBACK PaginaProc(HWND h, UINT msg, WPARAM wp, LPARAM lp) {
    Pagina p = (Pagina)GetWindowLongPtrW(h, GWLP_USERDATA);
    switch (msg) {
        case WM_COMMAND:
            comando(p, LOWORD(wp), HIWORD(wp));
            return 0;
        case WM_NOTIFY: {
            NMHDR* n = (NMHDR*)lp;
            if (n->idFrom == IDC_G_DATA && n->code == DTN_DATETIMECHANGE) carica_giorno();
            break;
        }
        case WM_CTLCOLORSTATIC: {
            // etichette con lo stesso sfondo bianco della pagina
            HDC dc = (HDC)wp;
            SetBkColor(dc, GetSysColor(COLOR_WINDOW));
            SetTextColor(dc, GetSysColor(COLOR_WINDOWTEXT));
            return (LRESULT)GetSysColorBrush(COLOR_WINDOW);
        }
    }
    return DefWindowProcW(h, msg, wp, lp);
}

void mostra_pagina(int indice) {
    for (int i = 0; i < N_PAGINE; i++) ShowWindow(g_pagine[i], i == indice ? SW_SHOW : SW_HIDE);
    // le pagine di sola lettura si aggiornano ogni volta che vengono aperte
    if (indice == P_ROUTINE) carica_routine();
    if (indice == P_SPESE) carica_spese();
}

LRESULT CALLBACK FinestraProc(HWND h, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_NOTIFY:
            if (((NMHDR*)lp)->hwndFrom == g_schede && ((NMHDR*)lp)->code == TCN_SELCHANGE) {
                mostra_pagina(TabCtrl_GetCurSel(g_schede));
            }
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(h, msg, wp, lp);
}

// ************ AVVIO ************

void imposta_cartella_dati() {
    // I file del diario vengono salvati nella stessa cartella del programma.
    wchar_t percorso[MAX_PATH];
    DWORD n = GetModuleFileNameW(nullptr, percorso, MAX_PATH);
    if (n == 0 || n >= MAX_PATH) return;
    wstring cartella(percorso);
    size_t pos = cartella.find_last_of(L"\\/");
    if (pos != wstring::npos) SetCurrentDirectoryW(cartella.substr(0, pos).c_str());
}

int WINAPI wWinMain(HINSTANCE istanza, HINSTANCE, PWSTR, int mostra) {
    g_istanza = istanza;
    imposta_cartella_dati();

    INITCOMMONCONTROLSEX icc = {sizeof(icc), ICC_TAB_CLASSES | ICC_DATE_CLASSES | ICC_LISTVIEW_CLASSES |
                                             ICC_UPDOWN_CLASS | ICC_STANDARD_CLASSES};
    InitCommonControlsEx(&icc);

    HDC schermo = GetDC(nullptr);
    g_dpi = GetDeviceCaps(schermo, LOGPIXELSY);
    ReleaseDC(nullptr, schermo);

    NONCLIENTMETRICSW ncm = {};
    ncm.cbSize = sizeof(ncm);
    SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
    g_font = CreateFontIndirectW(&ncm.lfMessageFont);
    LOGFONTW grassetto = ncm.lfMessageFont;
    grassetto.lfWeight = FW_BOLD;
    grassetto.lfHeight = grassetto.lfHeight * 12 / 10;
    g_font_titolo = CreateFontIndirectW(&grassetto);

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.hInstance = istanza;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hIcon = LoadIcon(istanza, MAKEINTRESOURCE(1));
    if (!wc.hIcon) wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);

    wc.lpfnWndProc = FinestraProc;
    wc.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
    wc.lpszClassName = L"DiarioFinestra";
    RegisterClassExW(&wc);

    wc.lpfnWndProc = PaginaProc;
    wc.hbrBackground = GetSysColorBrush(COLOR_WINDOW);
    wc.lpszClassName = L"DiarioPagina";
    wc.hIcon = nullptr;
    RegisterClassExW(&wc);

    wc.lpfnWndProc = GraficoProc;
    wc.lpszClassName = L"DiarioGrafico";
    RegisterClassExW(&wc);

    // Finestra principale a dimensione fissa
    RECT area = {0, 0, S(860), S(600)};
    DWORD stile = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN;
    AdjustWindowRect(&area, stile, FALSE);
    g_finestra = CreateWindowExW(0, L"DiarioFinestra", L"Diario digitale", stile, CW_USEDEFAULT, CW_USEDEFAULT,
                                 area.right - area.left, area.bottom - area.top, nullptr, nullptr, istanza, nullptr);

    RECT cliente;
    GetClientRect(g_finestra, &cliente);
    InflateRect(&cliente, -S(6), -S(6));
    g_schede = CreateWindowExW(0, WC_TABCONTROLW, L"", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_TABSTOP,
                               cliente.left, cliente.top, cliente.right - cliente.left, cliente.bottom - cliente.top,
                               g_finestra, nullptr, istanza, nullptr);
    SendMessageW(g_schede, WM_SETFONT, (WPARAM)g_font, TRUE);

    const wchar_t* nomi[N_PAGINE] = {L"Giorno", L"Routine", L"Mese",
                                     L"Anno", L"Spese"};
    for (int i = 0; i < N_PAGINE; i++) {
        TCITEMW t = {};
        t.mask = TCIF_TEXT;
        t.pszText = (LPWSTR)nomi[i];
        TabCtrl_InsertItem(g_schede, i, &t);
    }

    RECT interno = cliente;
    TabCtrl_AdjustRect(g_schede, FALSE, &interno);
    for (int i = 0; i < N_PAGINE; i++) {
        g_pagine[i] = CreateWindowExW(WS_EX_CONTROLPARENT, L"DiarioPagina", L"", WS_CHILD | WS_CLIPSIBLINGS,
                                      interno.left, interno.top, interno.right - interno.left,
                                      interno.bottom - interno.top, g_finestra, nullptr, istanza, nullptr);
        SetWindowLongPtrW(g_pagine[i], GWLP_USERDATA, i);
        // le pagine devono stare sopra al controllo delle schede, altrimenti restano coperte
        SetWindowPos(g_pagine[i], HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }

    g_caricamento = true;
    costruisci_pagina_giorno(g_pagine[P_GIORNO]);
    costruisci_pagina_routine(g_pagine[P_ROUTINE]);
    costruisci_pagina_mese(g_pagine[P_MESE]);
    costruisci_pagina_anno(g_pagine[P_ANNO]);
    costruisci_pagina_spese(g_pagine[P_SPESE]);
    g_caricamento = false;

    Data d = oggi();
    prepara_anno_routine(d.anno);
    prepara_mese_routine(d.mese, d.anno);
    carica_giorno();
    carica_mese();
    carica_anno();

    // Promemoria come nella versione a console
    int iniziale = P_GIORNO;
    if (d.giorno == 1 && d.mese == 1) iniziale = P_ANNO;
    else if (d.giorno == 1) iniziale = P_MESE;
    TabCtrl_SetCurSel(g_schede, iniziale);
    mostra_pagina(iniziale);

    ShowWindow(g_finestra, mostra);
    UpdateWindow(g_finestra);

    MSG m;
    while (GetMessageW(&m, nullptr, 0, 0) > 0) {
        if (IsDialogMessageW(g_finestra, &m)) continue;   // tasto TAB tra i campi
        TranslateMessage(&m);
        DispatchMessageW(&m);
    }
    DeleteObject(g_font);
    DeleteObject(g_font_titolo);
    return (int)m.wParam;
}
