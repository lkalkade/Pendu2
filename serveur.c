
/*----------------------------------------------
Serveur du Jeu du pendu à lancer avant le client
------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <ctype.h>
#include <pthread.h> 

#include "dico.c"

#define BUFFER_SIZE 1024

#define NB_CLIENTS_MAX  42

#define SERVICE_DEFAUT 33016
#define SERVEUR_DEFAUT "localhost"

#define DEBUG 1

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

//Structure qui définit un client du serveur
typedef struct{
    int sock;               /*socket du client*/
    char pseudo[BUFFER_SIZE];        /*pseudo du client*/
    pthread_t thread;       /*thread du serveur auquel est affecté le client*/
    int connected;         /*booléen indiquant si le client est connecté ou non*/
} Client;


//Variables globales pour pouvoir être récupérées depuis les threads
Client Client_Online[NB_CLIENTS_MAX];           /*Tableau contenant tous les clients*/
int nb_client = 0;                          /*Nombre de clients connectés au serveur*/
char message[BUFFER_SIZE]; 

int piocherMot(char *motPioche);
int nombreAleatoire(int nombreMax);

int gagne(int lettreTrouvee[], long tailleMot);
int rechercheLettre(char lettre, char motSecret[], int lettreTrouvee[]);
char lireCaractere(char lettre);

void listerInfo();


			/*****************************************************************************/
				/*--------------- Programme Serveur -----------------------*/
			/*****************************************************************************/




char lireCaractere(char lettre)
{
    char caractere = lettre;
    
    caractere = toupper(caractere); // On met la lettre en majuscule si elle ne l'est pas déjà

    return caractere; // On retourne le premier caractère qu'on a lu
}


int gagne(int lettreTrouvee[], long tailleMot)
{
    long i = 0;
    int joueurGagne = 1;

    for (i = 0 ; i < tailleMot ; i++)
    {
        if (lettreTrouvee[i] == 0)
            joueurGagne = 0;
    }

    return joueurGagne;
}

int rechercheLettre(char lettre, char motSecret[], int lettreTrouvee[])
{

    long i = 0;
    int bonneLettre = 0;
         
    // On parcourt motSecret pour vérifier si la lettre proposée y est
    for (i = 0 ; motSecret[i] != '\0' ; i++)
    {
        if (lettre == motSecret[i]) // Si la lettre y est
        {
            bonneLettre = 1; // On mémorise que c'était une bonne lettre
            lettreTrouvee[i] = 1; // On met à 1 le case du tableau de booléens correspondant à la lettre actuelle
        }
    }
    
                    
    return bonneLettre;
}


// verifie si le char c a déja été joué ou non
int verif_lettre(int *lettre_jouer, char c)
{
	int i;
	i = c - 'a'; 
	return lettre_jouer[i];
}
void maj_lettre(int *lettre_jouer, char c)
{
	int i;
	i = c - 'a'; // a check
	lettre_jouer[i]==1;
}


int cree_socket_tcp_ip_serveur(int port, char* adresse_ip)
{
	int sock;
	struct sockaddr_in adresse;

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		fprintf(stderr, "Erreur socket\n");
		return -1;
	}

	memset(&adresse, 0, sizeof(struct sockaddr_in));
	adresse.sin_family = AF_INET;
	adresse.sin_port = htons(port);
	inet_aton(adresse_ip, &adresse.sin_addr);

	if (bind(sock, (struct sockaddr*) &adresse, sizeof(struct sockaddr_in)) < 0)
	{
		close(sock);
		fprintf(stderr, "Erreur bind\n");
		return -1;
	}

	return sock;
}

int affiche_adresse_socket(int sock)
{
	struct sockaddr_in adresse;
	socklen_t longueur;

	longueur = sizeof(struct sockaddr_in);
	
	if (getsockname(sock, (struct sockaddr*) &adresse, &longueur) < 0)
	{
		fprintf(stderr, "Erreur getsockname\n");
		return -1;
	}

	printf("IP = %s, Port = %u\n", inet_ntoa(adresse.sin_addr), ntohs(adresse.sin_port));

	return 0;
}


int serveur_appli (char *serveur, int service);   

main(int argc,char *argv[])
{

	char *serveur= SERVEUR_DEFAUT;
	int service= SERVICE_DEFAUT; /* no de port) */


	/* Permet de passer un nombre de parametre variable a l'executable */
	switch (argc)
 	{
   	case 1:
		  printf("defaut service = %d\n", service);
		  break;
 	case 2:
		  service=atoi(argv[1]);
		  break;

   	default :
		  printf("Usage:\n> ./serveur service \n");
		  return EXIT_SUCCESS;
 	}

	
	serveur_appli(serveur,service);
}

