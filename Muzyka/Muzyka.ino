#include "pitches.h"

const int speakerPin = 13; 

// --- MELODIE ---

// 1. STAR WARS MAIN THEME
int starWarsMelody[] = {
  NOTE_G4, NOTE_G4, NOTE_G4, NOTE_C5, NOTE_G5, 
  NOTE_F5, NOTE_E5, NOTE_D5, NOTE_C6, NOTE_G5,
  NOTE_F5, NOTE_E5, NOTE_D5, NOTE_C6, NOTE_G5,
  NOTE_F5, NOTE_E5, NOTE_F5, NOTE_D5
};
int starWarsDurations[] = {
  8, 8, 8, 2, 2, 
  8, 8, 8, 2, 4,
  8, 8, 8, 2, 4,
  8, 8, 8, 2
};   

// 2. IMPERIAL MARCH
int vaderMelody[] = {
  NOTE_A4, NOTE_A4, NOTE_A4, NOTE_F4, NOTE_C5,
  NOTE_A4, NOTE_F4, NOTE_C5, NOTE_A4,
  NOTE_E5, NOTE_E5, NOTE_E5, NOTE_F5, NOTE_C5,
  NOTE_GS4, NOTE_F4, NOTE_C5, NOTE_A4
};
int vaderDurations[] = {
  4, 4, 4, 6, 12,
  4, 6, 12, 2,
  4, 4, 4, 6, 12,
  4, 6, 12, 2
};

// 3. SEVEN NATION ARMY (Riff)
int snaMelody[] = {
  NOTE_E3, NOTE_E3, NOTE_G3, NOTE_E3, NOTE_D3, NOTE_C3, NOTE_B2,
  NOTE_E3, NOTE_E3, NOTE_G3, NOTE_E3, NOTE_D3, NOTE_C3, NOTE_D3, NOTE_C3, NOTE_B2
};
int snaDurations[] = {
  4, 8, 8, 8, 8, 4, 4,
  4, 8, 8, 8, 8, 8, 8, 8, 4
};
int marioM[] = { NOTE_E5, NOTE_E5, 0, NOTE_E5, 0, NOTE_C5, NOTE_E5, 0, NOTE_G5, 0, NOTE_G4 };
int marioD[] = { 8, 8, 8, 8, 8, 8, 8, 8, 4, 4, 4 };

int piratesM[] = { NOTE_A4, NOTE_C5, NOTE_D5, NOTE_D5, 0, NOTE_D5, NOTE_E5, NOTE_F5, NOTE_F5, 0, NOTE_F5, NOTE_G5, NOTE_E5, NOTE_E5, 0, NOTE_D5, NOTE_C5, NOTE_C5, NOTE_D5 };
int piratesD[] = { 8, 8, 4, 8, 8, 8, 8, 4, 8, 8, 8, 8, 4, 8, 8, 8, 8, 8, 4 };

int indianaM[] = { NOTE_E4, NOTE_F4, NOTE_G4, NOTE_C5, 0, NOTE_D4, NOTE_E4, NOTE_F4, 0, NOTE_G4, NOTE_A4, NOTE_B4, NOTE_F5, 0, NOTE_A4, NOTE_B4, NOTE_C5, NOTE_D5, NOTE_E5 };
int indianaD[] = { 8, 16, 16, 2, 8, 8, 16, 16, 2, 8, 16, 16, 2, 8, 8, 16, 16, 16, 16 };

