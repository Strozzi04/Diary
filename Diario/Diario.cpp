#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <limits>
//srand(unsigned(time(NULL))); 
//n1 = rand()%10;
//SetConsoleTextAttribute(h, 14);
using namespace std;
//	system("pause");
struct Routine{
	bool fatto;
	string nome;
	int n_volte;
	
	
};
struct Giorno{
	time_t tempo_corrente = std::time(nullptr);
    tm* tempo_locale = std::localtime(&tempo_corrente);
	int giorno = tempo_locale->tm_mday;
    int mese = tempo_locale->tm_mon + 1;
    int anno = tempo_locale->tm_year + 1900;
	string materie[10];
	int voto_del_giorno;
	string tempo_atmosferico;
	string racconta_la_giornata;
	string racconta_film_o_serie;
	float acquisti;
};
struct Mese{
	time_t tempo_corrente = std::time(nullptr);
    tm* tempo_locale = std::localtime(&tempo_corrente);
	 int mese = tempo_locale->tm_mon + 1;
    int anno = tempo_locale->tm_year + 1900;
	float acquisti_mese;
	string serie_tv;
	string film;
	string libro;
	string gioco;
	string giorno_preferito;
	string canzone;
	string oggetto;
	string miglior_acquisto;
	string miglior_regalo;
	string video;
	string racconta_il_mese;
	
};
	
struct Anno{
	time_t tempo_corrente = std::time(nullptr);
    tm* tempo_locale = std::localtime(&tempo_corrente);
    int anno = tempo_locale->tm_year + 1900;
 