void * Connexion_client (void * c)
{
	Client * client = (Client *) c;
	char pseudos[256];
	char *answer = malloc (sizeof (*answer) * 256);
	int l,i,j=0;

	printf("Connexion d'un client.\n"); 
	
	//Si le client n'a pas de pseudo
	while(strlen((*client).pseudo)<=1)
	{
		l=read(client->sock, pseudos, sizeof(pseudos));			
	        sleep(3);
	        pseudos[l]='\0'; 
	        strcpy((*client).pseudo, pseudos);
        	write(client->sock,pseudos,strlen(pseudos)+1);		
    	}
	printf("Player %s est connecté\n", (*client).pseudo);
}

    

void * commande(void * c)
{
    	Client * client = (Client *) c;
	
	char *answer = malloc (sizeof (*answer) * 256);
	int longueur;
    	
	char envoi[BUFFER_SIZE];
    	char mot[BUFFER_SIZE];
   	char score[BUFFER_SIZE];
   	char coup[BUFFER_SIZE];  
 	
	int lettres[26]; // lettres[i] vaut 0 si la ieme lettre n'a pas été jouée
    	int pid;
	
    	pid_t fils;

    	char result[BUFFER_SIZE];
	
    
    	int i, points,continuer,choix;
    
    	char lettre = 0; // Stocke la lettre proposée par l'utilisateur (retour du scanf)
    	char l=0;
    	char motSecret[100] = {0}; // Ce sera le mot à trouver
   	int *lettreTrouvee = NULL; // Un tableau de booléens. Chaque case correspond à une lettre du mot secret. 0 = lettre non trouvée, 1 = lettre trouvée
    	long coupsRestants = 10; // Compteur de coups restants (0 = mort)
    	long tailleMot = 0;
    
    	points=0;
    	continuer=1;

	fils = fork();
		
	if (fils == -1)
	{
		fprintf(stderr, "Erreur fork\n");
		
	}	

	if (fils == 0)
	{
		//close(sock_contact);				
		while(continuer==1)
		{
		        /*à chaque connection d'un client on pioche un mot*/
 			if (!piocherMot(motSecret))
  			      exit(0);
    
  			printf("mot: %s\n",motSecret);
    
    			tailleMot = strlen(motSecret);

    			lettreTrouvee = malloc(tailleMot * sizeof(int)); // On alloue dynamiquement le tableau lettreTrouvee (dont on ne connaissait pas la taille au départ)
    			if (lettreTrouvee == NULL)
        			exit(0);

    			for (i = 0 ; i < tailleMot ; i++)
        		lettreTrouvee[i] = 0;

    			// On continue à jouer tant qu'il reste au moins un coup à jouer ou qu'on
    			// n'a pas gagné
    			while (coupsRestants > 0 && !gagne(lettreTrouvee, tailleMot))
    			{
        			// On affiche le mot secret en masquant les lettres non trouvées
        			//Exemple : *A**ON 
        			for (i = 0 ; i < tailleMot ; i++)
        			{
            				if (lettreTrouvee[i])
					{ 	// Si on a trouvé la lettre n°i
                				mot[i]=motSecret[i];// On l'affiche
		     	                }

			            	else
               					mot[i]='*';
               					// Sinon, on affiche une étoile pour les lettres non trouvées
        			}

            			fflush(stdin);
	    			fflush(stdout);
         				
				if (DEBUG == 1)
            			{
					//printf("Le resultat du serveur est: %s. \n", mot);
					sprintf(coup, "%ld",coupsRestants);
					sprintf(envoi, "Le mot a trouvee est: %s \n\nil vous reste %s coups a jouer", mot,coup);
		
            			}
            
            			// Ecriture de la reponse a la requete dans envoi

            			if (write(Client_Online[nb_client-1].sock, envoi, strlen(envoi)+1) < 0)
    				{
					perror("write sock to client");
        				exit(EXIT_FAILURE);
        				
    				}					
	
				if (read(Client_Online[nb_client-1].sock, message, BUFFER_SIZE) < 0)
    				{
        				perror("read sock client");
        				exit(EXIT_FAILURE);
        			
    				}
				
				if(strcmp(message,"/q")==0)
				{
					printf("%s a entré la commande /q\n", Client_Online[nb_client-1].pseudo);
					printf("%s a quitté le serveur\n", Client_Online[nb_client-1].pseudo);
					pthread_exit(NULL);
								
    				}					
        			
				for (i = 0 ; message[i] != '\0' ; i++)
        			{    
            				l = message[i]; 
        			}

				lettre=lireCaractere(l);
					
				// Si ce n'était PAS la bonne lettre
				if (!rechercheLettre(lettre, motSecret, lettreTrouvee))
        			{            
            				coupsRestants--; // On enlève un coup au joueur
        			}

				if(!(verif_lettre(lettres,l))) // La lettre n'a pas été donnée
				{
					maj_lettre(lettres,l);
				}
				else
				{
            				coupsRestants--; // On enlève un coup au joueur
				
				}
					
    			}


   			if (gagne(lettreTrouvee, tailleMot))
    			{
				points++;
				sprintf(score,"%d",points);	
	
	
				//printf("\n\nGagne ! Le mot secret etait bien : %s \nScore:%d\n", motSecret,points);
				sprintf(result, "Gagne ! Le mot secret etait bien : %s \nScore:%s", motSecret,score);
	
    			}

    			else
    			{
        			//printf("\n\nPerdu ! Le mot secret etait : %s \nScore:%d\n", motSecret,points);
        			sprintf(result, "Perdu ! Le mot secret etait : %s \nScore:%s", motSecret,score);
	
    			}

    			// Ecriture de la reponse a la requete result
			if (write(Client_Online[nb_client-1].sock, result, strlen(result)+1) < 0)
    			{
        			perror("write sock to client");
        			exit(EXIT_FAILURE);
        			
    			}
   				
			free(lettreTrouvee); // On libère la mémoire allouée manuellement (par malloc)
    
    			continuer=0;
    
   			if (continuer==0)
    			{
        			// Lecture du message
				if (read(Client_Online[nb_client-1].sock, message, BUFFER_SIZE) < 0)
    				{
        				perror("read sock client");
        				exit(EXIT_FAILURE);
        				
    				}
       				sscanf(message,"%d",&choix);
				if(choix==1)
				{					
					coupsRestants=10;
					fflush(stdin);
					fflush(stdout);
					memset(mot, 0, sizeof (mot));
					continuer=choix;					
				}
				else
				{
					printf("%s a quitté le serveur\n", Client_Online[nb_client-1].pseudo);
					close(Client_Online[nb_client-1].sock);
					pthread_exit(NULL);
					continuer=choix;
				}
				
    			}
    					
		}

	exit(EXIT_SUCCESS);

	} //fin de fils

	else
	{
		close(Client_Online[nb_client-1].sock);		
	}

	
}

