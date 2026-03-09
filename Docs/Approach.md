# Approach (Avvicinamento) – Documentazione feature

Feature per la pianificazione e visualizzazione dell’avvicinamento a un waypoint landable (campo/aviosuperficie) con scelta tra approccio **diretto** e **circuito di traffico**.

## Accesso

- **Schermata Target**: bottone **Approach** accanto a **OK** (stessa riga). Visibile quando il dialog Target è aperto; il waypoint di riferimento è il task point corrente.
- Il bottone Approach è stato rimosso dal dialog Dettaglio waypoint.

## Dialog Approach

All’apertura si usa lo stesso rendering della mappa Target (centrato sul waypoint, stessa vista).

### Tipo di approccio (due bottoni)

- **Diretto**: avvicinamento in linea retta dal punto a 5 km sul prolungamento asse pista fino al centro pista.
- **Circuito**: circuito di traffico VFR (sottovento, base, finale).

Il bottone della scelta attiva è evidenziato (bordo nero, sfondo chiaro) per indicare la modalità in uso.

### Pista assegnata

Due bottoni con le direzioni della pista (es. 09 / 27). La pista selezionata è evidenziata. Obbligatoria per disegnare sia il diretto sia il circuito.

### Circuito: lato

Se è selezionato **Circuito**, compaiono **Sinistro** e **Destro** per il lato del circuito. Anche qui la scelta attiva è evidenziata.

## Disegno sulla mappa

- **Diretto** (solo se è selezionato il bottone Diretto e una pista): segmento da **5 km** sul prolungamento asse pista (direzione di avvicinamento) fino al **centro pista**.
- **Circuito** (solo se è selezionato Circuito, una pista e un lato):  
  - Sottovento a ~800 m dal centro (≈ 30 s a 50 kt).  
  - Punto di virata base sull’intersezione con la radiale 45° dalla soglia.  
  - Base e finale fino al centro pista.

Il disegno viene eseguito solo quando le scelte necessarie sono complete (Diretto + pista, oppure Circuito + pista + lato).

## Track / Bearing

- In modalità **Diretto**, la **track** (linea di bearing dall’aereo) termina al **punto a 5 km** (inizio del braccio diretto), non al centro del waypoint.
- In modalità **Circuito**, la track termina al centro del waypoint.

## Dettagli tecnici

- **Waypoint**: rappresentazione grafica come in Target (runway + etichetta), anche in Approach Pan.
- **WndButton**: stato `SetSelected(bool)` per evidenziare la scelta (aspetto “reverse”: rialzato, bordo nero, sfondo evidenziato).
- **Costanti**: Direct = 5 km (`DIRECT_5KM_M`); circuito: offset sottovento 800 m, virata base a 800 m lungo il sottovento.
- **File principali**: `DrawApproach.cpp`, `DrawBearing.cpp`, `dlgApproach.cpp`, `dlgTarget.cpp`, `WindowControls.cpp` (SetSelected), dialoghi XML `dlgApproach_P.xml` / `dlgApproach_L.xml`.