	vector <string> obbiettivi;
	int n_obbiettivi_raggiunti;
	int n_obbiettivi_nn_raggiunti;
};

	Giorno g;
	Mese m;
	Anno a;
	void registra_giorno();
	void setta_diario();
	void registra_mese();
	void registr_anno(){

    time_t tempo_corrente = std::time(nullptr);
    tm* tempo_locale = std::localtime(&tempo_corrente);
    int anno_corrente = tempo_locale->tm_year + 1900;
    int anno_precedente = anno_corrente - 1;

    cout << "Registrazione anno " << anno_corrente << endl;

    // ************ 1) AGGIUNTA NUOVO ANNO IN routine.txt ************

    ifstream r_in("routine.txt");
    vector<string> righe_routine;
    string riga_rt;

    while(getline(r_in, riga_rt)){
        if(riga_rt != ""){
            righe_routine.push_back(riga_rt);
        }
    }
    r_in.close();

    for(int i=0; i<righe_routine.size(); i++){

        string r = righe_routine[i];

        if(r[r.size()-1] == ';'){
            r.erase(r.size()-1, 1);
        }

        r += ";" + to_string(anno_corrente) + ";0;";

        righe_routine[i] = r;
    }

    ofstream r_out("routine.txt", ios::trunc);
    for(int i=0; i<righe_routine.size(); i++){
        r_out << righe_routine[i] << endl;
    }
    r_out.close();

    cout << "Aggiunto anno " << anno_corrente << " alle routine." << endl;


    // ************ 2) OBIETTIVI ANNO PRECEDENTE ************

    string nome_file_prec = "obbiettivi_anno_" + to_string(anno_precedente) + ".txt";
    ifstream inputfile(nome_file_prec);

    vector<string> obbiettivi_vec;
    vector<string> esito_vec;

    if(inputfile.good()){
        cout << "Obbiettivi dell'anno precedente trovati:" << endl;

        string line;
        while(getline(inputfile, line)){
            if(line != ""){
                obbiettivi_vec.push_back(line);
            }
        }
        inputfile.close();

        for(int i=0; i<obbiettivi_vec.size(); i++){
            cout << obbiettivi_vec[i] << endl;
            cout << "Raggiunto? (1 = sì, 0 = no): ";
            int r;
            cin >> r;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            if(r == 1){
                esito_vec.push_back("fatto");
            } else {
                esito_vec.push_back("non fatto");
            }
        }

        ofstream outputfile(nome_file_prec, ios::trunc);
        for(int i=0; i<obbiettivi_vec.size(); i++){
            outputfile << obbiettivi_vec[i] << ": " << esito_vec[i] << ";" << endl;
        }
        outputfile.close();

    } else {
        cout << "Nessun obbiettivo dell'anno precedente trovato." << endl;
    }


    // ************ 3) NUOVI OBIETTIVI ANNO CORRENTE ************

    string nome_file_nuovo = "obbiettivi_anno_" + to_string(anno_corrente) + ".txt";
    ofstream nuovo_output(nome_file_nuovo);

    int n;
    cout << "Quanti obbiettivi vuoi registrare per il nuovo anno? ";
    cin >> n;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    string ob;

    for(int i=0; i<n; i++){
        cout << "Inserisci obbiettivo " << i+1 << ": ";
        getline(cin, ob);
        nuovo_output << ob << ";" << endl;
    }

    nuovo_output.close();


    // ************ 4) PREFERITI DELL’ANNO PRECEDENTE ************

    string nome_file_pref = "preferiti_anno_" + to_string(anno_precedente) + ".txt";
    ofstream pref(nome_file_pref);

    cout << "Ora registrerai i preferiti dell’anno precedente." << endl;

    string film[3], serie[3], giochi[3], libri[3];
    string giorno_bello, racconto_anno;

    cout << "Inserisci i 3 film preferiti:" << endl;
    for(int i=0; i<3; i++){
        cout << "Film " << i+1 << ": ";
        getline(cin, film[i]);
    }

    cout << "Inserisci le 3 serie TV preferite:" << endl;
    for(int i=0; i<3; i++){
        cout << "Serie " << i+1 << ": ";
        getline(cin, serie[i]);
    }

    cout << "Inserisci i 3 giochi preferiti:" << endl;
    for(int i=0; i<3; i++){
        cout << "Gioco " << i+1 << ": ";
        getline(cin, giochi[i]);
    }

    cout << "Inserisci i 3 libri preferiti:" << endl;
    for(int i=0; i<3; i++){
        cout << "Libro " << i+1 << ": ";
        getline(cin, libri[i]);
    }

    cout << "Qual è stato il giorno più bello dell’anno? ";
    getline(cin, giorno_bello);

    cout << "Racconta com'è andato l’anno: ";
    getline(cin, racconto_anno);

    pref << "Film preferiti:" << endl;
    for(int i=0; i<3; i++) pref << "- " << film[i] << endl;

    pref << "Serie TV preferite:" << endl;
    for(int i=0; i<3; i++) pref << "- " << serie[i] << endl;

    pref << "Giochi preferiti:" << endl;
    for(int i=0; i<3; i++) pref << "- " << giochi[i] << endl;

    pref << "Libri preferiti:" << endl;
    for(int i=0; i<3; i++) pref << "- " << libri[i] << endl;

    pref << "Giorno più bello: " << giorno_bello << endl;
    pref << "Racconto dell’anno: " << racconto_anno << endl;

    pref.close();

    cout << "Registrazione anno completata." << endl;
}
void mostra_statistiche_routine() {

    string nome_routine;
    int scelta;
    int anno, mese;

    cout << "Inserisci il nome della routine che vuoi controllare: ";
    cin >> nome_routine;

    cout << "Vuoi vedere:\n";
    cout << "1) Quante volte l'hai fatta in un mese di un anno\n";
    cout << "2) Quante volte l'hai fatta in tutto un anno\n";
    cout << "Scelta: ";
    cin >> scelta;

    // ******************** CASO 1: MENSILE ********************
    if (scelta == 1) {

        cout << "Inserisci anno (es: 2025): ";
        cin >> anno;
        cout << "Inserisci mese (1-12): ";
        cin >> mese;

        string nome_file = to_string(mese);
        nome_file += "_";
        nome_file += to_string(anno);
        nome_file += "_routine.txt";

        ifstream in(nome_file);

        if (!in.good()) {
            cout << "Nessun file trovato per quel mese/anno." << endl;
            return;
        }

        string riga;
        bool trovato = false;

        while (getline(in, riga)) {
            // ogni riga è tipo: leggere:5;
            if (riga.find(nome_routine + ":") == 0) {

                size_t pos1 = riga.find(':');
                size_t pos2 = riga.find(';');

                string numero = riga.substr(pos1 + 1, pos2 - pos1 - 1);
                cout << "Hai fatto \"" << nome_routine << "\" " << numero 
                     << " volte nel mese " << mese << " dell'anno " << anno << "." << endl;

                trovato = true;
                break;
            }
        }

        if (!trovato) {
            cout << "Routine non trovata in quel mese." << endl;
        }

        in.close();
    }

    // ******************** CASO 2: ANNUALE ********************
    else if (scelta == 2) {

        cout << "Inserisci anno (es: 2025): ";
        cin >> anno;

        ifstream in("routine.txt");

        if (!in.good()) {
            cout << "routine.txt non trovato." << endl;
            return;
        }

        string riga;
        bool trovato = false;

        while (getline(in, riga)) {
            if (riga.find(nome_routine + ";") == 0) {

                // es: leggere;2024;10;2025;3;
                stringstream ss(riga);
                string campo;
                vector<string> campi;

                while (getline(ss, campo, ';')) {
                    campi.push_back(campo);
                }

                for (int i = 1; i < campi.size(); i += 2) {

                    string anno_file = campi[i];

                    if (anno_file == to_string(anno)) {
                        cout << "Nel " << anno << " hai fatto \"" 
                             << nome_routine << "\" " << campi[i + 1] << " volte." << endl;

                        trovato = true;
                        break;
                    }
                }
            }
        }

        if (!trovato) {
            cout << "Routine non trovata in quell'anno." << endl;
        }

        in.close();
    }

    else {
        cout << "Scelta non valida." << endl;
    }
}
void mostra_obbiettivi_anno() {

    int anno_corrente = a.anno;
    string file = "obbiettivi_anno_" + to_string(anno_corrente) + ".txt";

    ifstream in(file);
    if (!in.good()) {
        cout << "Non ci sono obiettivi registrati per quest'anno." << endl;
        return;
    }

    cout << "\n--- OBIETTIVI " << anno_corrente << " ---\n\n";

    string riga;
    while (getline(in, riga)) {
        cout << riga << endl;
    }

    in.close();
}
void registra_spesa_mensile(float spesa) {

    string nome_file = "spese_" + to_string(g.mese) + "_" + to_string(g.anno) + ".txt";

    ifstream in(nome_file);
    float totale = 0;

    // Se esiste, leggo il totale precedente
    if (in.good()) {
        in >> totale;
    }
    in.close();

    totale += spesa;

    // Riscrivo il totale aggiornato
    ofstream out(nome_file);
    out << totale;
    out.close();
}
void registra_spesa_annuale(float spesa) {

    string nome_file = "spese_" + to_string(g.anno) + ".txt";

    ifstream in(nome_file);
    float totale = 0;

    if (in.good()) {
        in >> totale;
    }
    in.close();

    totale += spesa;

    ofstream out(nome_file);
    out << totale;
    out.close();
}
void mostra_spese() {

    int scelta;
    cout << "\nVuoi vedere:\n";
    cout << "1) Spese del mese corrente\n";
    cout << "2) Spese di un mese specifico\n";
    cout << "3) Spese dell'anno corrente\n";
    cout << "4) Spese di un anno specifico\n";
    cout << "5) Media mensile dell'anno corrente\n";
    cout << "6) Media annuale (media degli anni registrati)\n";
    cout << "Scelta: ";
    cin >> scelta;

    // 1) Spesa mese corrente
    if (scelta == 1) {

        string nome_file = "spese_" + to_string(g.mese) + "_" + to_string(g.anno) + ".txt";
        ifstream in(nome_file);
        float spesa = 0;

        if (in.good()) in >> spesa;
        in.close();

        cout << "Hai speso " << spesa << " euro nel mese attuale." << endl;
    }

    // 2) Spesa mese specifico
    else if (scelta == 2) {

        int mese, anno;
        cout << "Mese: ";
        cin >> mese;
        cout << "Anno: ";
        cin >> anno;

        string nome_file = "spese_" + to_string(mese) + "_" + to_string(anno) + ".txt";
        ifstream in(nome_file);
        float spesa = 0;

        if (in.good()) in >> spesa;
        in.close();

        cout << "Hai speso " << spesa << " euro in " << mese << "/" << anno << endl;
    }

    // 3) Spese anno corrente
    else if (scelta == 3) {

        string nome_file = "spese_" + to_string(g.anno) + ".txt";
        ifstream in(nome_file);
        float spesa = 0;

        if (in.good()) in >> spesa;
        in.close();

        cout << "Hai speso " << spesa << " euro in tutto l'anno corrente." << endl;
    }

    // 4) Spese anno specifico
    else if (scelta == 4) {

        int anno;
        cout << "Anno: ";
        cin >> anno;

        string nome_file = "spese_" + to_string(anno) + ".txt";
        ifstream in(nome_file);
        float spesa = 0;

        if (in.good()) in >> spesa;
        in.close();

        cout << "Hai speso " << spesa << " euro nell'anno " << anno << endl;
    }

    // 5) Media mensile dell'anno corrente
    else if (scelta == 5) {

        float totale_annuale = 0;
        string file_annuale = "spese_" + to_string(g.anno) + ".txt";

        ifstream in(file_annuale);
        if (in.good()) in >> totale_annuale;
        in.close();

        float media = totale_annuale / 12.0;

        cout << "Media di spesa mensile nel " << g.anno << ": " 
             << media << " euro." << endl;
    }

    // 6) Media annuale di tutti gli anni registrati
    else if (scelta == 6) {

        float somma_anni = 0;
        int anni = 0;

        // Cerchiamo file spese_X.txt da 2000 a 2100
        for (int anno = 2000; anno <= 2100; anno++) {

            string nome_file = "spese_" + to_string(anno) + ".txt";
            ifstream in(nome_file);

            if (in.good()) {
                float val = 0;
                in >> val;
                somma_anni += val;
                anni++;
            }

            in.close();
        }

        if (anni == 0) {
            cout << "Non ci sono dati annuali registrati." << endl;
            return;
        }

        float media_annuale = somma_anni / anni;

        cout << "Media annuale delle spese (su " << anni
             << " anni registrati): " << media_annuale << " euro." << endl;
    }

    else {
        cout << "Scelta non valida." << endl;
    }
}


	/*
		ifstream intputfile("giorno.txt");
		ifstream intputfile("mese.txt");
		ifstream intputfile("anno.txt");
		ofstream outputfile("giorno.txt");
		ofstream outputfile("mese.txt");
		ofstream outputfile("anno.txt");
		
			time_t tempo_corrente = std::time(nullptr);
    tm* tempo_locale = std::localtime(&tempo_corrente);
	int giorno = tempo_locale->tm_mday;
    int mese = tempo_locale->tm_mon + 1;
    int anno = tempo_locale->tm_year + 1900;
		*/
			void routine_mese();
	int main(){
		int scelta;
	cout<<"setta il diario o aggiungi una routine = 1 "<<endl;
	cout<<"registra il diario del giorno = 2"<<endl;
	if(g.giorno==1){
		cout<<"oggi e' il primo del mese, puoi registrare anche i preferiti del mese scorso = 3"<<endl;
	}
	if(g.giorno==1&&g.mese==1){
		cout<<"oggi è anche il primo dell'anno vuoi registrare l'anno = 4"<<endl;
	}
	cout << "vuoi vedere statistiche routine = 5" << endl;
	cout<< "vuoi vedere gli obbiettivi dell'anno corrente = 6"<<endl;
	cout<<"vedi spese registrate = 7"<<endl;

	cout<<"inserisci: ";
	cin>>scelta;
	cin.ignore(numeric_limits<streamsize>::max(), '\n');
	switch(scelta){
		case 1:
			setta_diario();
			
		break;
		case 2:
			cout<<"registra il tuo diario";
			 routine_mese();
			registra_giorno();
		break;
		case 3:
			 routine_mese();
			 registra_mese();
		break;
		case 4:
			routine_mese();
			registr_anno();
		break;
		case 5:
    mostra_statistiche_routine();
    break;
	case 6:
		mostra_obbiettivi_anno();
	break;
	case 7:
    mostra_spese();
    break;
	default:
		cout<< "errore";
	break;
		
	}
		
	
}
	void routine_mese(){
		string nome_file=to_string(m.mese);
		nome_file+="_"+to_string(m.anno)+"_routine.txt";
		
		ifstream check(nome_file);
		if(check.good()){
			cout << "File di routine mensile gia' esistente." << endl;
			check.close();
			return;
		}
		check.close();

		int k=0;
		string line;
		ifstream intputfile("elenco_routine.txt");
		if(!intputfile.good()){
			return;
		}
		while(getline(intputfile,line)){
			k++;
		}
		intputfile.close();
		intputfile.open("elenco_routine.txt");
		vector<string> routine(k);
		for(int i=0;i<k;i++){
			getline(intputfile,routine[i]);
		}
		intputfile.close();
		ofstream outputfile(nome_file);
		for(int i=0;i<k;i++){
			outputfile<<routine[i]<<":"<<"0"<<";"<<endl;
		}
		outputfile.close();
	}
	void registra_mese(){
		int mese_prec = m.mese - 1;
		int anno_prec = m.anno;
		if (mese_prec == 0) {
			mese_prec = 12;
			anno_prec--;
		}
		string nome_file=to_string(mese_prec);
		nome_file+="_"+to_string(anno_prec);
		nome_file+=".txt";
		cout<<nome_file<<endl;
		ofstream outputfile(nome_file);
		outputfile<<nome_file<<" preferiti del mese: "<<endl;
		cout<<"Ora registrerai i preferiti del mese scorso: ";
		cout<<"Film: ";
		getline(cin,m.film);
		outputfile<<"film preferito: "<<m.film<<endl;
		cout<<"serie_tv: ";
		getline(cin,m.serie_tv);
		outputfile<<"serie tv preferita: "<<m.serie_tv<<endl;
		cout<<"gioco: ";
		getline(cin,m.gioco);
		outputfile<<"gioco preferito: "<<m.gioco<<endl;
		cout<<"libro: ";
		getline(cin,m.libro);
		outputfile<<"libro preferito: "<<m.libro<<endl;
		cout<<"canzone: ";
		getline(cin,m.canzone);
		outputfile<<"canzone preferita: "<<m.canzone<<endl;
		cout<<"oggetto: ";
		getline(cin,m.oggetto);
		outputfile<<"oggetto preferito: "<<m.oggetto<<endl;
		cout<<"video: ";
		getline(cin,m.video);
		outputfile<<"video preferito: "<<m.video<<endl;
		cout<<"acquisto: ";
		getline(cin,m.miglior_acquisto);
		outputfile<<"acquisto preferito: "<<m.miglior_acquisto<<endl;
		cout<<"regalo: ";
		getline(cin,m.miglior_regalo);
		outputfile<<"regalo preferito: "<<m.miglior_regalo<<endl;
		cout<<"racconta il giorno preferito giorno: ";
		getline(cin,m.giorno_preferito);
		outputfile<<"giorno preferito: "<<m.giorno_preferito<<endl;
		cout<<"racconta il mese: ";
		getline(cin,m.racconta_il_mese);
		outputfile<<"giorno preferito: "<<m.racconta_il_mese<<endl;
		
	}