int serveur_appli(char *serveur, int service)
{
       	//char message[BUFFER_SIZE];   
  	int sock_contact;
	int sock_connectee,i;
	struct sockaddr_in adresse;
	socklen_t longueur;
	
	pthread_t t[NB_CLIENTS_MAX];	

	sock_contact = cree_socket_tcp_ip_serveur(service,serveur);
	if (sock_contact < 0)
	{
		return -1;
	}

	listen(sock_contact, 5);

	printf("Serveur (sock contact) -> ");
	affiche_adresse_socket(sock_contact);   


	while(1)
	{

    		/*connexion client*/
    		// Ouverture tube d'appel
    		// Lecture du tube d'appel (message)
    		//renvoie le id du client au client
		
		longueur = sizeof(struct sockaddr_in);
		sock_connectee = accept(sock_contact, (struct sockaddr*) &adresse, &longueur);
	
		if (sock_connectee < 0)
		{
			fprintf(stderr, "Erreur accept\n");
			return -1;
		}
		
		else
		{
			/* adresse_client_courant sera renseigné par accept via les infos du connect */
        		/*on check s'il y a assez de place sur le serveur*/
			if (nb_client >= NB_CLIENTS_MAX) 
			{
				perror("erreur : le serveur est saturé");
				exit(1);
			}
        		
			else
			{
				//si le client n'est pas encore connecté
            			if(Client_Online[nb_client].connected == 0)
				{                
                			//création d'un thread pour le client
                			Client_Online[nb_client].connected = 1;
                    			Client_Online[nb_client].pseudo[0] = '\0';
                   			Client_Online[nb_client].sock = sock_connectee;
                  			pthread_create(&Client_Online[nb_client].thread, NULL, Connexion_client, &Client_Online[nb_client]);
					pthread_join(Client_Online[nb_client].thread, (void*)NULL);
					
				}
			}
			                  				
			nb_client++;    //on incrémente le nombre de clients connectés sur le serveur
					
		}
		
		pthread_create(&t[nb_client-1], NULL, commande, &Client_Online[nb_client]);
		pthread_join(t[nb_client-1], (void*)NULL);

	}
	return 0;
}
