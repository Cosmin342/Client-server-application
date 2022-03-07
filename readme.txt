Radu Cosmin 323 CB

Tema 2 - Aplicatie client-server TCP si UDP

Am inceput implementarea temei prin deschiderea unui socket udp si a unuia
tcp in server.c, socket care asteapta conexiuni de la clienti. De asemenea,
am initializat o multime de descriptori ce contine initial cei doi socketi
si descriptorul pentru stdin. Apoi, am verificat de fiecare data pe ce
socketi vin date sau daca se primeste comanda "exit" de la tastatura.

Pentru comunicarea intre server si clienti am folosit un protocol bazat pe
structura msg ce contine campurile: topic, type, content, ip_udp si port.
Vor fi completate toate campurile in cazul in care server-ul transmite
client-ului un mesaj primit de la un client udp. Altfel, se va completa doar
campul content, daca server-ul doreste sa ii transmita clientului comanda exit.
In celalalt sens, client-ul poate trimite la server id-ul sau, completand
campul content sau un mesaj cu tip-ul 1 sau 2 (1 - subscribe, completandu-se
campurile content si topic, 2 - unsubscribe, completandu-se campul topic).

Daca server-ul primeste exit de la tastatura, va transmite exit si clientilor
conectati, va inchide socketi aferenti, va dezaloca memoria alocata si se va
inchide. Clientii i-am stocat intr-un array realocabil, in cazul in care se
umplea dublandu-i capacitatea. Daca se primeau date pe socket-ul tcp inseamna
ca un nou client dorea sa se conecteze. Astfel acceptam conexiunea si asteptam
sa trimtia id-ul. Daca exista deja un client cu id-ul respectiv, printam un
mesaj corespunzator si deconectam clientul. Altfel il adaugam in array-ul de
clienti.

Daca server-ul primea date pe socket-ul udp, receptiona mesajul, ii prelucra
continutul si il transmitea la clientii abonati la acel topic care erau
conectati. Daca unul din clienti era neconectat in acel moment, dar era abonat
cu optiunea sf = 1, se stoca mesajul pentru o trimitere ulterioara cand acesta
avea sa fie conectat.

Dupa ce verifica acesti descriptori, server-ul urma sa verifice fiecare socket
asociat unui client. Daca se primea date de la un client, se verifica intai
daca actualul client s-a deconectat, in caz afirmativ fiind eliminat din
array-ul de clienti si afisandu-se un mesaj. Altfel, se clientul se abona
la un topic sau dezabona, in functie de tipul mesajului primit.

Topicurile si mesajele de trimis ulterior le-am retinut tot in array-uri
realocabile, topicurile retinand la randul lor cate un array pentru clientii
abonati.

In subscriber am creat un socket pentru a face posibila conectarea la server si
am dat apoi connect la acesta. De asemenea, am initializat multimea de
descriptori ce avea sa contina socket-ul si descriptorul pentru stdin.

Apoi am verificat daca se primesc date de la tastatura sau de la server. Daca
se primea de la server un mesaj, verificam daca este "exit" sau nu. Altfel,
se primea un mesaj de la un topic si il afisam. Daca primeam date de la
tastatura verificam initial daca s-a primit "exit", iar daca nu se primea,
verificam daca actuala comanda este subscribe sau unsubscribe cu numarul
aferent de parametrii, in caz negativ printand un mesaj corespunzator.
Pentru subscribe si unsubscribe creeam un mesaj corespunzator ce il transmiteam
la server si afisam faptul ca un client s-a abonat/dezabonat de la un topic.

Am folosit in tema pentru verificarea apelurilor de sistem macro-ul DIE din
laboratorul 8.