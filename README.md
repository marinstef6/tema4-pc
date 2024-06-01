    In aceasta tema am folosit mai multe fisiere cum ar fi: buffer.c in care
    se definesc operatii pentru gestionarea unui buffer, helpers.c in care se
    definesc o serie de functii pentru realizarea comunicarilor prin retea
    folosind socket-uri si fisierul request.c care ajuta la construirea si
    formatarea cererilor HTTP pentru un client in C.
    Aceste fisiere au fost luate din laborator si m-am mai folosit si de un
    fisier parson.c luat de pe github :https://github.com/kgabis/parson care m-a
    ajutat sa lucrez cu JSON.
    In fifiserul client am scris codul care contine 5 functii si main.
    Prima functie, remove_newline, este folosita pentru a sterge caracterul "\n"
    de la final atunci cand folosesc fgets().
    A doua functie, check_number, este folosita pentru a verifica daca un sir
    de caractere este un numar intreg valid. Daca este afiseaza 1, altfel 0. Se
    va verifica daca pointerul este NULL, apoi lungimea sirului pe care o va
    stoca intr-o variabila. Va verifica si daca primul caracter este 0, si daca
    este adevarat va afisa 0. Mai am si o conditie care permite folosirea
    semnului minus si apoi se face verificarea daca exista doar cifre. La final
    se returneaza 0 sau 1.
    A treia functie, free_function, este folosita pentru a elibera memoria.
    A patra functie, create_json_payload, este folosita pentru a crea un sir de
    caractere in format JSON care contine 2 campuri: username si password.
    Se va initializa obiectul JSON, apoi se obtine referinta la obiectul JSON.
    Se vor seta campurile username si password folosind functia json_object_
    set_string(), se face serializarea obiectului in sir de caractere, acest
    lucru permitand sa fie transmis ca parte a unei cereri HTTP si dupa se va
    elibera memoria obiectului JSON. La final se vor returna datele.
    A cincea functie, auth_login este utilizata pentru procesul de autentificare
    prin intermediul unui socket TCP. Se va construi un URL pentru ruta de acces
    apoi se va seta tipul de continut, acesta fiind application/json. Se vor
    citi datele de la utilizator, adica nume si parola prin functia getline, se
    va crea payload-ul JSON ce contine username si password, se va construi 
    cererea HTTP POST, se trimite cererea la server, se curata memoria si la
    final se va returna raspunsul primit de la server.
    In functia main, se vor trata cele 8 cazuri. Voi folosi "setvbuf" pentru a 
    afisa in terminal direct ce se va afisa la stdout. Apoi voi aloca memorie
    pentru credentiale, cookie, payload. In while o sa incep sa tratez fiecare
    caz. Prima data voi citi comanda de la tastatura si voi trata cazurile in
    care pot primi erori. Voi elimina "\n" deoarece folosesc fgets si apoi
    implementez comanda "register". Se va inregistra un client de la deschiderea
    pana la primirea si procesarea raspunsului serverului. Prima data se va 
    verifica comanda, adica daca "register" a fost introdus corect. Apoi se va 
    deschide conexiunea TCP prin functia open_conection. Daca esueaza se va 
    afisa o eroare, altfel va trece la autentificarea utilizatorului folosind
    functia auth_login. Se va verifica raspunsul primit de la server si daca e
    NULL se afiseaza mesaj de eroare si se va inchide conexiunea. In caz contrar
    se primeste un raspuns si se cauta cuvantul error pentru a vedea daca a esuat.
    Daca nu se gaseste atunci inregistrarea a fost un succes si se afisa un mesaj
    pentru a confirma. La final se elibereaza memoria si se inchide conexiunea.
    Urmatoarea comanda, "login", incepe tot cu verificarea scrierii corecte a 
    comenzii. Daca este corecta atunci se deschide conexiunea TCP si se vor afisa
    mesaje de eroare acolo unde este cazul. Se verifica starea de autentificare
    pentru a vedea daca exista un cookie setat, indicand ca este deja logat.
    Daca este cazul se afiseaza un mesaj de eroare si se reia, altfel se va 
    solicita autentificarea la server. Daca nu primeste username si password, va
    afisa un mesaj de eroare. Se evalueaza raspunsul in cazul in care se vor
    trimite datele corecte si apoi se cauta cuvantul error pentru a stii ce mesaj
    se va afisa. La final se cauta cookie-ul pentru a-l salva si utilizat
    ulterior si apoi se va afisa un mesaj de succes. La final se vor elibera 
    resursele si se inchide conexiunea.
    In cazul "enter_library", se verifica prima data daca cookie este setat,
    fapt care indica ca utilizatorul este deja autentificat. Daca nu exista cookie 
    atunci se va afisa un mesaj de eroare si continua bucla pentru a primi si alte 
    comenzi.
    Va incerca sa stabileasca o conexiune TCP si va afisa din nou un mesaj daca
    nu reuseste. Daca reuseste atunci va construi un URL pentru acces si va 
    genera o cerere GET folosing "compute_get_delete_request". Primeste raspuns
    de la server si va verifica daca exista vreo eroare. Daca intalneste afiseaza
    eroarea, altfel va extrage jwt(Json Web Token) din raspuns. Acesta ne va 
    ajuta pentru urmatoarea autentificare si pentru accesul la resursele biblio-
    tecii. Tokenul va fi parsat prin JSON. La final se elibereaza memoria si se
    va inchide conexiunea.
    In cazul "get_books" prima data se va verifica daca exista un cookie valid 
    pentru a vedea daca utilizatorul este logat. Daca nu exista cookie se va 
    afisa un mesaj de eroare.
    Se va conecta la server prin TCP, iar daca va esua se va afisa din nou o
    eroare.In caz contrar se va construi un URL si se trimite o cerere GET la
    server folosind compute_get_delete_request. Dupa trimiterea cererii, primeste
    un raspuns de la server, iar daca contine cuvantul error atunci se va afisa 
    un mesaj de eroare. Altfel se verifica daca raspunsul este in format JSON si
    va extrage datele incepand de la primul caracter "[". Se va parsa un string
    JSON si apoi daca array-ul contine carti se afiseaza id-ul si titlul cartilor.
    Daca nu exista carti se afiseaza un mesaj de eroare si la final se elibereaza
    memoria si se inchide conexiunea.
    Pentru comanda "get_book" prima data se verifica daca exista cookie activ 
    care indica daca utilizatorul este logat. Daca nu exista cookie se afiseaza
    un mesaj de eroare. Se va deschide o conexiune la server folosind un socket
    si se va afisa o eroare daca esueaza. In caz contrar, utilizatorul este 
    solicitat sa introduca ID-ul cartii pe care o doreste. Daca esueaza citirea
    ID-ului atunci se afiseaza un mesaj de eroare si se inchide socketul.
    Se contruieste un URL si o cerere GET, iar in URL se va specifica si ID-ul.
    Se va verifica ca ID-ul introdus nu depaseste limita maxima, apoi se va 
    trimite cererea la server. Dupa ce primeste mesjaul va verifica daca contine
    cuvantul error si in caz afirmativ va afisa un mesaj, altfel daca mesajul e
    valid se va parsa JSON-ul pentru a lua datele cartii. Daca datele sunt parsate
    corect se afiseaza date precum: titlul, autor, editura, gen si numarul de 
    pagini. Daca parsarea esueaza se afiseaza un mesaj de eroare. La final se 
    va elibera memoria si se inchide conexiunea la server.
    In cazul comenzii "delete_book" se va verifica ca si in cazurile de mai sus
    daca exista cookie activ si in caz contrar se afiseaza un mesaj de eroare.
    Se stabileste conexiunea la server, iar daca esueaza se afiseaza un mesaj de
    eroare. In caz contrar utilizatorului i se va solicita sa introduca ID-ul
    cartii pe care doreste sa o stearga. Se citeste ID si se va elimina "\n".
    Se creeaza un URL si o cerere DELETE si se verifica dimensiunea ID-ului.
    Cererea DELETE va fi trimisa catre server si dupa ce primeste raspuns 
    verifica sa nu primeasca o eroare. Daca nu primeste atunci se va sterge 
    cartea si se va afisa un mesaj de succes, se elibereaza memoria si se inchide
    serverul.
    Comanda "add_book" verifica daca este cativ cookie-ul, daca nu este atunci
    afiseaza un mesaj si reia bucla. Va stabili o conexiune cu serverul, apoi
    i se va solicita utilizatorului sa introduca detalii despre noua carte,
    introducand titlul, autorul, genul, editura si numarul de pagini.
    Fiecare data va fi citita de la tastatura si se va elimina "\n".
    Toate datele vor fi setate intr-un JSON. Numarul de pagini va fi verificat
    ca sa fie format doar din cifre si in caz contrar se va afisa un mesaj de 
    eroare. Altfel se vor serializa datele si vor fi trimise serverului.
    Se construieste o cerere POST folosind detaliile cartii si se trimite
    catre server. Se verifica daca mesajul este de eroare sau de succes, se 
    va afisa, se va elibera memoria si dupa se inchide conexiunea.
    Comanda "logout" verifica daca exista cookie activ pentru a vedea daca este
    utilizatorul logat. Apoi va stabili o conexiune cu serverul, iar daca esueaza
    va afisa un mesaj corespunzator. In caz contrar va crea si trimite o cerere
    GET pentru a deconecta utilizatorul. Va primi un raspuns de la server si va
    verifica daca procesul este completat cu succes. Daca apare o eroare atunci
    se va afisa un mesaj, apoi se va elibera memoria si se va inchide conexiunea.
    Ultima comanda, "exit" va fi folosita pentru a inchide aplciatia client.
    Aceasta va elibera resursele si va inchide aplicatia.
    Daca comanda nu a fost introdusa atunci se va afisa un mesaj de eroare.
    