// 4. MEGALOVANIA (Sans Theme)
  int megaM[] = {
    NOTE_D4, NOTE_D4, NOTE_D5, NOTE_A4, 0, NOTE_GS4, NOTE_G4, NOTE_F4, NOTE_D4, NOTE_F4, NOTE_G4,
    NOTE_C4, NOTE_C4, NOTE_D5, NOTE_A4, 0, NOTE_GS4, NOTE_G4, NOTE_F4, NOTE_D4, NOTE_F4, NOTE_G4,
    NOTE_B3, NOTE_B3, NOTE_D5, NOTE_A4, 0, NOTE_GS4, NOTE_G4, NOTE_F4, NOTE_D4, NOTE_F4, NOTE_G4,
    NOTE_AS3, NOTE_AS3, NOTE_D5, NOTE_A4, 0, NOTE_GS4, NOTE_G4, NOTE_F4, NOTE_D4, NOTE_F4, NOTE_G4
  };
  int megaD[] = {
    16, 16, 8, 8, 16, 8, 8, 8, 16, 16, 16,
    16, 16, 8, 8, 16, 8, 8, 8, 16, 16, 16,
    16, 16, 8, 8, 16, 8, 8, 8, 16, 16, 16,
    16, 16, 8, 8, 16, 8, 8, 8, 16, 16, 16
  };

  // 5. DARUDE - SANDSTORM (Dududududu!)
  int sandM[] = { 
    NOTE_B4, NOTE_B4, NOTE_B4, NOTE_B4, NOTE_B4, 
    NOTE_B4, NOTE_B4, NOTE_B4, NOTE_B4, NOTE_B4, NOTE_B4, 
    NOTE_E5, NOTE_E5, NOTE_E5, NOTE_E5, NOTE_E5, NOTE_E5, NOTE_E5, 
    NOTE_D5, NOTE_D5, NOTE_D5, NOTE_D5, NOTE_D5, NOTE_D5, NOTE_D5, 
    NOTE_A4, NOTE_B4 
  };
  int sandD[] = { 
    8, 8, 8, 8, 8, 
    16, 16, 16, 16, 16, 16, 
    16, 16, 16, 16, 16, 16, 16, 
    16, 16, 16, 16, 16, 16, 16, 
    8, 4 
  };

  int elevatorM[] = { NOTE_C4, NOTE_E4, NOTE_G4, NOTE_C5, NOTE_E5, NOTE_G4, NOTE_C5, NOTE_AS4, NOTE_A4, NOTE_F4, NOTE_A4, NOTE_G4, NOTE_E4, NOTE_C4, NOTE_D4, NOTE_G3 };
  int elevatorD[] = { 4, 4, 4, 4, 2, 4, 2, 8, 4, 4, 4, 4, 4, 4, 4, 2 };

  int familiadaM[] = { 
    NOTE_C5, NOTE_G4, NOTE_E4, NOTE_C4, // Ta-da-da-dam
    NOTE_F4, NOTE_G4, NOTE_A4, NOTE_G4, 
    NOTE_C5, NOTE_G4, NOTE_E4, NOTE_C4, 
    NOTE_D4, NOTE_E4, NOTE_C4 
  };
  int familiadaD[] = { 
    4, 8, 8, 4, 
    8, 8, 8, 4, 
    4, 8, 8, 4, 
    8, 8, 2 
  };

  int sasiedziM[] = {
    NOTE_C5, NOTE_G4, NOTE_E4, NOTE_C5, NOTE_G4, 0,
    NOTE_E4, NOTE_F4, NOTE_G4, NOTE_G4, NOTE_G4, 0,
    NOTE_F4, NOTE_G4, NOTE_A4, NOTE_A4, NOTE_A4, 0,
    NOTE_G4, NOTE_C5, NOTE_E5, NOTE_D5, NOTE_C5
  };
  int sasiedziD[] = {
    4, 8, 8, 4, 4, 8,
    8, 8, 8, 8, 4, 8,
    8, 8, 8, 8, 4, 8,
    4, 4, 4, 4, 2
  };

  int lossM[] = { NOTE_C4, NOTE_B3, NOTE_AS3, NOTE_A3 };
int lossD[] = { 4, 4, 4, 2 }; // Ostatnia nuta jest długa i smutna

