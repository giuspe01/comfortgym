# Palestra digitale

Caso di studio del corso **ITPS** (Università degli Studi di Bari, primo anno
secondo semestre, traccia 2).

Web application per una palestra digitale: i clienti seguono programmi di
allenamento e piani nutrizionali, i professionisti (trainer e nutrizionisti)
li creano e monitorano i progressi.

## Architettura

- **Backend**: C++17, server HTTP con [cpp-httplib](https://github.com/yhirose/cpp-httplib)
  (single-header), database [SQLite3](https://www.sqlite.org/) compilato dal
  sorgente amalgamation.
- **Frontend**: HTML + CSS + JavaScript vanilla, [Bootstrap](https://getbootstrap.com/)
  via CDN.
- **Comunicazione**: REST API JSON tra browser e backend.

Tutte le dipendenze C/C++ sono incluse in [third_party/](third_party/) come
single-header / single-source: il progetto è auto-contenuto, **non richiede di
installare librerie esterne**.

## Requisiti

Per **compilare e sviluppare** servono solo:

| Strumento | Versione minima | Verifica |
|---|---|---|
| `g++` | GCC 7+ (C++17) | `g++ --version` |
| `make` | GNU Make 3.81+ | `make --version` |
| `git` | qualunque | `git --version` |

Per **eseguire** il binario compilato non serve nulla: l'eseguibile è linkato
staticamente, basta copiare la cartella e lanciarlo.

### Setup su Windows (con scoop)

```powershell
# 1. Installa scoop se non c'è (https://scoop.sh)
# 2. Installa il toolchain
scoop install gcc make git
```

Riavvia la shell oppure aggiorna il PATH della sessione corrente:

```powershell
$env:PATH = [Environment]::GetEnvironmentVariable('Path','Machine') + ';' + [Environment]::GetEnvironmentVariable('Path','User')
```

### Setup su Linux (Debian/Ubuntu)

```bash
sudo apt update && sudo apt install -y build-essential git
```

### Setup su macOS

```bash
xcode-select --install
brew install make
```

## Compilare ed eseguire

```bash
git clone <url-repository>
cd uniba
make            # compila -> palestra(.exe)
make run        # compila e avvia il server
```

Apri il browser su [http://localhost:8080](http://localhost:8080).

Altri target utili:

```bash
make clean      # rimuove build/ e l'eseguibile
make rebuild    # clean + all
```

## Struttura del progetto

```
uniba/
├── Makefile                  # Build cross-platform
├── README.md                 # Questo file
├── docs/                     # Documentazione del caso di studio
│   └── documentazione.md
├── third_party/              # Librerie single-header (incluse)
│   ├── httplib.h             # cpp-httplib v0.18.3
│   ├── json.hpp              # nlohmann/json v3.11.3
│   ├── sqlite3.c, sqlite3.h  # SQLite 3.47 amalgamation
│   └── sqlite3ext.h
├── public/                   # Frontend statico (HTML, CSS, JS)
├── src/                      # Codice C++
│   ├── main.cpp              # Entry point del server
│   ├── modello/              # Classi dominio (Utente, Programma, ...)
│   ├── strutture/            # Strutture dati generiche (Lista<T>)
│   ├── persistenza/          # Wrapper SQLite
│   ├── servizi/              # Business logic (auth, sessioni)
│   ├── controller/           # Route handler HTTP
│   └── utils/                # Helper (eccezioni, hash, http utils)
└── data/                     # Database SQLite (creato al primo avvio)
```

## Stato dello sviluppo

Vedi [docs/documentazione.md](docs/documentazione.md) per analisi, requisiti,
progettazione, codifica e test.

## Licenza

Progetto didattico. Le librerie in `third_party/` mantengono le rispettive
licenze (MIT per cpp-httplib e nlohmann/json, public domain per SQLite).
