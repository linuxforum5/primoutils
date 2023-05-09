Amennyiben a Primonak legalbb 1 működőképes memóriatartománya van, úgy a működő memóriablokkba (16KB) betölthető memóriateszt program.
3 változata van, a 3 memóriablokk tesztelésére. A betöltési és tesztelési címek a következők:
Program          Betöltési címe   Tesztelt memóriatartomány
memtest8000.c    0x4400           0x8000-0xAFFF
memtest4000.c    0x8000           0x4000-0x7FFF
memtestB000.c    0x4400           0xB000-0xFFFF               ; A képernyőmemóriából mindig azt teszteli, amelyik éppen nem látszik.
A hibásnak talált memóriacsipeket fizikai elhelyezkedésük szerint mutatja, mintha felülről néznék rá az alaplapra.
