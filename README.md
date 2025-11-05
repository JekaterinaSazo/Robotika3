# Robotics - 3 HW.
Išmanus temperatūros valdomas ventiliatorius

# Demo video:

https://github.com/user-attachments/assets/0b2d40f8-779a-4cf6-958a-51779a38d8ea


# Problem:
Karštomis dienomis kambaryje oras greitai sušyla ir tampa tvanku. Visados nauodojau paprastą ventiliatorių
tačiau ji reikėdavo reguliuoti rankiniu būdu - įjungti, išjungti ar keisti greitį priklausomai nuo temperatūros.
Norėdama šį procesą automatizuoti ir daugiau negalvoti apie šią problemą, gal labiau nepatogumą, 
man kilo mintis sukurti išmanų temperatūros valdomą ventiliatorių, 
kuris pagal aplinkos temperatūrą pats reguliuoja ventiliatoriaus greitį.

# Design:
Ši automatinė ventiliatoriaus valdumo sistema pati reguliuoja oro srautą pagal aplinkos temperatūrą.
Vartotojas turi du mygtukus: vienu galima pasirinkti pageidaujamą temperatūrą, kitu - ventiliatoriaus
greitį. RGB LED lemputė rodo nustatytą temperatūros režimą: mėlyna spalva simbolizuoja vėsią,
o raudona - šiltą aplinką. Sistema kas 10 ms matuoja temperatūrą ir, jai pasiekus nustatytą ribą,
automatiškai įjungia ventiliatorių, o temperatūrai nukritus - išjungia. 
Įrenginys išsaugo visus nustatymus EEPROM atmityje, todėl po išjungimo atkuria paskutinę būdeną.


# Parts list:
| Quantity | Component              |
| -------- | ---------------------- |
| 1        | Arduino Uno R3         |
| 1        | Temperatūros jutiklis  |
| 1        | DC Motoras             |
| 1        | Tranzistorius          |
| 2        | Mygtukai               |
| 1        | RGB LED lemputė        |  
| 4        | 220 Ω Resistoriai      |
| 1        | Maitinimo šaltinis (5V)|
| 1        | Diodas                 |

# Design and schematics:
## Design
<img width="800" height="786" alt="Fantabulous Bojo" src="https://github.com/user-attachments/assets/da04ae35-50c2-4c44-8d9a-5881e77d90a0" />

## Schematic of the design
<img width="1944" height="1514" alt="image" src="https://github.com/user-attachments/assets/adf6045f-dd9e-4368-8f1f-c94d51428587" />

# Encountered problems and future improvements:
## Encountered problems
- L293D prarado per daug elektros, tai buvo neįmanoma pasukti motorą, teko naudoti Tranzistoriu
- RGB lempute buvo bendro anodo, o ne bendro katodo, tai teko pakeisti kaip veikia kodas ir sujungti laidai
- Motoras iškrovė 2xAA baterijas per greitai tai teko keisti maitinimo šaltinį, bei jis negavo pakankamai galios kad suktis su mažesniais greičiais.
# Future improvements
- Pridėti LCD ekraną, kad būtų galima matyti nustatytą temperatūros ribą bei dabartinę temperatūrą.
- Įdiegti automatinį ventiliatoriaus greičio valdymą pagal temperatūros skirtumą (dabar tik on/off).
- Integruoti buzzerį ar garsinį signalą, kai pasiekiama nustatyta temperatūra.