void setta_diario(){
	time_t tempo_corrente = std::time(nullptr);
    tm* tempo_locale = std::localtime(&tempo_corrente);
    int anno = tempo_locale->tm_year + 1900;
	ifstream intputfile("routine.txt");
	string line;
	getline(intputfile, line);
	if(line==" "){
		intputfile.close();
		int n;
		cout<<"inserisci il numero di routine che vuoi inserire: ";
		cin>>n;
		ofstream outputfile("routine.txt",ios::trunc);
		vector<Routine> routines(n);
		for(int i=0;i<n;i++){
		cout<<"inserisci le tue routine, come: leggere ,giocare, allenarsi etc. ";
		cin>>routines[i].nome;
		routines[i].n_volte=0;
		outputfile<<routines[i].nome<<";"<<anno<<";"<<routines[i].n_volte<<";"<<endl;
			
		}
		outputfile.close();
			outputfile.open("elenco_routine.txt");
			cout<<routines[0].nome<<endl;
			for(int i=0;i<n;i++){
				outputfile<<routines[i].nome<<endl;
			}
				outputfile.close();
	}else{
		intputfile.close();
		cout<<"quante routine vuoi aggiungere?";
		int n;
		cin>>n;
		vector<Routine> routines(n);
		ofstream outputfile("routine.txt",ios::app);
		
		for(int i=0;i<n;i++){
			cout<<"inserisci il nome della routine: ";
			cin>>routines[i].nome;
			routines[i].n_volte=0;
			outputfile<<routines[i].nome<<";"<<anno<<";"<<routines[i].n_volte<<";"<<endl;
			
		}
		outputfile.close();
		outputfile.open("elenco_routine.txt");
			for(int i=0;i<n;i++){
				outputfile<<routines[i].nome<<endl;
			}
			outputfile.close();
	}
}
void registra_giorno(){
	string data=to_string(g.giorno);
	data+="-";
	data+=to_string(g.mese);
	data+="-";
	data+=to_string(g.anno);
	string file=data+"_giorno.txt";
		ofstream outputfile(file);
		outputfile<<data<<endl;
		cout<<endl<<data<<endl;
				outputfile.close();
		ifstream intputfile("routine.txt");
		int k=0;
		string line;
			while(getline(intputfile,line)){
			if(!line.empty()) k++;
		}
		vector<Routine> routines(k);
		intputfile.close();
		
		intputfile.open("routine.txt");
		vector<string> routine_lines(k);
		for(int i=0;i<k;i++){
			getline(intputfile, routine_lines[i]);
			if(!routine_lines[i].empty() && routine_lines[i].back() == ';'){
				routine_lines[i].pop_back();
			}
			
			stringstream ss(routine_lines[i]);
			getline(ss, routines[i].nome, ';');

			string anno_c = to_string(g.anno);
			string campo_anno, campo_valore;
			routines[i].n_volte = 0;
			while(getline(ss, campo_anno, ';')){
				if(getline(ss, campo_valore, ';')){
					if(campo_anno == anno_c){
						routines[i].n_volte = stoi(campo_valore);
						break;
					}
				}
			}
		}
		intputfile.close();

	cout<<"inserisci 1 se ha fatto la routine, 0 se non l'hai fatta. Le tue routine sono: "<<endl;
	vector<int> routine_n(k);
	for(int i=0;i<k;i++){
		cout<<routines[i].nome<<endl;	
		}
	cout<<"inserisci in questo modo 1 0 1 0;"<<endl<<"inserisci: ";
	for(int i=0;i<k;i++){
	cin>>routine_n[i];
	if(routine_n[i]==1){
		routines[i].fatto=true;
		stringstream ss(routine_lines[i]);
    		string campo;
    		vector<string> campi;
    while (getline(ss, campo, ';')) {
        campi.push_back(campo);
    }
			for(int v=1;v<campi.size();v++){
				string anno_c=to_string(g.anno);
				if(campi[v]==anno_c){
					int n = stoi(campi[v+1]);
					n++;
					campi[v+1]=to_string(n);
					routine_lines[i]= campi[0];
    				for (int l = 1; l < campi.size(); l++) {
					 routine_lines[i]+= ";" + campi[l];
    				}
    				campi={};
					break;
				}
			}
	}
}
cin.ignore(numeric_limits<streamsize>::max(), '\n');
	outputfile.open("routine.txt",ios::trunc);
	for(int i=0;i<k;i++){
		outputfile<<routine_lines[i]<<";"<<endl;
	}
	outputfile.close();
	outputfile.open(file,ios::app);
		outputfile<<"routine: ";
	for(int i=0;i<k;i++){
		if(routines[i].fatto==true){
			outputfile<<routines[i].nome<<";";
		}
	}
		outputfile<<endl<<"racconto giornata: ";
	cout<<"racconta la tua giornata: ";
	getline(cin,g.racconta_la_giornata);
	outputfile<<endl<<g.racconta_la_giornata<<endl;
	outputfile<<endl<<"tempo atmosferico: ";
	cout<<"come era il tempo oggi ?";
	getline(cin,g.tempo_atmosferico);
	outputfile<<endl<<"serie tv / film / libro:";
	cout<<"racconta una serie tv o un film o un libro:";
	getline(cin,g.racconta_film_o_serie);
	outputfile<<endl<<" voto alla giornata: ";
	cout<<"dai un voto alla giornata: ";
	cin>>g.voto_del_giorno;
	cout<<"quanto hai speso?: ";
	cin>>g.acquisti;
	cin.ignore(numeric_limits<streamsize>::max(), '\n');
	registra_spesa_mensile(g.acquisti);
	registra_spesa_annuale(g.acquisti);

	outputfile<<g.voto_del_giorno<<endl;
	outputfile<<g.acquisti<<endl;
	outputfile.close();
		time_t tempo_corrente = std::time(nullptr);
    tm* tempo_locale = std::localtime(&tempo_corrente);
    int anno = tempo_locale->tm_year + 1900;
	// Costruzione nome file
string nome_file = to_string(m.mese);
nome_file += "_" + to_string(m.anno) + "_routine.txt";

// 1) leggere tutto il file
ifstream in(nome_file);
vector<string> righe;
string riga;

while (getline(in, riga)) {
    righe.push_back(riga);
}
in.close();

// 2) modificare le righe in base a routines[i].fatto
for (int i = 0; i < k; i++) {

    if (routines[i].fatto == false) 
        continue;    // questa routine non va modificata

    for (string& r : righe) {

        // Controllo se la riga inizia con il nome es: "allenamento:"
        if (r.find(routines[i].nome + ":") == 0) {

            size_t pos = r.find(':');
            size_t end = r.find(';');

            string numero = r.substr(pos + 1, end - pos - 1);
            int n = stoi(numero);

            n++;  // incremento se fatto = true

            r = routines[i].nome + ":" + to_string(n) + ";";  // ricostruzione riga
        }
    }
}

// 3) riscrittura del file
ofstream out(nome_file);
for (const string& r : righe) {
    out << r << "\n";
}
out.close();

	
}