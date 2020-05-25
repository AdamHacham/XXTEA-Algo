#include <avr/io.h>
#include <stdint.h>
#include <avr/eeprom.h>
#include "xxtea.h"

//------------------------------------------------
// Programme "hello world" pour carte à puce
// 
//------------------------------------------------


// prototype des fonctions d'entrée/sortie
// définies en assembleur dans le fichier io.s
void sendbytet0(uint8_t b);
uint8_t recbytet0(void);

// variables globales en static ram
uint8_t cla, ins, p1, p2, p3;  // header de commande
uint8_t sw1, sw2;              // status word
int taille;
#define MAXI 16     // taille maxi des données lues
uint8_t data[MAXI]; // données introduites

//variable globale eeprom
uint8_t eecle[MAXI] EEMEM ;
uint8_t cle[MAXI];
uint8_t chiffre[MAXI];
#define HIST_STR	"hello world"
#define HIST_SIZE	sizeof(HIST_STR)

// Procédure qui renvoie l'ATR
void atr()
{
  int8_t n;
  char*hist;
  
  n=HIST_SIZE;
  hist=HIST_STR;
  sendbytet0(0x3b); 	// définition du protocole
  sendbytet0(n);		// nombre d'octets d'historique
  while(n--)		// Boucle d'envoi des octets d'historique
    {
      sendbytet0(*hist++);
    }
}


#define VERSION_STR	"1.00"
#define VERSION_SIZE	sizeof(VERSION_STR)

// émission de la version
// t est la taille de la chaîne sv

void version()
{
  int i;
  char*sv=VERSION_STR;
  
  // vérification de la taille
  if (p3!=VERSION_SIZE)
    {
      sw1=0x6c;		// taille incorrecte
      sw2=VERSION_SIZE;	// taille attendue
      return;
    }
  sendbytet0(ins);	// acquittement
  // émission des données
  for(i=0;i<p3;i++)
    {
      sendbytet0(sv[i]);
    }
  sw1=0x90;	// status word OK
}

// commande de réception de données
void intro_data()
{
  int i;
  // vérification de la taille
  if (p3>MAXI)
    {
      sw1=0x6c;	// P3 incorrect
      sw2=MAXI;	// sw2 contient l'information de la taille correcte
      return;
    }
  sendbytet0(ins);	// acquitement
  
  for(i=0;i<p3;i++)	// boucle de réception du message
    {
      data[i]=recbytet0();
    }
  taille = p3;
  sw1=0x90;
}
void encrypt_xxtea(int n, uint8_t cle[16]){
  int m;
  uint32_t *v ;
  uint32_t *v1 ;
  uint8_t* tmp;
  v = u8_to_U32 (data);
  v1 = u8_to_U32(cle);
  uint32_t const key[4]= {v1[0],v1[1],v1[2],v1[3]};
  btea(v,n,key);
  int i,j;
  for (i=0,m=0  ; i<4;i++){
    j = 0;
    tmp = u32_to_u8(v[i]);
    while(j<4){
      data[m] = tmp[j];
      j++;
      m++;
    }
  }
}

void intro_chiffre(){


  int i;
  // vérification de la taille                                                                    
  if (p3>MAXI)
    {
      sw1=0x6c; // P3 incorrect                                                                   
      sw2=MAXI; // sw2 contient l'information de la taille correcte                               
      return;
    }
  sendbytet0(ins);      // acquitement                                                            

  for(i=0;i<p3;i++)     // boucle de réception du message                                         
    {
      chiffre[i]=recbytet0();
    }
  taille = p3;
  sw1=0x90;


}

// Lecture des données introduites
void lire_data()
{
  int i;
  if(p3!= MAXI){
    sw1 = 0x6c;
    sw2 = MAXI;
    return;
  }
  sendbytet0(ins);
  for(i=0;i<p3;i++){
    sendbytet0(data[i]);
  }
  sw1=0x90;
}

void intro_cle_eeprom()
{
  int i;
  // vérification de la taille
  if (p3>MAXI)
    {
      sw1=0x6c;	// P3 incorrect
      sw2=MAXI;	// sw2 contient l'information de la taille correcte
      return;
    }
  sendbytet0(ins);	// acquitement
  for(i=0;i<p3;i++)	// boucle de réception du message
    {
      cle[i]=recbytet0();
     
    }
  eeprom_write_block(cle,eecle,p3);
  taille = p3;
  sw1=0x90;
}
 
void lire_dans_eeprom()
{
 
  if(p3!= MAXI){
    sw1 = 0x6c;
    sw2 = MAXI;
    return;
  }
  
  sendbytet0(ins);
  for(int i=0 ; i<p3 ; i++){
    cle[i]= eeprom_read_byte(eecle+i);
    sendbytet0(cle[i]);
  }
  sw1=0x90;
}

// Programme principal
//--------------------
int main(void)
{
  
  // initialisation des ports
  ACSR=0x80;
  	DDRA=0xff;
  	DDRB=0xff;
  	DDRC=0xff;
  	DDRD=0x00; 
  	PORTA=0xff;
  	PORTB=0xff;
  	PORTC=0xff;
  	PORTD=0xff;
  	// ATR
  	atr();

	sw2=0;		// pour éviter de le répéter dans toutes les commandes
  	// boucle de traitement des commandes
  	for(;;)
  	{
    		// lecture de l'entête
    		cla=recbytet0();
    		ins=recbytet0();
    		p1=recbytet0();
	    	p2=recbytet0();
    		p3=recbytet0();
	    	sw2=0;
		switch (cla)
		{
	  	case 0x80:
		    	switch(ins)
			{
			case 0:
				version();
				break;
		  	case 1:
	        		intro_data();
				break;
			case 2:
			        lire_data();
				break;
			case 3:
			        intro_cle_eeprom();
				break;
			case 4:
			        lire_dans_eeprom();
				break;
			case 5:
			  //		        intro_data();
			  //    lire_dans_eeprom();
			        encrypt_xxtea(4,cle);
			        break;
			default:
		    		sw1=0x6d; // code erreur ins inconnu
        		}
			break;
      		default:
        		sw1=0x6e; // code erreur classe inconnue
		}
		sendbytet0(sw1); // envoi du status word
		sendbytet0(sw2);
  	}
  	return 0;
}

