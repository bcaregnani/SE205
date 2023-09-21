/***********************************************************
             - UTILISER la commande netstat pour voir
               les connexions entrantes et sortantes
               sur la machine.
             - ATTENTION : le signal  SIGCHLD n'est 
               pas gere, il y aura donc plein
               de petits zombies a observer !
  *********************************************************/

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>



#define PORT_SERVEUR 45678


/*                         SERVEUR                               */

int Sock_Com;
static char message[] = "Reponse de pid                            \n";

/* Fonction executee par les fils du serveur qui dialogueront avec les clients */
void Gerant_Comm(void);


int main (void) {
  int Sock_Serv;
  struct sockaddr_in Le_Serveur;
  int taille = sizeof(Le_Serveur);
  
  if ((Sock_Serv = socket(PF_INET, SOCK_STREAM , 0)) < 0) {
    perror("Erreur d'ouverture du socket en mode connecte");
    exit (1);
  }
  /**********************************************************************
  construction de l'adresse du serveur, attention le numero
  de port est "en dur". 
  **********************************************************************/
  Le_Serveur.sin_family      = AF_INET;
  Le_Serveur.sin_addr.s_addr = INADDR_ANY;
  Le_Serveur.sin_port        = htons( PORT_SERVEUR );
  
  sprintf(message+15, " %d fils de %d", (int) getpid(), (int) getppid());
  printf("Ici le serveur %s\n", message);

  /*     A DECOMMENTER SI NECESSAIRE : 
         pour reutiliser plusieurs fois la meme adresse 
	 (ici, bidon est un int qui vaut 1 ):
  */
  /*if(setsockopt(Sock_Serv, SOL_SOCKET, SO_REUSEADDR, (char *)&bidon, sizeof(bidon)) == -1) 
    {
    perror ("Serveur : erreur sur sockopt ") ;
    }
  */
  
  if (bind( Sock_Serv , (struct sockaddr *) &Le_Serveur , sizeof(struct sockaddr_in ) ) < 0) {
    perror("Serveur: erreur sur bind");
    exit (1);
  }
  
  /* Verifications sur le serveur */
  if (getsockname(Sock_Serv ,(struct sockaddr *) &Le_Serveur, &taille) < 0) {
    perror ("Serveur: erreur sur getsocketname");
    exit (1);
  }
  printf ("Numero de mon port: %d\n", ntohs(Le_Serveur.sin_port));
  printf ("**********************************\n"); 
  
  /* Le serveur se met en attente sur le socket d ecoute */
  listen(Sock_Serv, 5);
  
  /*
   * Ci-dessous le pere ferme le socket de communications
   * et retourne attendre d'autres demandes de connexion
   */

  while (1) {
    /* cf. man -s 3n accept, attention EINTR ou EWOULBLOCK ne sont pas geres! */
    if ((Sock_Com = accept( Sock_Serv , (struct sockaddr *) 0,(int *) 0)) == -1) {
      perror("Serveur: erreur sur Accept");
      continue;
    }
    
    /* Creation d'un processus pour gerer la communication */
    if (fork() == 0) {
      close(Sock_Serv); 
      Gerant_Comm();
    }
    close(Sock_Com);
  }
}

/**************** Fonction de gestion des communications ************/ 
void Gerant_Comm(){
  int Ret_Read;
  char Buf[1024];
  
  if (write(Sock_Com,"Debut de comm.\n", 15) < 0)
    perror("Erreur ecriture Message");

  do {
    memset(Buf, 0, sizeof(Buf));
    if ((Ret_Read = read( Sock_Com , Buf,sizeof(Buf))) < 0) { 
      perror("Erreur lecture Message");
      printf("*************************************\n");
      continue;
    } 
    if (Ret_Read == 0) { 
      printf("Ici, pid %d: Fin de connexion\n", (int) getpid());
      printf("*************************************\n");
    } 
    else {
      printf("Ici %d, fils du serveur %d, lit:\n",
              (int) getpid(), (int) getppid());
      printf("%s\n", Buf);
    }
  } while (Ret_Read > 0);
  
  sprintf(message+15, " %d fils de %d", (int) getpid(), (int) getppid());
  if (write(Sock_Com, message, strlen(message) + 1 ) < 0) {
    printf("Ici pid %d:\n", (int) getpid());
    perror("Erreur ecriture d'un message");
    exit (2);
  }
  exit(1);
}