int epicMegaM[] = {
  // Sekcja: DOEH DEH DEH...
  NOTE_D4, NOTE_D4, NOTE_D5, NOTE_A4, NOTE_GS4, NOTE_G4, NOTE_F4, NOTE_D4, NOTE_F4, NOTE_G4,
  NOTE_C4, NOTE_C4, NOTE_D5, NOTE_A4, NOTE_GS4, NOTE_G4, NOTE_F4, NOTE_D4, NOTE_F4, NOTE_G4,
  NOTE_B3, NOTE_B3, NOTE_D5, NOTE_A4, NOTE_GS4, NOTE_G4, NOTE_F4, NOTE_D4, NOTE_F4, NOTE_G4,
  NOTE_AS3, NOTE_AS3, NOTE_D5, NOTE_A4, NOTE_GS4, NOTE_G4, NOTE_F4, NOTE_D4, NOTE_F4, NOTE_G4,
  
  // Sekcja: INTENSIFIES (Oktawa wyżej)
  NOTE_D5, NOTE_D5, NOTE_D6, NOTE_A5, NOTE_GS5, NOTE_G5, NOTE_F5, NOTE_D5, NOTE_F5, NOTE_G5,
  NOTE_C5, NOTE_C5, NOTE_D6, NOTE_A5, NOTE_GS5, NOTE_G5, NOTE_F5, NOTE_D5, NOTE_F5, NOTE_G5
};

int epicMegaD[] = {
  // Czas trwania (na podstawie Twoich delayów)
  8, 8, 4, 4, 8, 4, 4, 8, 8, 8,
  8, 8, 4, 4, 8, 4, 4, 8, 8, 8,
  8, 8, 4, 4, 8, 4, 4, 8, 8, 8,
  8, 8, 4, 4, 8, 4, 4, 8, 8, 8,
  
  // Intensifies
  8, 8, 4, 4, 8, 4, 4, 8, 8, 8,
  8, 8, 4, 4, 8, 4, 4, 8, 8, 8
};

int winFFM[] = {
  NOTE_C5, NOTE_C5, NOTE_C5, NOTE_C5, NOTE_GS4, NOTE_AS4, NOTE_C5, 0, NOTE_AS4, NOTE_C5
};
int winFFD[] = {
  8, 8, 8, 4, 4, 4, 8, 12, 8, 2
};

// --- FUNKCJA ODTWARZAJĄCA ---
void playTune(int melody[], int durations[], int size, int tempo) {

  float beatDuration = 60000.0 / tempo;
  for (int i = 0; i < size; i++) {
    // Obliczanie czasu trwania nuty w ms
    int noteDuration = (4.0 / durations[i]) * beatDuration;
    if (melody[i] == 0) {
      // Jeśli nuta to 0, po prostu czekaj (pauza w muzyce)
      delay(noteDuration);
    } else {
      tone(speakerPin, melody[i], noteDuration);
      delay(noteDuration);
      noTone(speakerPin);
      delay(20);
    }
  }
}
  

void setup() {
  pinMode(speakerPin, OUTPUT);
  
  // // 1. Gwiezdne Wojny
  // playTune(starWarsMelody, starWarsDurations, 19, 120);
  // delay(1000);
  
  // // 2. Marsz Imperialny
  // playTune(vaderMelody, vaderDurations, 18, 110);
  // delay(1000);
  
playTune(winFFM, winFFD, 10, 170);
  
  delay(2000);
  
// delay(2000);

// playTune(sasiedziM, sasiedziD, 23, 130);
//   delay(1500);

// playTune(familiadaM, familiadaD, 15, 120);
//   delay(1500);

// playTune(marioM, marioD, 11, 150);
// delay(2000);
// playTune(piratesM, piratesD, 19, 125);
// delay(2000);
// playTune(indianaM, indianaD, 19, 120);
// delay(2000);
// for(int i=0; i<4; i++) {
//     playTune(sandM, sandD, 27, 160);
//   }


// delay(2000);

// for(int i=0; i<2; i++) playTune(megaM, megaD, 44, 150);
// delay(2000);

// playTune(elevatorM, elevatorD, 16, 130);
//   delay(1500);

// playTune(lossM, lossD, 4, 80); // Wolne tempo dla smutku
// delay(1500);


// playTune(epicMegaM, epicMegaD, 60, 200);
}
void loop() {
  // Cisza po zakończeniu utworów
}